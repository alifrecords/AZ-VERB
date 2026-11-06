#include "PluginProcessor.h"
#include "PluginEditor.h"

AZVerbProcessor::AZVerbProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      juce::Thread ("AZ VERB Smart Vocal"),
      apvts (*this, &undo, "AZVERB", azp::createLayout())
{
    const char* ids[20] = { azp::algo, azp::predelay, azp::preNote, azp::sync, azp::size, azp::decay, azp::diffusion, azp::damp,
                            azp::lowcut, azp::highcut, azp::width, azp::mix, azp::freeze, azp::duck, azp::duckDb,
                            azp::sand, azp::crystal, azp::focus, azp::deess, azp::deessHz };
    for (int i = 0; i < 20; ++i) p_[i] = apvts.getRawParameterValue (ids[i]);
    abState[0] = apvts.copyState(); abState[1] = apvts.copyState();
}

AZVerbProcessor::~AZVerbProcessor() { stopThread (4000); }

bool AZVerbProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet(), in = l.getMainInputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono()) return false;
    if (in  != juce::AudioChannelSet::stereo() && in  != juce::AudioChannelSet::mono()) return false;
    return ! (in == juce::AudioChannelSet::stereo() && out == juce::AudioChannelSet::mono());
}

void AZVerbProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    engine.prepare (sampleRate, samplesPerBlock);
    updateEngineParams();
    engine.reset();
    capture.assign ((size_t) (sampleRate * 20.0), 0.0f);
    captureLen = 0;
}

void AZVerbProcessor::updateEngineParams()
{
    az::Params p;
    p.algo      = (az::Algo) juce::jlimit (0, 6, (int) p_[P_algo]->load());
    p.size      = p_[P_size]->load();
    p.decayS    = p_[P_decay]->load();
    p.diffusion = p_[P_diff]->load();
    p.dampHz    = p_[P_damp]->load();
    p.lowCutHz  = p_[P_lc]->load();
    p.highCutHz = p_[P_hc]->load();
    p.width     = p_[P_width]->load();
    p.mix       = p_[P_mix]->load();
    p.freeze    = p_[P_freeze]->load() > 0.5f;
    p.duck      = p_[P_duck]->load() > 0.5f;
    p.duckDb    = p_[P_duckDb]->load();
    p.sand      = p_[P_sand]->load();
    p.crystal   = p_[P_crystal]->load();
    p.focus     = p_[P_focus]->load();
    p.deEss     = p_[P_deess]->load();
    p.deEssHz   = p_[P_deessHz]->load();

    if (p_[P_sync]->load() > 0.5f)
    {
        const double bpm = hostBpm.load() > 20.0 ? hostBpm.load() : 120.0;
        const int ni = juce::jlimit (0, 7, (int) p_[P_note]->load());
        p.preDelayMs = (float) (azp::noteBeats[ni] * 60000.0 / bpm);
    }
    else p.preDelayMs = p_[P_pre]->load();

    engine.setParams (p);
}

void AZVerbProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto t0 = juce::Time::getHighResolutionTicks();
    const int n = buffer.getNumSamples();
    const int inCh = getTotalNumInputChannels(), outCh = getTotalNumOutputChannels();
    for (int c = inCh; c < outCh; ++c) buffer.clear (c, 0, n);
    if (n == 0) return;

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm()) hostBpm = *b;

    updateEngineParams();

    float* L = buffer.getWritePointer (0);
    float* R = outCh > 1 ? buffer.getWritePointer (1) : nullptr;
    if (inCh == 1 && R != nullptr) buffer.copyFrom (1, 0, buffer, 0, 0, n);

    // kirish metri + Smart Vocal yozuvi
    float pkL = 0, pkR = 0;
    const bool lis = listening.load();
    int cl = captureLen.load(); int vs = voicedSamples.load();
    double blockEnergy = 0; int written = 0;
    for (int i = 0; i < n; ++i)
    {
        const float l = L[i], r = R ? R[i] : l;
        pkL = std::max (pkL, std::abs (l)); pkR = std::max (pkR, std::abs (r));
        if (lis && cl < (int) capture.size())
        {
            const float m = 0.5f * (l + r);
            capture[(size_t) cl++] = m; blockEnergy += (double) m * m; ++written;
        }
    }
    // blok RMS -55 dBFS dan baland bo'lsa — vokal bor deb hisoblanadi
    if (written > 0 && blockEnergy / written > 3.2e-6) vs += written;
    inPeak[0] = std::max (inPeak[0].load(), pkL); inPeak[1] = std::max (inPeak[1].load(), pkR);
    liveLevel = std::max (pkL, pkR);
    if (lis)
    {
        captureLen = cl; voicedSamples = vs;
        if ((vs >= (int) (listenTargetSeconds * sr * 0.6) && cl >= (int) (listenTargetSeconds * sr)) || cl >= (int) capture.size())
        {
            listening = false; analysing = true;
            notify();  // fon oqimi tahlilni boshlaydi
        }
    }

    engine.process (L, R, n);

    float oL = 0, oR = 0;
    for (int i = 0; i < n; ++i) { oL = std::max (oL, std::abs (L[i])); if (R) oR = std::max (oR, std::abs (R[i])); }
    outPeak[0] = std::max (outPeak[0].load(), oL); outPeak[1] = std::max (outPeak[1].load(), R ? oR : oL);

    const double secs = juce::Time::highResolutionTicksToSeconds (juce::Time::getHighResolutionTicks() - t0);
    const float load = (float) (secs / ((double) n / sr));
    cpuLoad = cpuLoad.load() * 0.95f + load * 0.05f;
}

