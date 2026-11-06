#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/AZVerbEngine.h"
#include "Params.h"
#include "Presets.h"
#include "VocalAnalyzer.h"

class AZVerbProcessor : public juce::AudioProcessor, private juce::Thread
{
public:
    AZVerbProcessor();
    ~AZVerbProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "AZ VERB"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return (int) presets::factory().size(); }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram (int i) override { loadPreset (i); }
    const juce::String getProgramName (int i) override { return presets::factory()[(size_t) juce::jlimit (0, getNumPrograms() - 1, i)].name; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ---- UI uchun ----
    juce::UndoManager undo;
    juce::AudioProcessorValueTreeState apvts;

    void loadPreset (int index);
    void applyRecipe (const Recipe& r);
    int  currentPreset = 0;
    bool presetModified = false;

    // A/B
    void selectAB (int slot);
    int  abSlot = 0;

    // Metrlar va CPU
    std::atomic<float> inPeak[2] {}, outPeak[2] {}, cpuLoad { 0.0f }, liveLevel { 0.0f };
    std::atomic<double> hostBpm { 0.0 };

    // Smart Vocal
    void startListening();
    void cancelListening();
    std::atomic<bool> listening { false }, analysing { false };
    std::atomic<int>  voicedSamples { 0 };
    double  listenTargetSeconds = 8.0;
    double  getSampleRateSafe() const { return sr; }
    double  captureLenSeconds (double rate) const { return rate > 0 ? captureLen.load() / rate : 0.0; }

    juce::CriticalSection resultLock;
    VocalProfile profile;
    std::vector<Recommendation> recs;
    std::atomic<int> resultVersion { 0 };

    // Tavsiyani qo'llash / solishtirish
    void applyRecommendation (int i);
    void toggleSmartAB();
    bool smartApplied = false, smartShowingBefore = false;
    int  smartIndex = 0;

private:
    void run() override;   // tahlil oqimi
    void updateEngineParams();

    az::Engine engine;
    double sr = 48000.0;

    std::vector<float> capture; std::atomic<int> captureLen { 0 };
    juce::ValueTree abState[2], smartBefore, smartAfter;

    std::atomic<float>* p_[20] {};
    enum { P_algo, P_pre, P_note, P_sync, P_size, P_decay, P_diff, P_damp, P_lc, P_hc, P_width, P_mix, P_freeze, P_duck, P_duckDb, P_sand, P_crystal, P_focus, P_deess, P_deessHz };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AZVerbProcessor)
};
