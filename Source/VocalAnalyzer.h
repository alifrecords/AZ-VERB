// =============================================================================
//  SMART VOCAL — vokal tembri tahlili va zamonaviy reverb tavsiyalari
//  Plagin ichida, oflayn (fon oqimida) ishlaydi: bulut/AI kerak emas,
//  audio oqimga deyarli yuk bermaydi.
// =============================================================================
#pragma once
#include <juce_dsp/juce_dsp.h>
#include "Presets.h"

struct VocalProfile
{
    bool   valid = false;
    float  f0 = 0;              // median asosiy chastota
    juce::String voiceType, voiceSub;
    juce::String timbre, timbreSub;
    float  centroid = 0;
    float  sibHz = 6500, sibShare = 0; juce::String sibLabel;
    double bpm = 0; bool bpmFromHost = false; juce::String style;
    float  bands[6] {};         // 80-250, 250-800, 800-2.5k, 2.5-5k, 5-10k, 10-16k ulushi
};

struct Recommendation { Recipe recipe; int matchPct = 0; juce::String why; };

class VocalAnalyzer
{
public:
    // mono: tinglangan buffer, voicedMask: har namuna uchun emas, kadr bo'yicha hisoblanadi
    static VocalProfile analyse (const std::vector<float>& x, double fs, double hostBpm)
    {
        VocalProfile vp;
        const int order = 12, N = 1 << order, hop = N / 2;
        if ((int) x.size() < N * 4) return vp;

        juce::dsp::FFT fft (order);
        std::vector<float> win ((size_t) N), buf ((size_t) N * 2);
        for (int i = 0; i < N; ++i) win[(size_t) i] = 0.5f - 0.5f * std::cos (2.0f * juce::MathConstants<float>::pi * (float) i / (float) N);

        // --- ovozli kadrlarni topish (RMS darvoza: eng baland kadrdan -30 dB) ---
        std::vector<float> frameRms;
        for (size_t s = 0; s + (size_t) N <= x.size(); s += (size_t) hop)
        {
            double e = 0; for (int i = 0; i < N; ++i) e += x[s + (size_t) i] * x[s + (size_t) i];
            frameRms.push_back ((float) std::sqrt (e / N));
        }
        const float peakRms = *std::max_element (frameRms.begin(), frameRms.end());
        const float gate = peakRms * 0.0316f;

        std::vector<double> spec ((size_t) N / 2 + 1, 0.0);
        std::vector<float> f0s;
        int voiced = 0;
        for (size_t fi = 0; fi < frameRms.size(); ++fi)
        {
            if (frameRms[fi] < gate) continue;
            const size_t s = fi * (size_t) hop;
            std::fill (buf.begin(), buf.end(), 0.0f);
            for (int i = 0; i < N; ++i) buf[(size_t) i] = x[s + (size_t) i] * win[(size_t) i];
            fft.performFrequencyOnlyForwardTransform (buf.data(), true);
            for (int k = 0; k <= N / 2; ++k) spec[(size_t) k] += (double) buf[(size_t) k] * buf[(size_t) k];
            const float f = yin (x.data() + s, N, fs);
            if (f > 0) f0s.push_back (f);
            ++voiced;
        }
        if (voiced < 4) return vp;

        // --- spektral ko'rsatkichlar ---
        auto binHz = [&] (int k) { return (double) k * fs / N; };
        const double edges[7] = { 80, 250, 800, 2500, 5000, 10000, 16000 };
        double total = 0, band[6] {}, cNum = 0;
        for (int k = 1; k <= N / 2; ++k)
        {
            const double hz = binHz (k), e = spec[(size_t) k];
            if (hz < 80 || hz > 16000) continue;
            total += e; cNum += hz * e;
            for (int b = 0; b < 6; ++b) if (hz >= edges[b] && hz < edges[b + 1]) band[b] += e;
        }
        if (total <= 0) return vp;
        for (int b = 0; b < 6; ++b) vp.bands[b] = (float) (band[b] / total);
        vp.centroid = (float) (cNum / total);

        // sibilyant cho'qqisi (4.5–10 kHz, silliqlangan)
        double best = 0; int bestK = 0;
        for (int k = 1; k < N / 2 - 8; ++k)
        {
            const double hz = binHz (k); if (hz < 4500 || hz > 10000) continue;
            double sm = 0; for (int j = -6; j <= 6; ++j) sm += spec[(size_t) (k + j)];
            if (sm > best) { best = sm; bestK = k; }
        }
        vp.sibHz = (float) binHz (bestK);
        vp.sibShare = vp.bands[4];
        vp.sibLabel = vp.sibShare > 0.09f ? "kuchli" : vp.sibShare > 0.04f ? "o'rtacha" : "yumshoq";

        // --- ovoz turi (F0 median) ---
        if (! f0s.empty())
        {
            std::sort (f0s.begin(), f0s.end());
            vp.f0 = f0s[f0s.size() / 2];
        }
        const float f = vp.f0;
        if (f <= 0)        { vp.voiceType = "Aniqlanmadi"; vp.voiceSub = "pitch yo'q"; }
        else if (f < 115)  { vp.voiceType = "Bas";      vp.voiceSub = juce::String::fromUTF8 ("erkak · ") + juce::String ((int) f) + " Hz"; }
        else if (f < 160)  { vp.voiceType = "Bariton";  vp.voiceSub = juce::String::fromUTF8 ("erkak · ") + juce::String ((int) f) + " Hz"; }
        else if (f < 215)  { vp.voiceType = "Tenor";    vp.voiceSub = juce::String::fromUTF8 ("erkak · ") + juce::String ((int) f) + " Hz"; }
        else if (f < 300)  { vp.voiceType = "Alt";      vp.voiceSub = juce::String::fromUTF8 ("ayol · ")  + juce::String ((int) f) + " Hz"; }
        else               { vp.voiceType = "Soprano";  vp.voiceSub = juce::String::fromUTF8 ("ayol · ")  + juce::String ((int) f) + " Hz"; }

        // --- tembr ---
        const float warm = vp.bands[1], nasal = vp.bands[2], clar = vp.bands[3];
        if (nasal > 0.45f)                              vp.timbre = "Burunli";
        else if (vp.centroid < 1300 || warm > 0.45f)    vp.timbre = "Iliq, to'q";
        else if (vp.centroid > 2600 || clar > 0.22f)    vp.timbre = "Yorqin";
        else                                            vp.timbre = "Muvozanatli";
        vp.timbreSub = "markaz " + azp::fmtHz (vp.centroid);

        // --- temp ---
        if (hostBpm > 30) { vp.bpm = hostBpm; vp.bpmFromHost = true; }
        else vp.bpm = estimateBpm (x, fs);
        const double b = vp.bpm;
        vp.style = b < 85 ? "Ballada" : b < 116 ? "Pop / R&B" : b < 136 ? "Dance-pop" : "Trap / Drill";
        vp.valid = true;
        return vp;
    }

