// =============================================================================
//  AZ VERB — DSP yadrosi  (c) Azimxodjayev Music
//  JUCE'ga bog'liq emas: alohida test qilish va boshqa loyihalarda ishlatish mumkin.
//
//  Tovush falsafasi:
//   * HALL  — "yumshoq qum": silliq, donador emas, metall jiringlashsiz dum.
//             8 liniyali FDN + Householder matritsa + tasodifiy (SAND) modulyatsiya.
//   * PLATE — "kristal": zich, yorqin, lekin sibilyantsiz — reverb kirishida
//             avtomatik de-esser, dumda toza "air" shelf.
//   * Mixda ko'milmaydi: FOCUS (vokal kuylayotganda dumning 300 Hz–3 kHz
//             qismini dinamik bo'shatadi), DUCK (iboralar orasida "gullaydi").
//   * CPU: oversampling yo'q, sinus/modulyatsiya control-rate (har 16 namunada).
// =============================================================================
#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace az
{
constexpr float kPi = 3.14159265358979f;

inline float dbToGain (float db) { return std::pow (10.0f, db * 0.05f); }

// ---------- Kichik bloklar -----------------------------------------------------
struct SVF   // Simper TPT state-variable filter
{
    float g = 0, k = 1.4142f, a1 = 0, a2 = 0, a3 = 0, ic1 = 0, ic2 = 0;
    void set (float fc, float q, float fs)
    {
        fc = std::clamp (fc, 10.0f, 0.45f * fs);
        g = std::tan (kPi * fc / fs); k = 1.0f / q;
        a1 = 1.0f / (1.0f + g * (g + k)); a2 = g * a1; a3 = g * a2;
    }
    inline void tick (float v0, float& lp, float& bp, float& hp)
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2;
        lp = v2; bp = v1; hp = v0 - k * v1 - v2;
    }
    inline float lp (float x) { float l, b, h; tick (x, l, b, h); return l; }
    inline float hp (float x) { float l, b, h; tick (x, l, b, h); return h; }
    inline float bpNorm (float x) { float l, b, h; tick (x, l, b, h); return k * b; } // 0 dB cho'qqi
    void reset() { ic1 = ic2 = 0; }
};

struct OnePole
{
    float a = 0, z = 0;
    void setHz (float fc, float fs) { a = std::exp (-2.0f * kPi * std::min (fc, 0.49f * fs) / fs); }
    void setMs (float ms, float fs) { a = std::exp (-1.0f / (std::max (ms, 0.01f) * 0.001f * fs)); }
    inline float p (float x) { z = x + a * (z - x); return z; }
    void reset() { z = 0; }
};

struct Delay
{
    std::vector<float> b; int mask = 0, w = 0;
    void init (int maxSamples)
    {
        int n = 1; while (n < maxSamples + 4) n <<= 1;
        b.assign ((size_t) n, 0.0f); mask = n - 1; w = 0;
    }
    inline void push (float x) { b[(size_t) w] = x; w = (w + 1) & mask; }
    inline float read (int d) const { return b[(size_t) ((w - 1 - d) & mask)]; }
    inline float readFrac (float d) const
    {
        const int i = (int) d; const float f = d - (float) i;
        const float a = b[(size_t) ((w - 1 - i) & mask)];
        const float c = b[(size_t) ((w - 2 - i) & mask)];
        return a + (c - a) * f;
    }
    void clear() { std::fill (b.begin(), b.end(), 0.0f); }
};

struct Allpass
{
    Delay d; int len = 1; float g = 0.5f;
    inline float p (float x)
    {
        const float del = d.read (len - 1);
        const float v = x + g * del;
        d.push (v);
        return del - g * v;
    }
};

// Kichik, tez psevdo-tasodifiy generator (SAND uchun)
struct Rng { uint32_t s = 22222; inline float next() { s = s * 1664525u + 1013904223u; return (float) (s >> 8) * (1.0f / 8388608.0f) * 2.0f - 1.0f; } };

// ---------- Parametrlar --------------------------------------------------------
enum class Algo { Hall = 0, Church, Plate, Room1, Room2, Ambient, NonLin };

