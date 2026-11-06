#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace ui
{
    const juce::Colour bg0 { 0xff09090a }, bg1 { 0xff0e0c0d }, bg2 { 0xff1a1112 };
    const juce::Colour red { 0xffff3347 }, redHot { 0xffff2a3d }, redSoft { 0xffff8a95 };
    const juce::Colour text { 0xffe9e6e6 }, dim { 0xff8d8587 }, dimmer { 0xff6f686a }, silver { 0xffd9d3d4 }, green { 0xff3ddc97 };
    juce::Font font (float h, int weight = 400, float kern = 0.0f);   // Inter (ichki shrift)
}

class AZLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AZLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;
    juce::Font getPopupMenuFont() override { return ui::font (14.0f); }
};

// Ruchka + qiymat + nom
class Knob : public juce::Component
{
public:
    Knob (AZVerbProcessor&, const juce::String& paramId, const juce::String& label, juce::Colour accent, bool big = false);
    void attachTo (const juce::String& paramId);
    void resized() override;
    void paint (juce::Graphics&) override;
    juce::Slider slider;
private:
    AZVerbProcessor& proc;
    juce::String pid, label; bool big;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
};

class MeterView : public juce::Component
{
public:
    explicit MeterView (AZVerbProcessor& p) : proc (p) {}
    void tick();
    void paint (juce::Graphics&) override;
private:
    AZVerbProcessor& proc; float lv[4] {};
};

class StageView : public juce::Component
{
public:
    explicit StageView (AZVerbProcessor& p);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    Knob sand, crystal, focus, deess;
private:
    AZVerbProcessor& proc; float dragStartSize = 0, dragStartDecay = 0;
    juce::RangedAudioParameter* sizeP; juce::RangedAudioParameter* decayP;
};

class SmartPanel : public juce::Component, private juce::Timer
{
public:
    explicit SmartPanel (AZVerbProcessor& p);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;
private:
    void timerCallback() override;
    juce::String recipeLine (const Recipe&) const;
    AZVerbProcessor& proc;
    juce::TextButton apply { "QO'LLASH" }, ab { "A/B" }, next { "KEYINGI" };
    float hist[34] {}; int histPos = 0; int seenVersion = -1;
    juce::Rectangle<int> listenBox, altBoxes[3];
    VocalProfile prof; std::vector<Recommendation> recs;
};

class AZVerbEditor : public juce::AudioProcessorEditor,
                     private juce::AudioProcessorValueTreeState::Listener,
                     private juce::AsyncUpdater, private juce::Timer
{
public:
    explicit AZVerbEditor (AZVerbProcessor&);
    ~AZVerbEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct MainView : juce::Component
    {
        explicit MainView (AZVerbEditor& e) : ed (e) { setOpaque (true); }
        void paint (juce::Graphics& g) override;
        void mouseUp (const juce::MouseEvent&) override;
        AZVerbEditor& ed;
    };
    void parameterChanged (const juce::String&, float) override { triggerAsyncUpdate(); }
    void handleAsyncUpdate() override;
    void timerCallback() override;
    void showPresetMenu();
    void layout();

    AZVerbProcessor& proc;
    AZLookAndFeel lnf;
    MainView view { *this };

    juce::TextButton prevBtn { "prev" }, nextBtn { "next" }, aBtn { "A" }, bBtn { "B" }, undoBtn { "undo" }, redoBtn { "redo" };
    juce::Rectangle<int> presetBox, cpuBox;
    juce::OwnedArray<juce::TextButton> algoBtns;
    StageView stage;
    MeterView meters;
    juce::ToggleButton freezeBtn { "FREEZE" }, duckBtn { "DUCK" }, syncBtn { "SYNC" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAtt, duckAtt, syncAtt;
    SmartPanel smart;
    juce::OwnedArray<Knob> knobs; Knob* preKnob = nullptr;
    bool lastSync = true; float cpuShown = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AZVerbEditor)
};
