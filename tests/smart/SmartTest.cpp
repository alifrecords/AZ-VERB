#include "PluginProcessor.h"
// Sintetik vokal: glottal impulslar + formantlar + vibrato + "s" sibilyant
static std::vector<float> makeVocal (double fs, double f0, double formShift, double sibHz, double bpm, double secs)
{
    std::vector<float> x ((size_t) (fs * secs)); juce::Random r (1);
    const double F[3] = { 650 * formShift, 1100 * formShift, 2800 * formShift };
    double ph = 0; float s1[3] = {}, s2[3] = {}, bs1 = 0, bs2 = 0;
    const double beat = 60.0 / bpm;
    for (size_t i = 0; i < x.size(); ++i)
    {
        const double t = (double) i / fs, inBeat = std::fmod (t, beat);
        const bool sing = inBeat < beat * 0.8;
        const double f = f0 * (1 + 0.012 * std::sin (2 * M_PI * 5.5 * t));
        ph += f / fs; float src = 0; if (ph >= 1) { ph -= 1; src = 1; }
        float v = 0;
        for (int k = 0; k < 3; ++k) { // rezonator
            const double w = 2 * M_PI * F[k] / fs, rr = 0.97; float y = (float) (src + 2 * rr * std::cos (w) * s1[k] - rr * rr * s2[k]);
            s2[k] = s1[k]; s1[k] = y; v += y * (k == 0 ? 1.0f : k == 1 ? 0.6f : 0.3f); }
        v *= sing ? 0.02f : 0.0f;
        { const float n = r.nextFloat() * 2 - 1; const double w = 2 * M_PI * sibHz / fs, rr = 0.93; float y = (float) (n + 2 * rr * std::cos (w) * bs1 - rr * rr * bs2); bs2 = bs1; bs1 = y; if (inBeat < 0.07) v += 0.03f * y; }
        x[i] = v;
    }
    return x;
}

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    struct Case { const char* name; double f0, form, sib, bpm; };
    for (auto c : { Case { "Erkak bariton, sekin ballada", 130, 0.9, 6200, 72 },
                    Case { "Ayol soprano, tez pop/trap", 330, 1.2, 8200, 140 } })
    {
        AZVerbProcessor p; const double fs = 48000; p.setPlayConfigDetails (2, 2, fs, 512); p.prepareToPlay (fs, 512);
        auto v = makeVocal (fs, c.f0, c.form, c.sib, c.bpm, 12);
        juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m;
        p.startListening();
        for (size_t s = 0; s + 512 <= v.size(); s += 512)
        { for (int i = 0; i < 512; ++i) b.setSample (0, i, v[s + i]), b.setSample (1, i, v[s + i]); p.processBlock (b, m); }
        for (int w = 0; w < 100 && (p.analysing || p.listening); ++w) juce::Thread::sleep (50);
        const juce::ScopedLock sl (p.resultLock);
        auto& pr = p.profile;
        printf ("\n=== %s ===\n", c.name);
        printf ("Ovoz: %s (%s) | Tembr: %s (%s) | Sibilyant: %.0f Hz %s | %0.f BPM %s\n",
            pr.voiceType.toRawUTF8(), pr.voiceSub.toRawUTF8(), pr.timbre.toRawUTF8(), pr.timbreSub.toRawUTF8(), pr.sibHz, pr.sibLabel.toRawUTF8(), pr.bpm, pr.style.toRawUTF8());
        for (size_t i = 0; i < std::min<size_t> (4, p.recs.size()); ++i)
        { auto& r = p.recs[i].recipe; printf ("  %d%%  %-34s decay %.2fs  damp %.0f  lowcut %.0f  de-ess %.0f Hz %.0f%%  crystal %.0f%%\n",
            p.recs[i].matchPct, r.name.toRawUTF8(), r.decay, r.damp, r.lowcut, r.deessHz, r.deess*100, r.crystal*100); }
        p.applyRecommendation (0);
        printf ("Qo'llandi -> algo=%s decay=%.2f, A/B: ", p.apvts.getParameter ("algo")->getCurrentValueAsText().toRawUTF8(), p.apvts.getRawParameterValue ("decay")->load());
        p.toggleSmartAB(); printf ("oldingi decay=%.2f -> ", p.apvts.getRawParameterValue ("decay")->load());
        p.toggleSmartAB(); printf ("yana %.2f\n", p.apvts.getRawParameterValue ("decay")->load());
        juce::MemoryBlock mb; p.getStateInformation (mb); AZVerbProcessor q; q.setStateInformation (mb.getData(), (int) mb.getSize());
        printf ("Holat saqlash/tiklash: decay %.2f == %.2f\n", p.apvts.getRawParameterValue ("decay")->load(), q.apvts.getRawParameterValue ("decay")->load());
    }
}