struct Params
{
    Algo  algo       = Algo::Plate;
    float preDelayMs = 24.0f;    // 0..2000 (sync bo'lsa protsessor hisoblab beradi)
    float size       = 0.5f;     // 0..1
    float decayS     = 2.4f;     // 0.1..20
    float diffusion  = 0.78f;    // 0..1
    float dampHz     = 6200.0f;  // dum ichidagi HF so'nish
    float lowCutHz   = 180.0f;
    float highCutHz  = 12000.0f;
    float width      = 1.2f;     // 0..2
    float mix        = 0.32f;    // 0..1
    bool  freeze     = false;
    bool  duck       = true;
    float duckDb     = 8.0f;     // 0..18
    // --- AZ fishkalari ---
    float sand       = 0.45f;    // yumshoqlik / tasodifiy modulyatsiya
    float crystal    = 0.40f;    // dum "havosi" va jilosi
    float focus      = 0.50f;    // anti-masking (vokal ustida dumni bo'shatish)
    float deEss      = 0.50f;    // reverb kirishidagi sibilyant nazorati
    float deEssHz    = 6500.0f;  // Smart Vocal aniqlaydi
};

// ---------- Algoritm xaritasi --------------------------------------------------
struct AlgoShape
{
    float sizeLo, sizeHi;     // FDN uzunlik ko'paytuvchisi
    float erLevel, erSpread;  // erta aks-sadolar
    float diffMax;            // kirish diffuziyasi chegarasi
    float modMs, modRate;     // asosiy modulyatsiya
    float dampMul;            // damping ko'paytuvchisi
    float lateLevel;
    float trim;               // algoritmlar orasida bir xil balandlik
    float loopDiff;           // liniya ichidagi diffuziya (zichlik, metallsiz dum)
};

inline AlgoShape shapeFor (Algo a)
{
    switch (a)
    {
        case Algo::Hall:    return { 0.85f, 2.00f, 0.30f, 1.00f, 0.78f, 0.45f, 1.00f, 1.00f, 1.00f, 1.27f, 0.50f };
        case Algo::Church:  return { 1.30f, 2.70f, 0.38f, 1.60f, 0.70f, 0.55f, 0.70f, 0.85f, 1.00f, 1.28f, 0.45f };
        case Algo::Plate:   return { 0.55f, 1.20f, 0.00f, 0.00f, 0.80f, 0.16f, 1.30f, 1.35f, 1.10f, 1.03f, 0.66f };
        case Algo::Room1:   return { 0.25f, 0.62f, 0.70f, 0.45f, 0.62f, 0.18f, 0.90f, 0.95f, 0.85f, 0.70f, 0.60f };
        case Algo::Room2:   return { 0.42f, 0.95f, 0.50f, 0.65f, 0.74f, 0.25f, 0.80f, 0.80f, 0.95f, 0.88f, 0.58f };
        case Algo::Ambient: return { 0.70f, 1.50f, 0.00f, 0.00f, 0.80f, 0.70f, 0.55f, 0.90f, 1.00f, 1.45f, 0.62f };
        case Algo::NonLin:  default:
                            return { 0.40f, 0.80f, 0.00f, 0.00f, 0.78f, 0.10f, 1.00f, 1.10f, 0.00f, 1.03f, 0.0f };
    }
}

// =============================================================================
class Engine
{
public:
    static constexpr int N = 8;

    void prepare (double sampleRate, int /*maxBlock*/)
    {
        fs = (float) sampleRate;
        preL.init ((int) (2.1f * fs)); preR.init ((int) (2.1f * fs));
        for (auto& d : lines) d.init ((int) (0.40f * fs));
        for (auto& a : loopAp) a.d.init ((int) (0.012f * fs));
        erBuf.init ((int) (0.60f * fs));

        static const float apMsL[4] = { 4.771f, 3.595f, 12.73f, 9.307f };
        static const float apMsR[4] = { 4.931f, 3.713f, 12.11f, 9.791f };
        for (int i = 0; i < 4; ++i)
        {
            apL[i].d.init ((int) (0.03f * fs)); apMaxL[i] = apMsL[i];
            apR[i].d.init ((int) (0.03f * fs)); apMaxR[i] = apMsR[i];
        }
        for (int i = 0; i < N; ++i) { lfoPh[i] = (float) i * 0.79f; rnd[i].s = 1234u + 977u * (uint32_t) i; }
        reset();
    }