// ---------------- Smart Vocal -------------------------------------------------
void AZVerbProcessor::startListening()
{
    if (analysing) return;
    if (! isThreadRunning()) startThread (juce::Thread::Priority::low);
    captureLen = 0; voicedSamples = 0;
    listening = true;
}

// Tinglash paytida bosilsa: 2 s dan ko'p ovoz yig'ilgan bo'lsa — darhol tahlil, aks holda bekor
void AZVerbProcessor::cancelListening()
{
    if (! listening) return;
    listening = false;
    if (voicedSamples.load() > (int) (2.0 * sr)) { analysing = true; notify(); }
}

void AZVerbProcessor::run()
{
    while (! threadShouldExit())
    {
        wait (-1);
        if (threadShouldExit()) break;
        if (! analysing) continue;
        std::vector<float> copy (capture.begin(), capture.begin() + captureLen.load());
        auto vp = VocalAnalyzer::analyse (copy, sr, hostBpm.load());
        auto rr = vp.valid ? VocalAnalyzer::recommend (vp) : std::vector<Recommendation>();
        {
            const juce::ScopedLock sl (resultLock);
            profile = vp; recs = rr; smartIndex = 0; smartApplied = false;
        }
        analysing = false;
        ++resultVersion;
    }
}

void AZVerbProcessor::applyRecommendation (int i)
{
    Recipe r;
    {
        const juce::ScopedLock sl (resultLock);
        if (i < 0 || i >= (int) recs.size()) return;
        r = recs[(size_t) i].recipe;
    }
    if (! smartApplied || smartShowingBefore) smartBefore = apvts.copyState();
    undo.beginNewTransaction ("Smart Vocal");
    applyRecipe (r);
    smartAfter = apvts.copyState();
    smartApplied = true; smartShowingBefore = false;
}

void AZVerbProcessor::toggleSmartAB()
{
    if (! smartApplied) return;
    if (smartShowingBefore) { apvts.replaceState (smartAfter.createCopy()); smartShowingBefore = false; }
    else                    { smartAfter = apvts.copyState(); apvts.replaceState (smartBefore.createCopy()); smartShowingBefore = true; }
}

// ---------------- Presetlar va A/B --------------------------------------------
static void setParam (juce::AudioProcessorValueTreeState& s, const char* id, float v)
{
    if (auto* p = s.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (v));
        p->endChangeGesture();
    }
}

void AZVerbProcessor::applyRecipe (const Recipe& r)
{
    setParam (apvts, azp::algo, (float) r.algo);
    setParam (apvts, azp::size, r.size);       setParam (apvts, azp::decay, r.decay);
    setParam (apvts, azp::diffusion, r.diffusion); setParam (apvts, azp::damp, r.damp);
    setParam (apvts, azp::lowcut, r.lowcut);   setParam (apvts, azp::highcut, r.highcut);
    setParam (apvts, azp::width, r.width);     setParam (apvts, azp::mix, r.mix);
    setParam (apvts, azp::sand, r.sand);       setParam (apvts, azp::crystal, r.crystal);
    setParam (apvts, azp::focus, r.focus);     setParam (apvts, azp::deess, r.deess);
    setParam (apvts, azp::deessHz, r.deessHz); setParam (apvts, azp::duck, r.duck ? 1.0f : 0.0f);
    setParam (apvts, azp::duckDb, r.duckDb);   setParam (apvts, azp::sync, r.sync ? 1.0f : 0.0f);
    setParam (apvts, azp::preNote, (float) r.note); setParam (apvts, azp::predelay, r.predelayMs);
    setParam (apvts, azp::freeze, 0.0f);
}

void AZVerbProcessor::loadPreset (int index)
{
    const auto& f = presets::factory();
    currentPreset = juce::jlimit (0, (int) f.size() - 1, index);
    undo.beginNewTransaction ("Preset");
    applyRecipe (f[(size_t) currentPreset]);
    presetModified = false;
}

void AZVerbProcessor::selectAB (int slot)
{
    if (slot == abSlot) return;
    abState[abSlot] = apvts.copyState();
    abSlot = slot;
    apvts.replaceState (abState[abSlot].createCopy());
}

// ---------------- Holatni saqlash ---------------------------------------------
void AZVerbProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto st = apvts.copyState();
    st.setProperty ("preset", currentPreset, nullptr);
    st.setProperty ("version", JucePlugin_VersionString, nullptr);
    if (auto xml = st.createXml()) copyXmlToBinary (*xml, dest);
}

void AZVerbProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            currentPreset = (int) vt.getProperty ("preset", 0);
            apvts.replaceState (vt);
        }
}

juce::AudioProcessorEditor* AZVerbProcessor::createEditor() { return new AZVerbEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AZVerbProcessor(); }
