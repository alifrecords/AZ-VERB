// AZ VERB yadrosi uchun oflayn test: impuls javobi, RT60, barqarorlik, CPU.
#include "../Source/dsp/AZVerbEngine.h"
#include <cstdio>
#include <chrono>
#include <string>

static void writeWav (const std::string& path, const std::vector<float>& L, const std::vector<float>& R, int fs)
{
    FILE* f = fopen (path.c_str(), "wb"); if (! f) return;
    const int n = (int) L.size(); const int bytes = n * 4;
    auto w32 = [&] (uint32_t v) { fwrite (&v, 4, 1, f); }; auto w16 = [&] (uint16_t v) { fwrite (&v, 2, 1, f); };
    fwrite ("RIFF", 1, 4, f); w32 (36 + bytes); fwrite ("WAVEfmt ", 1, 8, f); w32 (16); w16 (1); w16 (2); w32 (fs); w32 (fs * 4); w16 (4); w16 (16);
    fwrite ("data", 1, 4, f); w32 (bytes);
    for (int i = 0; i < n; ++i) { int16_t a = (int16_t) std::clamp (L[i] * 32767.0f, -32768.0f, 32767.0f), b = (int16_t) std::clamp (R[i] * 32767.0f, -32768.0f, 32767.0f); fwrite (&a, 2, 1, f); fwrite (&b, 2, 1, f); }
    fclose (f);
}

int main()
{
    const int fs = 48000;
    const char* names[] = { "Hall", "Church", "Plate", "Room1", "Room2", "Ambient", "NonLin" };
    printf ("%-8s %8s %8s %8s %8s %8s %6s\n", "algo", "peak", "energy", "RT60set", "RT60mes", "corrLR", "ok");
    for (int a = 0; a < 7; ++a)
    {
        az::Engine e; e.prepare (fs, 512);
        az::Params p; p.algo = (az::Algo) a; p.mix = 1.0f; p.decayS = 2.0f; p.preDelayMs = 0; p.duck = false; p.focus = 0; p.width = 1.0f;
        p.lowCutHz = 20; p.highCutHz = 20000; p.crystal = 0; e.setParams (p); e.reset(); e.setParams (p);
        const int n = fs * 6; std::vector<float> L (n, 0), R (n, 0); L[0] = R[0] = 1.0f;
        for (int i = 0; i < n; i += 512) e.process (L.data() + i, R.data() + i, std::min (512, n - i));
        double en = 0, pk = 0, lr = 0, ll = 0, rr = 0; bool ok = true;
        for (int i = 0; i < n; ++i) { if (! std::isfinite (L[i]) || ! std::isfinite (R[i])) ok = false; en += L[i] * L[i] + R[i] * R[i]; pk = std::max (pk, (double) std::fabs (L[i])); lr += L[i] * R[i]; ll += L[i] * L[i]; rr += R[i] * R[i]; }
        // Schroeder integratsiyasi -> -5..-25 dB qiyaligi
        std::vector<double> sch (n); double acc = 0; for (int i = n - 1; i >= 0; --i) { acc += L[i] * L[i] + R[i] * R[i]; sch[i] = acc; }
        int i5 = -1, i25 = -1; for (int i = 0; i < n; ++i) { double db = 10 * std::log10 (sch[i] / sch[0] + 1e-30); if (i5 < 0 && db < -5) i5 = i; if (i25 < 0 && db < -25) { i25 = i; break; } }
        double rt = (i5 > 0 && i25 > 0) ? 3.0 * (i25 - i5) / (double) fs : -1;
        printf ("%-8s %8.3f %8.3f %8.2f %8.2f %8.2f %6s\n", names[a], pk, en, p.decayS, rt, lr / std::sqrt (ll * rr + 1e-20), ok ? "ha" : "XATO");
        writeWav (std::string ("ir_") + names[a] + ".wav", L, R, fs);
    }

    // Barqarorlik: 60 s oq shovqin, maksimal sozlamalar
    {
        az::Engine e; e.prepare (fs, 512); az::Params p; p.algo = az::Algo::Church; p.size = 1; p.decayS = 20; p.diffusion = 1; p.sand = 1; p.crystal = 1; p.mix = 1; e.setParams (p);
        az::Rng r; std::vector<float> L (512), R (512); double mx = 0;
        for (int b = 0; b < fs * 60 / 512; ++b) { for (int i = 0; i < 512; ++i) { L[i] = r.next() * 0.3f; R[i] = r.next() * 0.3f; } e.process (L.data(), R.data(), 512); for (float v : L) mx = std::max (mx, (double) std::fabs (v)); }
        printf ("Stress (Church, 20 s, 60 s shovqin): max=%.2f %s\n", mx, (mx < 8 && std::isfinite (mx)) ? "barqaror" : "BEQAROR");
        p.freeze = true; e.setParams (p);
        for (int b = 0; b < fs * 30 / 512; ++b) { for (int i = 0; i < 512; ++i) { L[i] = 0; R[i] = 0; } e.process (L.data(), R.data(), 512); }
        double m2 = 0; for (float v : L) m2 = std::max (m2, (double) std::fabs (v));
        printf ("Freeze 30 s keyin: max=%.3f %s\n", m2, (m2 < 8 && m2 > 0.001) ? "ushlab turibdi" : "MUAMMO");
    }

    // CPU: 10 s stereo 48k
    {
        az::Engine e; e.prepare (fs, 256); az::Params p; p.algo = az::Algo::Hall; e.setParams (p);
        std::vector<float> L (256, 0.1f), R (256, 0.1f);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (int b = 0; b < fs * 10 / 256; ++b) e.process (L.data(), R.data(), 256);
        double sec = std::chrono::duration<double> (std::chrono::high_resolution_clock::now() - t0).count();
        printf ("CPU: 10 s audio %.3f s da => bir yadroning %.2f%%\n", sec, sec / 10.0 * 100.0);
    }
}