    void reset()
    {
        preL.clear(); preR.clear(); erBuf.clear();
        for (auto& d : lines) d.clear();
        for (auto& a : loopAp) a.d.clear();
        for (int i = 0; i < 4; ++i) { apL[i].d.clear(); apR[i].d.clear(); }
        for (auto* f : allFilters()) f->reset();
        for (auto& o : damp) o.reset();
        envDry.reset(); envFull.reset(); envSib.reset(); duckSm.reset();
        for (int i = 0; i < N; ++i) { curLen[i] = tgtLen[i]; modCur[i] = modTgt[i] = 0; noiseCur[i] = noiseTgt[i] = 0; }
        updateCoeffs (true);
    }

    void setParams (const Params& np) { p = np; updateCoeffs (false); }
    const Params& params() const { return p; }

    // Joriy dum uzunligi (host uchun)
    double tailSeconds() const { return p.freeze ? 1.0e9 : (double) (p.preDelayMs * 0.001f + p.decayS * 1.2f); }

    // Stereo in-place ishlov
    void process (float* L, float* R, int n)
    {
        for (int s = 0; s < n; ++s)
        {
            if ((ctrlCounter++ & 15) == 0) controlTick();

            const float dryL = L[s], dryR = R ? R[s] : L[s];
            const float monoIn = 0.5f * (dryL + dryR);

            // ---- dry konvert (DUCK va FOCUS uchun) ----
            const float aIn = std::fabs (monoIn);
            const float env = aIn > envDry.z ? (envDry.z = aIn + envAtk * (envDry.z - aIn)) : envDry.p (aIn);

            // ---- reverb kirishi: de-esser ----
            float inL = dryL, inR = dryR;
            {
                const float hL = inL - deEssHpL.lp (inL), hR = inR - deEssHpR.lp (inR);   // toza crossover: in - LP
                const float hm = 0.5f * std::fabs (hL + hR);
                envSib.p (hm); envFull.p (aIn);
                float gr = 1.0f;
                const float thr = (0.30f - 0.22f * p.deEss) * envFull.z + 1.0e-6f;
                if (envSib.z > thr) gr = std::pow (thr / envSib.z, 0.4f + 0.6f * p.deEss);
                const float cut = (1.0f - gr) * std::min (1.0f, p.deEss * 1.6f);
                inL -= hL * cut; inR -= hR * cut;
            }
            if (p.freeze) { inL = 0; inR = 0; }

            // ---- predelay (stereo) ----
            preL.push (inL); preR.push (inR);
            preSm += 0.0015f * (preTarget - preSm);
            const float pdS = std::max (0.0f, preSm);
            float xL = preL.readFrac (pdS), xR = preR.readFrac (pdS);
            const float pd = 0.5f * (xL + xR);

            // ---- erta aks-sadolar ----
            float erL = 0, erR = 0;
            erBuf.push (pd);
            if (erOn)
            {
                for (int t = 0; t < 8; ++t) { erL += erBuf.read (erTapL[t]) * erGain[t]; erR += erBuf.read (erTapR[t]) * erGain[t]; }
                erL *= erLevel; erR *= erLevel;
            }
            if (isNonLin)
            {
                for (int t = 0; t < 24; ++t)
                {
                    const float v = erBuf.read (nlTap[t]) * nlGain[t];
                    if (t & 1) erR += v; else erL += v;
                }
            }

            // ---- kirish diffuziyasi ----
            const float fL = xL + erL * 0.5f, fR = xR + erR * 0.5f;
            float dL = fL, dR = fR;
            for (int i = 0; i < 4; ++i) { dL = apL[i].p (dL); dR = apR[i].p (dR); }

            // ---- FDN (8 liniya, Householder) ----
            float out[N]; float sum = 0;
            for (int i = 0; i < N; ++i)
            {
                curLen[i] += 0.0008f * (tgtLen[i] - curLen[i]);           // SIZE silliq o'zgaradi
                const float m = modCur[i] + modStep[i] * (float) (ctrlCounter & 15);
                const float rd = std::clamp (curLen[i] + m, 2.0f, (float) lines[i].mask - 4.0f);
                float v = lines[i].readFrac (rd);
                v = loopAp[i].p (damp[i].p (v)) * fb[i];
                out[i] = v; sum += v;
            }
            const float hh = sum * (2.0f / N);
            const float inj[2] = { dL * inGain, dR * inGain };
            for (int i = 0; i < N; ++i)
            {
                const float sgn = (i & 2) ? -1.0f : 1.0f;
                lines[i].push (out[i] - hh + inj[i & 1] * sgn);
            }

            float wL = (out[0] - out[2] + out[4] - out[6]) * lateLevel;
            float wR = (out[1] - out[3] + out[5] - out[7]) * lateLevel;
            wL += erL; wR += erR;
            if (isNonLin) { wL += dL * 0.35f; wR += dR * 0.35f; }
            wL *= outTrim; wR *= outTrim;

            // ---- dum EQ: low cut / high cut ----
            wL = lcL.hp (wL); wR = lcR.hp (wR);
            wL = hcL.lp (wL); wR = hcR.lp (wR);

            // ---- FOCUS: loyqalikni olib tashlash + vokal ustida dinamik bo'shatish ----
            const float envN = std::min (1.0f, env * 8.0f);              // ~ -18 dBFS da to'liq
            const float dyn  = p.freeze ? 0.0f : p.focus * 0.78f * envN;
            wL -= mudL.bpNorm (wL) * p.focus * 0.32f + presL.bpNorm (wL) * dyn;
            wR -= mudR.bpNorm (wR) * p.focus * 0.32f + presR.bpNorm (wR) * dyn;

            // ---- CRYSTAL: havo shelf + yumshoq garmonik jilo ----
            if (p.crystal > 0.001f)
            {
                const float aL = airL.hp (wL), aR = airR.hp (wR);
                const float eL = exL.bpNorm (wL), eR = exR.bpNorm (wR);
                const float hL = exHpL.hp (eL * std::fabs (eL) * 6.0f), hR = exHpR.hp (eR * std::fabs (eR) * 6.0f);
                wL += aL * crystalAir + hL * p.crystal * 0.5f;
                wR += aR * crystalAir + hR * p.crystal * 0.5f;
            }

            // ---- DUCK (bloom) ----
            float dg = 1.0f;
            if (p.duck && ! p.freeze)
            {
                const float envDb = 20.0f * std::log10 (env + 1.0e-6f);
                const float amt = std::clamp ((envDb + 40.0f) / 24.0f, 0.0f, 1.0f);
                const float tgt = dbToGain (-p.duckDb * amt);
                dg = tgt < duckSm.z ? (duckSm.z = tgt + duckAtk * (duckSm.z - tgt)) : duckSm.p (tgt);
            }
            else dg = duckSm.p (1.0f);
            wL *= dg; wR *= dg;

            // ---- kenglik (M/S) ----
            const float mid = 0.5f * (wL + wR), side = 0.5f * (wL - wR) * p.width;
            wL = mid + side; wR = mid - side;

            // ---- aralash ----
            dryG += 0.002f * (dryT - dryG); wetG += 0.002f * (wetT - wetG);
            L[s] = dryL * dryG + wL * wetG;
            if (R) R[s] = dryR * dryG + wR * wetG;
        }
    }

private:
    Params p;
    float fs = 48000.0f;

