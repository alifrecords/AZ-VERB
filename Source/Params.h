#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace azp
{
    // Parametr identifikatorlari (avtomatizatsiya nomlari)
    inline constexpr const char* algo      = "algo";
    inline constexpr const char* predelay  = "predelay";
    inline constexpr const char* preNote   = "preNote";
    inline constexpr const char* sync      = "sync";
    inline constexpr const char* size      = "size";
    inline constexpr const char* decay     = "decay";
    inline constexpr const char* diffusion = "diffusion";
    inline constexpr const char* damp      = "damp";
    inline constexpr const char* lowcut    = "lowcut";
    inline constexpr const char* highcut   = "highcut";
    inline constexpr const char* width     = "width";
    inline constexpr const char* mix       = "mix";
    inline constexpr const char* freeze    = "freeze";
    inline constexpr const char* duck      = "duck";
    inline constexpr const char* duckDb    = "duckDb";
    inline constexpr const char* sand      = "sand";
    inline constexpr const char* crystal   = "crystal";
    inline constexpr const char* focus     = "focus";
    inline constexpr const char* deess     = "deess";
    inline constexpr const char* deessHz   = "deessHz";

    inline const juce::StringArray algoNames { "Hall", "Church", "Plate", "Room I", "Room II", "Ambient", "NonLin" };
    inline const juce::StringArray noteNames { "1/64", "1/32T", "1/32", "1/16T", "1/16", "1/8T", "1/8", "1/4" };
    inline constexpr double noteBeats[] = { 1.0/16, 1.0/12, 1.0/8, 1.0/6, 1.0/4, 1.0/3, 1.0/2, 1.0 };

    inline juce::NormalisableRange<float> skewed (float lo, float hi, float centre)
    {
        juce::NormalisableRange<float> r (lo, hi); r.setSkewForCentre (centre); return r;
    }

    inline juce::String fmtHz (float v)  { return v >= 1000.0f ? juce::String (v / 1000.0f, v >= 10000.0f ? 1 : 1) + " kHz" : juce::String ((int) v) + " Hz"; }
    inline juce::String fmtPct (float v) { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto F = [&] (const char* id, const char* name, NormalisableRange<float> r, float def, std::function<String (float, int)> fmt)
        {
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, r, def,
                AudioParameterFloatAttributes().withStringFromValueFunction (fmt)));
        };
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { algo, 1 }, "Algorithm", algoNames, 2));
        F (predelay, "Pre-Delay", skewed (0.0f, 500.0f, 60.0f), 24.0f, [] (float v, int) { return String (v, v < 10 ? 1 : 0) + " ms"; });
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { preNote, 1 }, "Pre-Delay Note", noteNames, 4));
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { sync, 1 }, "Sync", true));
        F (size, "Size", NormalisableRange<float> (0.0f, 1.0f), 0.5f, [] (float v, int)
           { return v < 0.2f ? String ("Tiny") : v < 0.4f ? String ("Small") : v < 0.65f ? String ("Medium") : v < 0.85f ? String ("Large") : String ("Huge"); });
        F (decay, "Decay", skewed (0.1f, 20.0f, 2.0f), 2.4f, [] (float v, int) { return String (v, 2) + " s"; });
        F (diffusion, "Diffusion", NormalisableRange<float> (0.0f, 1.0f), 0.78f, [] (float v, int) { return fmtPct (v); });
        F (damp, "Damp HF", skewed (1000.0f, 20000.0f, 6000.0f), 6500.0f, [] (float v, int) { return fmtHz (v); });
        F (lowcut, "Low Cut", skewed (20.0f, 1000.0f, 200.0f), 180.0f, [] (float v, int) { return fmtHz (v); });
        F (highcut, "High Cut", skewed (1000.0f, 20000.0f, 8000.0f), 16000.0f, [] (float v, int) { return fmtHz (v); });
        F (width, "Width", NormalisableRange<float> (0.0f, 2.0f), 1.0f, [] (float v, int) { return fmtPct (v); });
        F (mix, "Mix", NormalisableRange<float> (0.0f, 1.0f), 0.32f, [] (float v, int) { return fmtPct (v); });
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { freeze, 1 }, "Freeze", false));
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { duck, 1 }, "Duck", true));
        F (duckDb, "Duck Depth", NormalisableRange<float> (0.0f, 18.0f), 8.0f, [] (float v, int) { return String (v, 1) + " dB"; });
        F (sand, "Sand", NormalisableRange<float> (0.0f, 1.0f), 0.30f, [] (float v, int) { return fmtPct (v); });
        F (crystal, "Crystal", NormalisableRange<float> (0.0f, 1.0f), 0.40f, [] (float v, int) { return fmtPct (v); });
        F (focus, "Focus", NormalisableRange<float> (0.0f, 1.0f), 0.50f, [] (float v, int) { return fmtPct (v); });
        F (deess, "De-Ess", NormalisableRange<float> (0.0f, 1.0f), 0.50f, [] (float v, int) { return fmtPct (v); });
        F (deessHz, "De-Ess Freq", skewed (3000.0f, 12000.0f, 6500.0f), 6500.0f, [] (float v, int) { return fmtHz (v); });
        return { p.begin(), p.end() };
    }
}
