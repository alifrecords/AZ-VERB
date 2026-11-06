#pragma once
#include "Params.h"

// "Retsept" — ham zavod presetlari, ham Smart Vocal tavsiyalari uchun bir xil format.
struct Recipe
{
    juce::String name, category;
    int   algo;          // azp::algoNames indeksi
    float size, decay, diffusion, damp, lowcut, highcut, width, mix;
    float sand, crystal, focus, deess, deessHz;
    bool  duck; float duckDb;
    bool  sync; int note; float predelayMs;
};

namespace presets
{
// Algoritm: 0 Hall, 1 Church, 2 Plate, 3 Room I, 4 Room II, 5 Ambient, 6 NonLin
// Note: 0 1/64, 1 1/32T, 2 1/32, 3 1/16T, 4 1/16, 5 1/8T, 6 1/8, 7 1/4
inline const std::vector<Recipe>& factory()
{
    static const std::vector<Recipe> list {
        //  name                          cat      al  size  decay diff  damp    lcut  hcut    width mix   sand crys focus deess dHz    duck dDb  sync note pre
        { juce::String::fromUTF8 ("Warm Plate — Pop Lead"),        "VOCAL",  2, 0.50f, 2.40f, 0.78f, 6200, 180, 15600, 1.06f, 0.32f, 0.32f, 0.40f, 0.50f, 0.50f, 6500, true, 8, true, 4, 24 },
        { juce::String::fromUTF8 ("Dark Plate — Modern Pop"),      "VOCAL",  2, 0.45f, 1.80f, 0.80f, 5200, 220, 14300, 1.04f, 0.20f, 0.24f, 0.25f, 0.65f, 0.60f, 6500, true, 7, true, 4, 30 },
        { juce::String::fromUTF8 ("Crystal Plate — Airy Pop"),     "VOCAL",  2, 0.40f, 1.50f, 0.85f, 9500, 300, 18000, 1.09f, 0.18f, 0.17f, 0.70f, 0.60f, 0.70f, 7000, true, 6, true, 4, 20 },
        { juce::String::fromUTF8 ("Sand Hall — Ballad"),           "VOCAL",  0, 0.70f, 3.20f, 0.80f, 5000, 200, 14300, 1.10f, 0.24f, 0.52f, 0.30f, 0.55f, 0.55f, 6500, true, 9, true, 6, 60 },
        { juce::String::fromUTF8 ("Silk Hall — R&B"),              "VOCAL",  0, 0.55f, 2.20f, 0.75f, 6800, 240, 16900, 1.12f, 0.20f, 0.42f, 0.45f, 0.60f, 0.55f, 6800, true, 8, true, 4, 35 },
        { juce::String::fromUTF8 ("Intimate Room — Rap"),          "VOCAL",  4, 0.30f, 0.70f, 0.70f, 7500, 250, 15600, 1.00f, 0.14f, 0.21f, 0.30f, 0.70f, 0.60f, 6500, true, 6, false, 4, 8  },
        { juce::String::fromUTF8 ("Gated Air — Trap / Drill"),     "VOCAL",  6, 0.30f, 0.60f, 0.80f, 8000, 300, 18000, 1.15f, 0.12f, 0.14f, 0.50f, 0.70f, 0.60f, 7000, true, 10, true, 4, 15 },
        { juce::String::fromUTF8 ("Ambient Wash — Hook"),          "VOCAL",  5, 0.70f, 4.50f, 0.85f, 6000, 260, 15600, 1.15f, 0.22f, 0.59f, 0.45f, 0.70f, 0.55f, 6500, true, 11, true, 7, 90 },
        { juce::String::fromUTF8 ("Sand Hall — Strings & Pads"),   "INSTR",  0, 0.85f, 4.00f, 0.80f, 5500, 120, 15600, 1.15f, 0.35f, 0.56f, 0.30f, 0.20f, 0.20f, 7000, false, 6, false, 4, 25 },
        { juce::String::fromUTF8 ("Cathedral — Choir"),            "INSTR",  1, 0.80f, 5.50f, 0.75f, 4500, 150, 13000, 1.12f, 0.35f, 0.42f, 0.20f, 0.20f, 0.30f, 6500, false, 6, false, 4, 40 },
        { juce::String::fromUTF8 ("Tight Room — Drums"),           "INSTR",  3, 0.35f, 0.60f, 0.70f, 8000, 120, 18000, 1.03f, 0.20f, 0.10f, 0.30f, 0.30f, 0.00f, 6500, false, 6, false, 4, 4  },
        { juce::String::fromUTF8 ("Studio Room — Guitar"),         "INSTR",  4, 0.45f, 1.10f, 0.75f, 7000, 150, 15600, 1.09f, 0.25f, 0.24f, 0.35f, 0.30f, 0.20f, 6500, false, 6, false, 4, 12 },
        { juce::String::fromUTF8 ("Bright Plate — Snare"),         "INSTR",  2, 0.40f, 1.30f, 0.90f, 11000, 200, 18000, 1.06f, 0.25f, 0.14f, 0.60f, 0.20f, 0.00f, 6500, false, 6, false, 4, 6  },
        { juce::String::fromUTF8 ("80s Gate — Snare"),             "INSTR",  6, 0.35f, 0.50f, 0.85f, 9000, 150, 18000, 1.12f, 0.30f, 0.07f, 0.40f, 0.00f, 0.00f, 6500, false, 6, false, 4, 2  },
        { juce::String::fromUTF8 ("Ambient — Keys"),               "INSTR",  5, 0.60f, 3.50f, 0.85f, 6500, 140, 15600, 1.15f, 0.35f, 0.56f, 0.40f, 0.20f, 0.20f, 6500, false, 6, false, 4, 30 },
        { juce::String::fromUTF8 ("Infinite Sand — FX"),           "FX",     5, 1.00f, 15.00f, 1.00f, 5000, 200, 11700, 1.15f, 0.50f, 0.70f, 0.30f, 0.00f, 0.30f, 6500, false, 6, false, 4, 0  },
    };
    return list;
}
}