    Delay preL, preR, erBuf;
    Delay lines[N];
    OnePole damp[N];
    Allpass loopAp[N];
    Allpass apL[4], apR[4]; float apMaxL[4] {}, apMaxR[4] {};

    SVF lcL, lcR, hcL, hcR, mudL, mudR, presL, presR, airL, airR, exL, exR, exHpL, exHpR, deEssHpL, deEssHpR;
    std::vector<SVF*> allFilters() { return { &lcL,&lcR,&hcL,&hcR,&mudL,&mudR,&presL,&presR,&airL,&airR,&exL,&exR,&exHpL,&exHpR,&deEssHpL,&deEssHpR }; }

    OnePole envDry, envFull, envSib, duckSm;
    float envAtk = 0, duckAtk = 0;

    float baseMs[N] = { 29.71f, 37.11f, 41.13f, 43.67f, 53.93f, 59.29f, 67.13f, 73.73f };
    float tgtLen[N] {}, curLen[N] {}, fb[N] {};
    float lfoPh[N] {}, modCur[N] {}, modTgt[N] {}, modStep[N] {}, noiseCur[N] {}, noiseTgt[N] {};
    Rng rnd[N];
    float modDepthS = 0, modRateHz = 0.5f;
    unsigned ctrlCounter = 0; int noiseCount = 0;