    // Profilga qarab tavsiyalar (eng moslari birinchi)
    static std::vector<Recommendation> recommend (const VocalProfile& vp)
    {
        struct T { int presetIdx; float bpmLo, bpmHi; float brightPref; float sibTol; juce::String trendName; };
        // brightPref: -1 to'q ovozga mos, +1 yorqin ovozga mos
        const T templates[] = {
            { 1,  80, 130,  0.6f, 0.8f, juce::String::fromUTF8 ("Dark Plate — Modern Pop Lead") },
            { 2,  80, 130, -0.8f, 0.3f, juce::String::fromUTF8 ("Crystal Plate — Airy Pop") },
            { 3,  55,  92, -0.4f, 0.6f, juce::String::fromUTF8 ("Sand Hall — Ballad") },
            { 4,  70, 110, -0.2f, 0.6f, juce::String::fromUTF8 ("Silk Hall — R&B") },
            { 5,  85, 170,  0.2f, 0.8f, juce::String::fromUTF8 ("Intimate Room — Rap / Afrobeats") },
            { 6, 125, 175,  0.0f, 0.7f, juce::String::fromUTF8 ("Gated Air — Trap / Drill") },
            { 7,  60, 140,  0.0f, 0.6f, juce::String::fromUTF8 ("Ambient Wash — Hook / Outro") },
        };
        const float bright = juce::jlimit (-1.0f, 1.0f, (vp.centroid - 1900.0f) / 900.0f);
        const float sib = juce::jlimit (0.0f, 1.0f, vp.sibShare / 0.12f);
        double bpm = vp.bpm > 0 ? vp.bpm : 100.0;

        std::vector<Recommendation> out;
        for (auto& t : templates)
        {
            // diapazon markaziga qanchalik yaqin — shuncha mos
            const double centre = 0.5 * (t.bpmLo + t.bpmHi), half = 0.5 * (t.bpmHi - t.bpmLo);
            double tempoFit = 1.0 - 0.35 * std::min (1.0, std::abs (bpm - centre) / half);
            if (bpm < t.bpmLo) tempoFit = 0.65 * std::exp (-std::pow ((t.bpmLo - bpm) / 18.0, 2));
            if (bpm > t.bpmHi) tempoFit = 0.65 * std::exp (-std::pow ((bpm - t.bpmHi) / 18.0, 2));
            const double toneFit = 1.0 - 0.6 * std::abs (bright - t.brightPref);
            const double sibFit  = 1.0 - 0.6 * std::max (0.0f, sib - t.sibTol);
            const double score = juce::jlimit (0.0, 1.0, 0.6 * tempoFit + 0.28 * toneFit + 0.12 * sibFit);

            Recommendation r;
            r.recipe = presets::factory()[(size_t) t.presetIdx];
            r.recipe.name = t.trendName;
            tailor (r.recipe, vp, bpm, bright, sib);
            r.matchPct = juce::roundToInt (45.0 + 54.0 * score * score);
            r.why = whyText (vp, bright);
            out.push_back (r);
        }
        std::sort (out.begin(), out.end(), [] (auto& a, auto& b) { return a.matchPct > b.matchPct; });
        return out;
    }

private:
    // Retseptni aynan shu ovozga moslash
    static void tailor (Recipe& r, const VocalProfile& vp, double bpm, float bright, float sib)
    {
        // dum tempoga bog'lanadi: ballada ~4 hissa, tez treklar ~2 hissa
        const double beat = 60.0 / bpm;
        const double beats = r.algo == 6 ? 0.0 : (bpm < 90 ? 4.0 : bpm < 120 ? 3.0 : 2.0);
        if (beats > 0) r.decay = (float) juce::jlimit (0.5, 6.0, beat * beats * (r.algo == 5 ? 1.8 : r.algo == 4 ? 0.35 : 1.0));
        // predelay: tez temp -> 1/16, sekin -> 1/8
        r.sync = true; r.note = bpm >= 110 ? 4 : 6;
        if (r.algo == 4) { r.sync = false; r.predelayMs = 8; }
        // yorqin ovoz -> qorong'iroq dum, to'q ovoz -> ko'proq kristal
        r.damp    = juce::jlimit (3500.0f, 14000.0f, r.damp * (1.0f - 0.22f * bright));
        r.crystal = juce::jlimit (0.0f, 1.0f, r.crystal - 0.22f * bright);
        // past ovozlarda low cut pastroq, ayol ovozlarida balandroq
        const float f0 = vp.f0 > 0 ? vp.f0 : 160.0f;
        r.lowcut = juce::jlimit (120.0f, 350.0f, f0 * 1.4f + 20.0f);
        // sibilyant: chastota va kuchi aniqlangan joyga
        r.deessHz = vp.sibHz;
        r.deess = juce::jlimit (0.2f, 0.95f, 0.35f + 0.6f * sib);
        // FOCUS: dumning vokal ostida ko'milmasligi — har doim faol
        r.focus = juce::jlimit (0.4f, 0.85f, r.focus + 0.1f * (1.0f - bright));
    }