    int erTapL[8] {}, erTapR[8] {}; float erGain[8] {}; float erLevel = 0; bool erOn = false;
    int nlTap[24] {}; float nlGain[24] {}; bool isNonLin = false;

    float outTrim = 1.0f, inGain = 0.35f, lateLevel = 1.0f, crystalAir = 0.0f;
    float preTarget = 0, preSm = 0;
    float dryG = 1, wetG = 0, dryT = 1, wetT = 0;

    void controlTick()
    {
        // LFO + SAND (silliqlangan tasodif) — har 16 namunada
        const float dt = 16.0f / fs;
        const bool newNoise = (++noiseCount % 512) == 0;
        for (int i = 0; i < N; ++i)
        {
            lfoPh[i] += 2.0f * kPi * modRateHz * (0.63f + 0.11f * (float) i) * dt;
            if (lfoPh[i] > 2.0f * kPi) lfoPh[i] -= 2.0f * kPi;
            if (newNoise) noiseTgt[i] = rnd[i].next();
            noiseCur[i] += 0.004f * (noiseTgt[i] - noiseCur[i]);
            const float sandAmt = p.sand;
            const float m = modDepthS * ((1.0f - 0.6f * sandAmt) * std::sin (lfoPh[i]) + (0.35f + 1.6f * sandAmt) * noiseCur[i]);
            modCur[i] = modTgt[i];
            modTgt[i] = m;
            modStep[i] = (modTgt[i] - modCur[i]) / 16.0f;
        }
    }

    void updateCoeffs (bool snap)
    {
        const AlgoShape sh = shapeFor (p.algo);
        isNonLin = (p.algo == Algo::NonLin);

        // --- FDN uzunliklari va feedback ---
        const float scale = sh.sizeLo + (sh.sizeHi - sh.sizeLo) * p.size;
        // interpolyatsiya va damping yo'qotishini kompensatsiya qilish (o'lchov asosida)
        const float dampEff = std::min (p.dampHz * sh.dampMul, 20000.0f);
        const float rt = std::clamp (p.decayS, 0.1f, 30.0f) * (1.12f + 0.12f * (1.0f - dampEff / 20000.0f));
        for (int i = 0; i < N; ++i)
        {
            tgtLen[i] = baseMs[i] * scale * 0.001f * fs;
            if (snap) curLen[i] = tgtLen[i];
            static const float loopApMs[N] = { 1.31f, 1.73f, 2.11f, 2.53f, 2.97f, 3.41f, 3.89f, 4.37f };
            loopAp[i].len = std::max (1, (int) (loopApMs[i] * (0.6f + 0.8f * p.size) * 0.001f * fs));
            loopAp[i].g = sh.loopDiff * (0.75f + 0.25f * p.diffusion);
            const float effLen = tgtLen[i] + (float) loopAp[i].len;
            const float g = std::pow (10.0f, -3.0f * (effLen / fs) / rt);
            fb[i] = p.freeze ? 1.0f : (isNonLin ? 0.0f : std::min (g, 0.9995f));
            damp[i].setHz (p.freeze ? 22000.0f : std::min (p.dampHz * sh.dampMul, 20000.0f), fs);
        }
        lateLevel = isNonLin ? 0.0f : sh.lateLevel * 0.55f;
        outTrim = sh.trim;
        inGain = 0.42f;

        // --- modulyatsiya ---
        modDepthS = (sh.modMs * (0.5f + 0.9f * p.sand)) * 0.001f * fs;
        modRateHz = sh.modRate;

        // --- kirish diffuzerlari ---
        const float g = std::clamp (p.diffusion * sh.diffMax + 0.06f * p.sand, 0.0f, 0.82f);
        const float apScale = 0.6f + 0.7f * p.size;
        for (int i = 0; i < 4; ++i)
        {
            apL[i].len = std::max (1, (int) (apMaxL[i] * apScale * 0.001f * fs));
            apR[i].len = std::max (1, (int) (apMaxR[i] * apScale * 0.001f * fs));
            apL[i].g = apR[i].g = (i < 2 ? g : g * 0.85f);
        }

        // --- erta aks-sadolar ---
        erOn = sh.erLevel > 0.0f && ! isNonLin;
        erLevel = sh.erLevel;
        static const float erMsL[8] = { 7.1f, 11.3f, 17.9f, 23.4f, 31.7f, 38.2f, 46.9f, 55.3f };
        static const float erMsR[8] = { 8.3f, 12.9f, 16.1f, 25.7f, 29.9f, 40.6f, 44.1f, 58.7f };
        const float erScale = sh.erSpread * (0.5f + p.size);
        for (int t = 0; t < 8; ++t)
        {
            erTapL[t] = std::min ((int) (erMsL[t] * erScale * 0.001f * fs), erBuf.mask - 2);
            erTapR[t] = std::min ((int) (erMsR[t] * erScale * 0.001f * fs), erBuf.mask - 2);
            erGain[t] = 0.55f * std::pow (0.82f, (float) t) * ((t & 1) ? -1.0f : 1.0f);
        }

        // --- NonLin (gated) ---
        if (isNonLin)
        {
            const float winMs = 90.0f + 330.0f * p.size;
            Rng r; r.s = 777u;
            for (int t = 0; t < 24; ++t)
            {
                const float pos = ((float) t + 0.5f + 0.35f * r.next()) / 24.0f;
                nlTap[t] = std::min ((int) (pos * winMs * 0.001f * fs), erBuf.mask - 2);
                const float shape = pos < 0.85f ? (0.75f + 0.25f * pos) : (1.0f - (pos - 0.85f) / 0.15f);
                nlGain[t] = 0.16f * shape * ((t % 3) ? 1.0f : -1.0f);
            }
        }

        // --- predelay ---
        preTarget = std::clamp (p.preDelayMs, 0.0f, 2000.0f) * 0.001f * fs;
        if (snap) preSm = preTarget;

        // --- filtrlar ---
        lcL.set (p.lowCutHz, 0.707f, fs);  lcR.set (p.lowCutHz, 0.707f, fs);
        hcL.set (p.highCutHz, 0.707f, fs); hcR.set (p.highCutHz, 0.707f, fs);
        mudL.set (320.0f, 0.9f, fs);  mudR.set (320.0f, 0.9f, fs);
        presL.set (1600.0f, 0.55f, fs); presR.set (1600.0f, 0.55f, fs);
        airL.set (8500.0f, 0.707f, fs); airR.set (8500.0f, 0.707f, fs);
        exL.set (3800.0f, 0.8f, fs); exR.set (3800.0f, 0.8f, fs);
        exHpL.set (7000.0f, 0.707f, fs); exHpR.set (7000.0f, 0.707f, fs);
        deEssHpL.set (p.deEssHz * 0.78f, 0.707f, fs); deEssHpR.set (p.deEssHz * 0.78f, 0.707f, fs);
        crystalAir = p.crystal * (p.algo == Algo::Plate ? 0.75f : 0.55f);

        // --- konvertlar ---
        envAtk = std::exp (-1.0f / (0.004f * fs));
        envDry.setMs (70.0f, fs);
        envFull.setMs (12.0f, fs); envSib.setMs (6.0f, fs);
        duckAtk = std::exp (-1.0f / (0.012f * fs));
        duckSm.setMs (180.0f, fs);              // "bloom" — ibora tugagach dum gullaydi
        if (snap) duckSm.z = 1.0f;

        // --- aralash: vokal uchun 50% gacha dry 0 dB ---
        const float m = std::clamp (p.mix, 0.0f, 1.0f);
        dryT = std::min (1.0f, 2.0f * (1.0f - m));
        wetT = std::min (1.0f, 2.0f * m);
        if (snap) { dryG = dryT; wetG = wetT; }
    }
};
} // namespace az