    static juce::String whyText (const VocalProfile& vp, float bright)
    {
        return (bright > 0.3f ? juce::String (juce::String::fromUTF8 ("Yorqin ovoz — dum qorong'iroq, sibilyant nazoratda"))
              : bright < -0.3f ? juce::String (juce::String::fromUTF8 ("To'q ovoz — dumga kristal havo qo'shildi"))
                               : juce::String (juce::String::fromUTF8 ("Muvozanatli ovoz — toza, zamonaviy dum")))
              + juce::String::fromUTF8 (" · pre ") + azp::noteNames[vp.bpm >= 110 ? 4 : 6];
    }

    // YIN pitch (2x decimatsiya bilan, tejamkor)
    static float yin (const float* x, int n, double fs)
    {
        const int m = n / 2; std::vector<float> d ((size_t) m);
        for (int i = 0; i < m; ++i) d[(size_t) i] = 0.5f * (x[2 * i] + x[2 * i + 1]);
        const double sr = fs / 2.0;
        const int tauMin = (int) (sr / 900.0), tauMax = std::min (m / 2, (int) (sr / 65.0));
        const int W = m / 2;
        std::vector<float> df ((size_t) tauMax + 1, 0.0f);
        for (int tau = 1; tau <= tauMax; ++tau)
        {
            double s = 0; for (int j = 0; j < W; ++j) { const float a = d[(size_t) j] - d[(size_t) (j + tau)]; s += a * a; }
            df[(size_t) tau] = (float) s;
        }
        double run = 0;
        for (int tau = 1; tau <= tauMax; ++tau)
        {
            run += df[(size_t) tau];
            const float cm = run > 0 ? (float) (df[(size_t) tau] * tau / run) : 1.0f;
            if (tau >= tauMin && cm < 0.15f)
            {
                while (tau + 1 <= tauMax)
                {
                    run += df[(size_t) tau + 1];
                    const float nx = (float) (df[(size_t) tau + 1] * (tau + 1) / run);
                    if (nx < cm) ++tau; else break;
                }
                return (float) (sr / tau);
            }
        }
        return 0.0f;
    }

    // Host temp bermasa: onset avtokorrelyatsiyasi
    static double estimateBpm (const std::vector<float>& x, double fs)
    {
        const int hop = 512; std::vector<float> e;
        for (size_t s = 0; s + (size_t) hop <= x.size(); s += (size_t) hop)
        {
            double a = 0; for (int i = 0; i < hop; ++i) a += x[s + (size_t) i] * x[s + (size_t) i];
            e.push_back ((float) std::log (a + 1e-9));
        }
        std::vector<float> flux (e.size(), 0.0f);
        for (size_t i = 1; i < e.size(); ++i) flux[i] = std::max (0.0f, e[i] - e[i - 1]);
        const double fr = fs / hop; double bestScore = -1, bestBpm = 100;
        for (double bpm = 70; bpm <= 170; bpm += 0.5)
        {
            const int lag = (int) std::round (fr * 60.0 / bpm); double sc = 0;
            for (size_t i = (size_t) lag; i < flux.size(); ++i) sc += flux[i] * flux[i - (size_t) lag];
            sc /= (double) (flux.size() - (size_t) lag + 1);
            if (sc > bestScore) { bestScore = sc; bestBpm = bpm; }
        }
        return bestBpm;
    }
};
