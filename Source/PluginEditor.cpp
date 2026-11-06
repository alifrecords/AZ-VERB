#include "PluginEditor.h"
#include "BinaryData.h"

using namespace juce;

static constexpr int kW = 980, kH = 700;

// ============================================================================ shrift
Font ui::font (float h, int weight, float kern)
{
    static Typeface::Ptr reg  = Typeface::createSystemTypefaceFor (BinaryData::InterRegular_otf,  BinaryData::InterRegular_otfSize);
    static Typeface::Ptr med  = Typeface::createSystemTypefaceFor (BinaryData::InterMedium_otf,   BinaryData::InterMedium_otfSize);
    static Typeface::Ptr semi = Typeface::createSystemTypefaceFor (BinaryData::InterSemiBold_otf, BinaryData::InterSemiBold_otfSize);
    static Typeface::Ptr bold = Typeface::createSystemTypefaceFor (BinaryData::InterBold_otf,     BinaryData::InterBold_otfSize);
    auto tf = weight >= 700 ? bold : weight >= 600 ? semi : weight >= 500 ? med : reg;
    return Font (FontOptions().withTypeface (tf).withHeight (h)).withExtraKerningFactor (kern);
}

static void spaced (Graphics& g, const String& s, Rectangle<float> r, float h, Colour c, Justification j = Justification::centredLeft, int w = 500, float kern = 0.2f)
{
    g.setColour (c); g.setFont (ui::font (h, w, kern)); g.drawText (s, r, j, false);
}

// ============================================================================ LookAndFeel
AZLookAndFeel::AZLookAndFeel()
{
    setColour (PopupMenu::backgroundColourId, Colour (0xff141112));
    setColour (PopupMenu::textColourId, ui::text);
    setColour (PopupMenu::highlightedBackgroundColourId, ui::redHot.withAlpha (0.25f));
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (PopupMenu::headerTextColourId, ui::redSoft);
}

void AZLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, Slider& s)
{
    const auto accent = Colour ((uint32) (int64) s.getProperties().getWithDefault ("accent", (int64) ui::red.getARGB()));
    const float size = (float) jmin (w, h), cx = (float) x + (float) w * 0.5f, cy = (float) y + (float) h * 0.5f;
    const float R = size * 0.42f, thick = jmax (2.5f, size * 0.05f);
    const float ang = a0 + pos * (a1 - a0);

    Path track; track.addCentredArc (cx, cy, R, R, 0, a0, a1, true);
    g.setColour (Colours::white.withAlpha (0.08f));
    g.strokePath (track, PathStrokeType (thick, PathStrokeType::curved, PathStrokeType::rounded));
    Path val; val.addCentredArc (cx, cy, R, R, 0, a0, ang, true);
    g.setColour (accent);
    g.strokePath (val, PathStrokeType (thick, PathStrokeType::curved, PathStrokeType::rounded));

    const float br = size * 0.32f;
    ColourGradient body (Colour (0xff3a3335), cx - br * 0.3f, cy - br * 0.6f, Colour (0xff151213), cx + br * 0.4f, cy + br, true);
    g.setGradientFill (body); g.fillEllipse (cx - br, cy - br, br * 2, br * 2);
    g.setColour (Colours::white.withAlpha (0.08f)); g.drawEllipse (cx - br, cy - br, br * 2, br * 2, 1.0f);

    const float sx = std::sin (ang), sy = -std::cos (ang);
    g.setColour (Colours::white);
    g.drawLine (cx + sx * br * 0.25f, cy + sy * br * 0.25f, cx + sx * br * 0.7f, cy + sy * br * 0.7f, jmax (2.0f, size * 0.03f));
}

void AZLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool over, bool down)
{
    const auto kind = b.getProperties()["kind"].toString();
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();
    if (kind == "algo")
    {
        if (on)
        {
            g.setGradientFill (ColourGradient (ui::redHot.withAlpha (0.18f), r.getX(), 0, ui::redHot.withAlpha (0.02f), r.getRight(), 0, false));
            g.fillRoundedRectangle (r, 9); g.setColour (ui::redHot.withAlpha (0.35f)); g.drawRoundedRectangle (r, 9, 1);
        }
        else if (over) { g.setColour (Colours::white.withAlpha (0.03f)); g.fillRoundedRectangle (r, 9); }
        auto d = Rectangle<float> (r.getX() + 12, r.getCentreY() - 3, 6, 6);
        if (on) { g.setColour (ui::red.withAlpha (0.35f)); g.fillEllipse (d.expanded (3)); }
        g.setColour (on ? ui::red : Colours::white.withAlpha (0.11f)); g.fillEllipse (d);
        return;
    }
    if (kind == "go")
    {
        g.setColour (b.isEnabled() ? (down ? ui::redHot.darker (0.2f) : over ? ui::red.brighter (0.1f) : ui::redHot) : Colour (0x33ffffff));
        g.fillRoundedRectangle (r, 6); return;
    }
    const float rad = kind == "small" ? 6.0f : 8.0f;
    g.setColour (on ? ui::redHot.withAlpha (0.14f) : Colours::white.withAlpha (over ? 0.06f : 0.025f));
    g.fillRoundedRectangle (r, rad);
    g.setColour (on ? ui::redHot.withAlpha (0.5f) : Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (r, rad, 1);
}

void AZLookAndFeel::drawButtonText (Graphics& g, TextButton& b, bool, bool)
{
    const auto kind = b.getProperties()["kind"].toString();
    const bool on = b.getToggleState();
    if (kind == "algo")
    {
        spaced (g, b.getButtonText(), b.getLocalBounds().toFloat().withTrimmedLeft (27), 12.5f, on ? Colours::white : Colour (0xffa49c9e), Justification::centredLeft, 400, 0);
        return;
    }
    Colour c = kind == "go" ? Colours::white : on ? ui::redSoft : Colour (0xffbdb6b7);
    if (! b.isEnabled()) c = c.withAlpha (0.35f);
    const auto t = b.getButtonText();
    if (t == "prev" || t == "next" || t == "undo" || t == "redo")      // vektor ikonkalar
    {
        auto r = b.getLocalBounds().toFloat();
        const float cx = r.getCentreX(), cy = r.getCentreY();
        Path p;
        if (t == "prev" || t == "next")
        {
            const float d = t == "prev" ? 1.0f : -1.0f;
            p.startNewSubPath (cx + 2.5f * d, cy - 5); p.lineTo (cx - 2.5f * d, cy); p.lineTo (cx + 2.5f * d, cy + 5);
        }
        else
        {
            const bool u = t == "undo"; const float d = u ? 1.0f : -1.0f;
            p.addCentredArc (cx, cy + 1, 6, 5.5f, 0, u ? -2.2f : 2.2f, u ? 1.6f : -1.6f, true);
            const float ax = cx - 5.4f * d, ay = cy - 2.6f;
            Path a; a.startNewSubPath (ax - 3.5f * d, ay - 1.5f); a.lineTo (ax, ay + 0.5f); a.lineTo (ax + 1.0f * d, ay - 3.8f);
            g.setColour (c); g.strokePath (a, PathStrokeType (1.5f, PathStrokeType::curved, PathStrokeType::rounded));
        }
        g.setColour (c); g.strokePath (p, PathStrokeType (1.5f, PathStrokeType::curved, PathStrokeType::rounded));
        return;
    }
    const float h = kind == "small" || kind == "go" ? 9.5f : 12.0f;
    spaced (g, b.getButtonText(), b.getLocalBounds().toFloat(), h, c, Justification::centred, kind == "go" ? 600 : 500, kind == "small" || kind == "go" ? 0.14f : 0);
}

void AZLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& b, bool over, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (Colours::white.withAlpha (over ? 0.04f : 0.02f)); g.fillRoundedRectangle (r, 8);
    g.setColour (Colours::white.withAlpha (0.063f)); g.drawRoundedRectangle (r, 8, 1);
    spaced (g, b.getButtonText(), r.withTrimmedLeft (10), 10.5f, Colour (0xffa49c9e), Justification::centredLeft, 500, 0.16f);
    auto sw = Rectangle<float> (r.getRight() - 34, r.getCentreY() - 6.5f, 24, 13);
    const bool on = b.getToggleState();
    g.setColour (on ? ui::redHot : Colours::white.withAlpha (0.094f)); g.fillRoundedRectangle (sw, 6.5f);
    g.setColour (on ? Colours::white : ui::dim);
    g.fillEllipse (on ? sw.getRight() - 11 : sw.getX() + 2, sw.getY() + 2, 9, 9);
}

// ============================================================================ Knob
Knob::Knob (AZVerbProcessor& p, const String& id, const String& l, Colour accent, bool b) : proc (p), label (l), big (b)
{
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    slider.getProperties().set ("accent", (int64) accent.getARGB());
    slider.setMouseDragSensitivity (big ? 260 : 220);
    slider.onDragStart = [this] { proc.undo.beginNewTransaction(); };
    slider.onValueChange = [this] { repaint(); };
    addAndMakeVisible (slider);
    attachTo (id);
}

void Knob::attachTo (const String& id)
{
    pid = id; att.reset();
    att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, slider);
    if (auto* prm = proc.apvts.getParameter (id))
        slider.setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
    repaint();
}

void Knob::resized()
{
    const int s = big ? 92 : (getHeight() < 90 ? 42 : 64);
    slider.setBounds ((getWidth() - s) / 2, 0, s, s);
}

void Knob::paint (Graphics& g)
{
    auto* prm = proc.apvts.getParameter (pid);
    const String v = prm ? prm->getCurrentValueAsText() : String();
    const int top = slider.getBottom() + (getHeight() < 90 ? 1 : 4);
    const bool small = getHeight() < 90;
    spaced (g, v, Rectangle<float> (0, (float) top, (float) getWidth(), small ? 13.0f : 18.0f), small ? 10.5f : (big ? 15.0f : 12.0f),
            Colours::white, Justification::centred, big ? 600 : 400, 0);
    spaced (g, label, Rectangle<float> (0, (float) top + (small ? 13.0f : 20.0f), (float) getWidth(), 13.0f), small ? 8.5f : 9.5f,
            small ? ui::redSoft : ui::dim, Justification::centred, 500, 0.2f);
}

// ============================================================================ Meter
void MeterView::tick()
{
    const float v[4] = { proc.inPeak[0].exchange (0.0f), proc.inPeak[1].exchange (0.0f), proc.outPeak[0].exchange (0.0f), proc.outPeak[1].exchange (0.0f) };
    bool changed = false;
    for (int i = 0; i < 4; ++i)
    {
        const float db = Decibels::gainToDecibels (v[i], -60.0f);
        const float t = jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
        const float nv = t > lv[i] ? t : lv[i] - 0.025f;
        if (std::abs (nv - lv[i]) > 0.002f) changed = true;
        lv[i] = jmax (0.0f, nv);
    }
    if (changed) repaint();
}

void MeterView::paint (Graphics& g)
{
    const float h = (float) getHeight() - 20;
    const float xs[4] = { 0, 15, 36, 51 };
    for (int i = 0; i < 4; ++i)
    {
        auto r = Rectangle<float> (xs[i] + 12, 0, 8, h);
        g.setColour (Colours::white.withAlpha (0.047f)); g.fillRoundedRectangle (r, 4);
        auto f = r.withTop (r.getBottom() - h * lv[i]);
        if (i < 2) g.setGradientFill (ColourGradient (Colour (0xffa49c9e), 0, f.getY(), Colour (0xff5a5355), 0, r.getBottom(), false));
        else       g.setGradientFill (ColourGradient (Colour (0xffffd0a0), 0, r.getY(), ui::redHot, 0, r.getBottom(), false));
        g.fillRoundedRectangle (f, 4);
    }
    spaced (g, "IN", { 12, h + 6, 23, 12 }, 9, ui::dimmer, Justification::centred);
    spaced (g, "OUT", { 46, h + 6, 25, 12 }, 9, ui::dimmer, Justification::centred);
}

// ============================================================================ Stage (fazo vizuali)
StageView::StageView (AZVerbProcessor& p)
    : sand (p, azp::sand, "SAND", ui::red), crystal (p, azp::crystal, "CRYSTAL", ui::red),
      focus (p, azp::focus, "FOCUS", ui::red), deess (p, azp::deess, "DE-ESS", ui::silver), proc (p)
{
    for (auto* k : { &sand, &crystal, &focus, &deess }) addAndMakeVisible (k);
    sizeP = p.apvts.getParameter (azp::size); decayP = p.apvts.getParameter (azp::decay);
    setMouseCursor (MouseCursor::UpDownLeftRightResizeCursor);
}

void StageView::resized()
{
    int x = getWidth() - 22 - 4 * 58;
    for (auto* k : { &sand, &crystal, &focus, &deess }) { k->setBounds (x, 34, 58, 72); x += 58; }
}

void StageView::mouseDown (const MouseEvent&)
{
    dragStartSize = sizeP->getValue(); dragStartDecay = decayP->getValue();
    proc.undo.beginNewTransaction ("Stage");
    sizeP->beginChangeGesture(); decayP->beginChangeGesture();
}
void StageView::mouseDrag (const MouseEvent& e)
{
    // gorizontal = SIZE, vertikal = DECAY — fazoni "qo'l bilan" cho'zish
    sizeP->setValueNotifyingHost (jlimit (0.0f, 1.0f, dragStartSize + (float) e.getDistanceFromDragStartX() / 400.0f));
    decayP->setValueNotifyingHost (jlimit (0.0f, 1.0f, dragStartDecay - (float) e.getDistanceFromDragStartY() / 300.0f));
}
void StageView::mouseUp (const MouseEvent&) { sizeP->endChangeGesture(); decayP->endChangeGesture(); }

void StageView::paint (Graphics& g)
{
    auto& s = proc.apvts;
    const float size = s.getRawParameterValue (azp::size)->load();
    const float decay = s.getRawParameterValue (azp::decay)->load();
    const float diff = s.getRawParameterValue (azp::diffusion)->load();
    const float sandV = s.getRawParameterValue (azp::sand)->load();
    const bool freeze = s.getRawParameterValue (azp::freeze)->load() > 0.5f;
    const int algo = (int) s.getRawParameterValue (azp::algo)->load();
    const bool sync = s.getRawParameterValue (azp::sync)->load() > 0.5f;
    const double bpm = proc.hostBpm.load() > 20 ? proc.hostBpm.load() : 120.0;
    const float preMs = sync ? (float) (azp::noteBeats[(int) s.getRawParameterValue (azp::preNote)->load()] * 60000.0 / bpm)
                             : s.getRawParameterValue (azp::predelay)->load();

    const float W = (float) getWidth(), H = (float) getHeight();
    const float cx = W * 0.5f, cy = H * 0.456f;

    // perspektiv pol
    g.setColour (Colours::white.withAlpha (0.035f));
    for (int i = 0; i < 14; ++i) { const float y = H * 0.635f + (float) (i * i) * 0.9f; if (y < H) g.drawHorizontalLine ((int) y, 0, W); }
    g.setColour (Colours::white.withAlpha (0.03f));
    for (int i = -10; i <= 10; ++i) g.drawLine (cx + (float) i * 14, H * 0.635f, cx + (float) i * 90, H, 1.0f);

    // halqalar
    const float ringStep = 14.0f + size * 16.0f;
    const float keep = jlimit (0.62f, 0.9f, 0.66f + 0.05f * std::log2 (1.0f + decay));
    for (int i = 1; i <= 11; ++i)
    {
        const float r = (float) i * ringStep;
        const float a = freeze ? 0.35f : 0.55f * std::pow (keep, (float) i);
        g.setColour ((freeze ? Colour (0xff8ab4ff) : ui::red).withAlpha (a));
        if (i % 3 == 0)
        {
            Path p; p.addEllipse (cx - r * 1.35f, cy - r * 0.62f, r * 2.7f, r * 1.24f);
            Path d; const float dash[] = { 2.0f, 5.0f }; PathStrokeType (1.0f).createDashedStroke (d, p, dash, 2); g.fillPath (d);
        }
        else g.drawEllipse (cx - r * 1.35f, cy - r * 0.62f, r * 2.7f, r * 1.24f, i < 3 ? 1.6f : 1.0f);
    }

    // yadro nuri
    ColourGradient core (Colour (0xffff6a55), cx, cy, ui::redHot.withAlpha (0.0f), cx + 70, cy, true);
    core.addColour (0.35, ui::redHot.withAlpha (0.55f));
    g.setGradientFill (core); g.fillEllipse (cx - 70, cy - 70, 140, 140);
    g.setColour (Colours::white); g.fillEllipse (cx - 5, cy - 5, 10, 10);

    // erta aks-sado zarrachalari (zichlik = diffusion, tarqoqlik = sand)
    Random rnd (7 + algo);
    const int count = 40 + (int) (diff * 130);
    for (int i = 0; i < count; ++i)
    {
        const float a = rnd.nextFloat() * MathConstants<float>::twoPi;
        const float d = (40.0f + std::pow (rnd.nextFloat(), 0.7f) * 300.0f) * (0.55f + 0.6f * size);
        const float jit = sandV * 6.0f * (rnd.nextFloat() - 0.5f);
        const float x = cx + std::cos (a) * d * 1.01f + jit, y = cy + std::sin (a) * d * 0.465f + jit;
        const float o = jmax (0.06f, 0.9f - d / 330.0f);
        const float rr = rnd.nextFloat() * 1.6f + 0.5f;
        g.setColour ((i % 9 == 0 ? Colours::white : Colour (0xffff5a66)).withAlpha (o));
        g.fillEllipse (x - rr, y - rr, rr * 2, rr * 2);
    }

    // nurlar
    g.setColour (ui::red.withAlpha (0.18f));
    for (int i = 0; i < 6; ++i)
    {
        const float a = -2.6f + (float) i * 0.55f;
        Path p; p.startNewSubPath (cx, cy); p.lineTo (cx + std::cos (a) * 230, cy + std::sin (a) * 105);
        Path d; const float dash[] = { 1.0f, 4.0f }; PathStrokeType (1.0f).createDashedStroke (d, p, dash, 2); g.fillPath (d);
    }

    // SIZE tutqichi
    const float hx = cx + 7 * ringStep * 1.35f;
    g.setColour (ui::bg1); g.fillEllipse (hx - 6, cy - 6, 12, 12);
    g.setColour (ui::red); g.drawEllipse (hx - 6, cy - 6, 12, 12, 1.5f);
    spaced (g, "SIZE", { hx + 11, cy - 6, 50, 12 }, 9.5f, ui::redSoft, Justification::centredLeft, 500, 0.15f);

    // impuls javobi
    const float x0 = 40, x1 = W - 40, base = H - 32;
    const float shown = jmax (0.4f, (freeze ? 4.0f : decay) * 1.05f + preMs * 0.001f);
    Path ir, fill; ir.startNewSubPath (x0, base);
    for (float x = 0; x <= x1 - x0; x += 3)
    {
        const float t = x / (x1 - x0) * shown - preMs * 0.001f;
        float env = 0;
        if (t >= 0)
        {
            if (algo == 6) { const float w = 0.09f + 0.33f * size; env = t < w * 0.85f ? 0.8f : t < w ? 0.8f * (1 - (t - w * 0.85f) / (w * 0.15f)) : 0; }
            else env = freeze ? 0.55f : std::exp (-t * 6.9f / jmax (0.1f, decay));
            const float att = jlimit (0.0f, 1.0f, t / 0.012f);
            env *= att;
        }
        const float jit = (std::sin (x * 1.7f) * 0.5f + std::sin (x * 0.53f) * 0.5f) * 0.6f + 0.4f;
        ir.lineTo (x0 + x, base - env * 70 * jit);
    }
    fill = ir; fill.lineTo (x1, base); fill.closeSubPath();
    g.setGradientFill (ColourGradient (ui::red.withAlpha (0.45f), x0, 0, ui::red.withAlpha (0.0f), x1, 0, false)); g.fillPath (fill);
    g.setColour (Colour (0xffff5060).withAlpha (0.9f)); g.strokePath (ir, PathStrokeType (1.2f));
    g.setColour (Colours::white.withAlpha (0.12f)); g.drawHorizontalLine ((int) base, x0, x1);
    const float px = x0 + (x1 - x0) * (preMs * 0.001f / shown);
    {
        Path p; p.startNewSubPath (px, base - 80); p.lineTo (px, base + 6);
        Path d; const float dash[] = { 2.0f, 3.0f }; PathStrokeType (1.0f).createDashedStroke (d, p, dash, 2);
        g.setColour (Colours::white.withAlpha (0.5f)); g.fillPath (d);
    }
    const String preTxt = sync ? "PRE " + azp::noteNames[(int) s.getRawParameterValue (azp::preNote)->load()] + "  " + String ((int) preMs) + "ms"
                               : "PRE " + String ((int) preMs) + "ms";
    spaced (g, preTxt, { px + 5, base - 80, 160, 12 }, 9, Colour (0xffa49c9e));
    spaced (g, "0", { x0, base + 6, 30, 12 }, 9, ui::dimmer);
    spaced (g, String (shown, 1) + "s", { x1 - 60, base + 6, 60, 12 }, 9, ui::dimmer, Justification::centredRight);

    // o'qishlar
    spaced (g, "DECAY RT60", { 22, 14, 200, 14 }, 10, ui::dimmer);
    g.setFont (ui::font (30, 700)); g.setColour (Colours::white);
    const String dv = freeze ? String (CharPointer_UTF8 ("\xe2\x88\x9e")) : String (decay, 2);
    g.drawText (dv, Rectangle<float> (22, 30, 160, 36), Justification::centredLeft);
    const float tw = GlyphArrangement::getStringWidth (ui::font (30, 700), dv);
    if (! freeze) spaced (g, "s", { 24 + tw, 44, 20, 20 }, 13, ui::redSoft, Justification::centredLeft, 600, 0);
    spaced (g, azp::algoNames[algo].toUpperCase() + (freeze ? juce::String::fromUTF8 ("  \xc2\xb7  FREEZE") : ""), { 22, 70, 220, 12 }, 9.5f, ui::redSoft);
    spaced (g, "AZ XARAKTER", { W - 22 - 4 * 58, 16, 232, 12 }, 9.5f, ui::dimmer, Justification::centred);
}

// ============================================================================ Smart Vocal paneli
SmartPanel::SmartPanel (AZVerbProcessor& p) : proc (p)
{
    apply.getProperties().set ("kind", "go"); ab.getProperties().set ("kind", "small"); next.getProperties().set ("kind", "small");
    next.setButtonText (juce::String::fromUTF8 ("KEYINGI \xe2\x80\xba"));
    for (auto* b : { &apply, &ab, &next }) addAndMakeVisible (b);
    apply.onClick = [this] { proc.applyRecommendation (proc.smartIndex); repaint(); };
    ab.onClick    = [this] { proc.toggleSmartAB(); repaint(); };
    next.onClick  = [this] { if (! recs.empty()) { proc.smartIndex = (proc.smartIndex + 1) % (int) recs.size(); repaint(); } };
    startTimerHz (30);
}

void SmartPanel::resized()
{
    listenBox = { 20, 12, 190, 86 };
    const int cx = getWidth() - 20 - 300;
    apply.setBounds (cx + 12, 72, 84, 22); ab.setBounds (cx + 102, 72, 44, 22); next.setBounds (cx + 152, 72, 82, 22);
}

void SmartPanel::timerCallback()
{
    if (proc.resultVersion.load() != seenVersion)
    {
        seenVersion = proc.resultVersion.load();
        const ScopedLock sl (proc.resultLock);
        prof = proc.profile; recs = proc.recs;
        repaint();
    }
    const bool active = proc.listening.load() || proc.analysing.load();
    hist[histPos] = active ? proc.liveLevel.load() : 0.0f; histPos = (histPos + 1) % 34;
    const bool hasRecs = ! recs.empty();
    apply.setEnabled (hasRecs); next.setEnabled (hasRecs); ab.setEnabled (proc.smartApplied);
    ab.setToggleState (proc.smartApplied && proc.smartShowingBefore, dontSendNotification);
    if (active) repaint (listenBox);
}

void SmartPanel::mouseUp (const MouseEvent& e)
{
    if (listenBox.contains (e.getPosition()))
    {
        if (proc.listening) proc.cancelListening(); else proc.startListening();
        repaint(); return;
    }
    for (int i = 0; i < 3; ++i)
        if (altBoxes[i].contains (e.getPosition()))
        {
            const int idx = (proc.smartIndex + 1 + i) % jmax (1, (int) recs.size());
            proc.smartIndex = idx; repaint(); return;
        }
}

String SmartPanel::recipeLine (const Recipe& r) const
{
    String pre = r.sync ? "Pre " + azp::noteNames[r.note] : "Pre " + String ((int) r.predelayMs) + " ms";
    return azp::algoNames[r.algo] + juce::String::fromUTF8 (" \xc2\xb7 Decay ") + String (r.decay, 1) + juce::String::fromUTF8 (" s \xc2\xb7 ") + pre + juce::String::fromUTF8 (" \xc2\xb7 Damp ") + azp::fmtHz (r.damp)
         + juce::String::fromUTF8 (" \xc2\xb7 Low cut ") + azp::fmtHz (r.lowcut) + juce::String::fromUTF8 (" \xc2\xb7 De-ess ") + azp::fmtHz (r.deessHz) + (r.duck ? juce::String::fromUTF8 (" \xc2\xb7 Duck ON") : "");
}

void SmartPanel::paint (Graphics& g)
{
    g.setGradientFill (ColourGradient (ui::redHot.withAlpha (0.05f), 0, 0, Colours::transparentBlack, (float) getWidth() * 0.4f, 0, false));
    g.fillRect (getLocalBounds());

    // --- tinglash qutisi ---
    const bool lis = proc.listening.load(), ana = proc.analysing.load();
    auto lb = listenBox.toFloat();
    g.setColour (ui::redHot.withAlpha (lis ? 0.14f : 0.08f)); g.fillRoundedRectangle (lb, 12);
    g.setColour (ui::redHot.withAlpha (lis ? 0.6f : 0.33f)); g.drawRoundedRectangle (lb.reduced (0.5f), 12, 1);
    g.setColour (ui::red.withAlpha (lis ? 0.4f : 0.2f)); g.fillEllipse (lb.getX() + 11, lb.getY() + 16, 13, 13);
    g.setColour (ui::red); g.fillEllipse (lb.getX() + 14, lb.getY() + 19, 7, 7);
    spaced (g, "SMART VOCAL", { lb.getX() + 30, lb.getY() + 14, 150, 16 }, 10, ui::redSoft);
    for (int i = 0; i < 34; ++i)
    {
        const float v = hist[(histPos + i) % 34];
        const float bh = (lis || ana) ? jlimit (2.0f, 26.0f, 4.0f + std::sqrt (v) * 30.0f) : 3.0f + 1.5f * std::abs (std::sin ((float) i * 0.9f));
        g.setColour (Colour (0xffff5060).withAlpha (0.45f + bh / 50.0f));
        g.fillRoundedRectangle (lb.getX() + 14 + (float) i * 4.8f, lb.getY() + 45 - bh * 0.5f, 3, bh, 1.5f);
    }
    String st;
    if (lis)
    {
        const double sr = proc.getSampleRateSafe();
        const double left = jmax (0.0, proc.listenTargetSeconds - proc.captureLenSeconds (sr));
        st = juce::String::fromUTF8 ("Vokal tinglanmoqda\xe2\x80\xa6 ") + String ((int) std::ceil (left)) + " s";
    }
    else if (ana) st = juce::String::fromUTF8 ("Tahlil qilinmoqda\xe2\x80\xa6");
    else if (prof.valid) st = juce::String::fromUTF8 ("Tayyor \xc2\xb7 qayta tinglash");
    else st = "Bosing va vokalni ijro eting";
    spaced (g, st, { lb.getX() + 14, lb.getY() + 62, 170, 14 }, 10, ui::dim, Justification::centredLeft, 400, 0);

    // --- tembr tahlili ---
    const float px = 226, pw = (float) getWidth() - 20 - 300 - 14 - px;
    spaced (g, "VOKAL TEMBRI TAHLILI", { px, 12, pw, 12 }, 9.5f, ui::dimmer, Justification::centredLeft, 500, 0.24f);
    const String heads[4] = { "OVOZ TURI", "TEMBR", "SIBILYANT", "TEMP / USLUB" };
    String vals[4] = { juce::String::fromUTF8 ("\xe2\x80\x94"), juce::String::fromUTF8 ("\xe2\x80\x94"), juce::String::fromUTF8 ("\xe2\x80\x94"), juce::String::fromUTF8 ("\xe2\x80\x94") }, subs[4];
    if (prof.valid)
    {
        vals[0] = prof.voiceType; subs[0] = prof.voiceSub;
        vals[1] = prof.timbre;    subs[1] = prof.timbreSub;
        vals[2] = azp::fmtHz (prof.sibHz); subs[2] = prof.sibLabel;
        vals[3] = String (juce::roundToInt (prof.bpm)) + " BPM"; subs[3] = prof.style + (prof.bpmFromHost ? "" : juce::String::fromUTF8 (" \xc2\xb7 taxmin"));
    }
    const float cw = (pw - 21) / 4;
    for (int i = 0; i < 4; ++i)
    {
        auto c = Rectangle<float> (px + (float) i * (cw + 7), 28, cw, 52);
        g.setColour (Colours::white.withAlpha (0.024f)); g.fillRoundedRectangle (c, 9);
        g.setColour (Colours::white.withAlpha (0.063f)); g.drawRoundedRectangle (c.reduced (0.5f), 9, 1);
        spaced (g, heads[i], { c.getX() + 10, c.getY() + 7, cw - 14, 11 }, 8.5f, ui::dimmer, Justification::centredLeft, 500, 0.18f);
        spaced (g, vals[i], { c.getX() + 10, c.getY() + 19, cw - 14, 16 }, 12.5f, Colours::white, Justification::centredLeft, 600, 0);
        spaced (g, subs[i], { c.getX() + 10, c.getY() + 35, cw - 14, 13 }, 10, ui::redSoft, Justification::centredLeft, 400, 0);
    }
    // muqobil variantlar
    g.setFont (ui::font (10, 400, 0.04f));
    float ax = px + 104;
    spaced (g, "Boshqa variantlar:", { px, 86, 110, 14 }, 10, ui::dim, Justification::centredLeft, 400, 0.04f);
    for (int i = 0; i < 3; ++i) altBoxes[i] = {};
    if (recs.size() > 1)
        for (int i = 0; i < 3 && i + 1 < (int) recs.size(); ++i)
        {
            const auto& r = recs[(size_t) ((proc.smartIndex + 1 + i) % (int) recs.size())];
            String nm = r.recipe.name.upToFirstOccurrenceOf (juce::String::fromUTF8 (" \xe2\x80\x94"), false, false);
            const float w = GlyphArrangement::getStringWidth (ui::font (10, 500), nm) + 14;
            if (ax + w > px + pw) break;
            altBoxes[i] = Rectangle<float> (ax, 85, w, 16).toNearestInt();
            spaced (g, nm, { ax, 86, w, 14 }, 10, ui::silver, Justification::centredLeft, 500, 0);
            ax += w;
        }
    else spaced (g, juce::String::fromUTF8 ("\xe2\x80\x94"), { ax, 86, 60, 14 }, 10, ui::dimmer, Justification::centredLeft, 400, 0);

    // --- tavsiya kartasi ---
    auto rc = Rectangle<float> ((float) getWidth() - 20 - 300, 12, 300, 86);
    g.setGradientFill (ColourGradient (ui::redHot.withAlpha (0.16f), rc.getX(), rc.getY(), ui::redHot.withAlpha (0.03f), rc.getRight(), rc.getBottom(), false));
    g.fillRoundedRectangle (rc, 12);
    g.setColour (ui::redHot.withAlpha (0.33f)); g.drawRoundedRectangle (rc.reduced (0.5f), 12, 1);
    spaced (g, "2026 TREND TAVSIYASI", { rc.getX() + 12, rc.getY() + 9, 200, 11 }, 8.5f, ui::redSoft, Justification::centredLeft, 500, 0.18f);
    if (! recs.empty())
    {
        const auto& r = recs[(size_t) jlimit (0, (int) recs.size() - 1, proc.smartIndex)];
        spaced (g, "MOSLIK " + String (r.matchPct) + "%", { rc.getRight() - 112, rc.getY() + 9, 100, 11 }, 8.5f, ui::redSoft, Justification::centredRight, 500, 0.18f);
        spaced (g, r.recipe.name, { rc.getX() + 12, rc.getY() + 22, 280, 18 }, 13.5f, Colours::white, Justification::centredLeft, 700, 0);
        g.setFont (ui::font (9.5f)); g.setColour (Colour (0xffbdb6b7));
        g.drawFittedText (recipeLine (r.recipe), Rectangle<float> (rc.getX() + 12, rc.getY() + 40, 278, 22).toNearestInt(), Justification::topLeft, 2, 1.0f);
    }
    else
    {
        spaced (g, "Vokalni tinglating", { rc.getX() + 12, rc.getY() + 22, 280, 18 }, 13.5f, Colours::white, Justification::centredLeft, 700, 0);
        spaced (g, "Plagin tembrni o'lchab, eng mos zamonaviy reverbni taklif qiladi", { rc.getX() + 12, rc.getY() + 40, 280, 14 }, 9.5f, Colour (0xffbdb6b7), Justification::centredLeft, 400, 0);
    }
}

// ============================================================================ Editor
AZVerbEditor::AZVerbEditor (AZVerbProcessor& p)
    : AudioProcessorEditor (&p), proc (p), stage (p), meters (p), smart (p)
{
    setLookAndFeel (&lnf);
    LookAndFeel::setDefaultLookAndFeel (&lnf);
    addAndMakeVisible (view);

    for (auto* b : { &prevBtn, &nextBtn, &aBtn, &bBtn, &undoBtn, &redoBtn }) { b->getProperties().set ("kind", "pill"); view.addAndMakeVisible (b); }
    prevBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.loadPreset ((proc.currentPreset + n - 1) % n); view.repaint(); };
    nextBtn.onClick = [this] { proc.loadPreset ((proc.currentPreset + 1) % proc.getNumPrograms()); view.repaint(); };
    aBtn.onClick = [this] { proc.selectAB (0); timerCallback(); };
    bBtn.onClick = [this] { proc.selectAB (1); timerCallback(); };
    undoBtn.onClick = [this] { proc.undo.undo(); };
    redoBtn.onClick = [this] { proc.undo.redo(); };
    undoBtn.setTooltip ("Undo"); redoBtn.setTooltip ("Redo");

    for (int i = 0; i < azp::algoNames.size(); ++i)
    {
        auto* b = algoBtns.add (new TextButton (azp::algoNames[i]));
        b->getProperties().set ("kind", "algo");
        b->onClick = [this, i]
        {
            auto* prm = proc.apvts.getParameter (azp::algo);
            proc.undo.beginNewTransaction ("Algorithm");
            prm->beginChangeGesture(); prm->setValueNotifyingHost (prm->convertTo0to1 ((float) i)); prm->endChangeGesture();
        };
        view.addAndMakeVisible (b);
    }
    view.addAndMakeVisible (stage);
    view.addAndMakeVisible (meters);
    for (auto* t : { &freezeBtn, &duckBtn, &syncBtn }) view.addAndMakeVisible (t);
    freezeAtt = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, azp::freeze, freezeBtn);
    duckAtt   = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, azp::duck, duckBtn);
    syncAtt   = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, azp::sync, syncBtn);
    view.addAndMakeVisible (smart);

    lastSync = proc.apvts.getRawParameterValue (azp::sync)->load() > 0.5f;
    preKnob = knobs.add (new Knob (proc, lastSync ? azp::preNote : azp::predelay, "PRE-DELAY", ui::red));
    knobs.add (new Knob (proc, azp::size, "SIZE", ui::red));
    knobs.add (new Knob (proc, azp::decay, "DECAY", ui::red));
    knobs.add (new Knob (proc, azp::diffusion, "DIFFUSION", ui::red));
    knobs.add (new Knob (proc, azp::damp, "DAMP HF", ui::red));
    knobs.add (new Knob (proc, azp::lowcut, "LOW CUT", ui::silver));
    knobs.add (new Knob (proc, azp::highcut, "HIGH CUT", ui::silver));
    knobs.add (new Knob (proc, azp::width, "WIDTH", ui::silver));
    knobs.add (new Knob (proc, azp::mix, "MIX", ui::red, true));
    for (auto* k : knobs) view.addAndMakeVisible (k);

    for (auto* prm : proc.getParameters())
        if (auto* rp = dynamic_cast<RangedAudioParameter*> (prm)) proc.apvts.addParameterListener (rp->getParameterID(), this);

    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio ((double) kW / kH);
    setResizeLimits (kW * 6 / 10, kH * 6 / 10, kW * 2, kH * 2);
    setSize (kW, kH);
    layout();
    handleAsyncUpdate();
    startTimerHz (30);
}

AZVerbEditor::~AZVerbEditor()
{
    for (auto* prm : proc.getParameters())
        if (auto* rp = dynamic_cast<RangedAudioParameter*> (prm)) proc.apvts.removeParameterListener (rp->getParameterID(), this);
    LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void AZVerbEditor::paint (Graphics& g) { g.fillAll (ui::bg0); }

void AZVerbEditor::resized()
{
    view.setBounds (0, 0, kW, kH);
    view.setTransform (AffineTransform::scale ((float) getWidth() / (float) kW));
}

void AZVerbEditor::layout()
{
    presetBox = { 360, 12, 300, 32 };
    prevBtn.setBounds (presetBox.getRight() - 60, 16, 26, 24);
    nextBtn.setBounds (presetBox.getRight() - 31, 16, 26, 24);
    aBtn.setBounds (676, 14, 30, 28); bBtn.setBounds (712, 14, 30, 28);
    undoBtn.setBounds (754, 14, 32, 28); redoBtn.setBounds (790, 14, 32, 28);
    cpuBox = { 840, 14, 120, 28 };
    for (int i = 0; i < algoBtns.size(); ++i) algoBtns[i]->setBounds (18, 82 + i * 39, 124, 34);
    stage.setBounds (150, 56, 680, 362);
    meters.setBounds (838, 92, 100, 200);
    freezeBtn.setBounds (846, 306, 116, 30); duckBtn.setBounds (846, 343, 116, 30); syncBtn.setBounds (846, 380, 116, 30);
    smart.setBounds (0, 418, kW, 110);
    int x = 22; const int y = 540;
    for (int i = 0; i < knobs.size(); ++i)
    {
        if (i == 5 || i == 8) x += 12;
        const int w = i == 8 ? 128 : 98;
        knobs[i]->setBounds (x, y, w, i == 8 ? 140 : 110);
        x += w;
    }
}

void AZVerbEditor::handleAsyncUpdate()
{
    const int algo = (int) proc.apvts.getRawParameterValue (azp::algo)->load();
    for (int i = 0; i < algoBtns.size(); ++i) algoBtns[i]->setToggleState (i == algo, dontSendNotification);
    const bool sync = proc.apvts.getRawParameterValue (azp::sync)->load() > 0.5f;
    if (sync != lastSync) { lastSync = sync; preKnob->attachTo (sync ? azp::preNote : azp::predelay); }
    stage.repaint();
    for (auto* k : knobs) k->repaint();
    for (auto* k : { &stage.sand, &stage.crystal, &stage.focus, &stage.deess }) k->repaint();
    view.repaint (presetBox);
}

void AZVerbEditor::timerCallback()
{
    meters.tick();
    aBtn.setToggleState (proc.abSlot == 0, dontSendNotification);
    bBtn.setToggleState (proc.abSlot == 1, dontSendNotification);
    undoBtn.setEnabled (proc.undo.canUndo()); redoBtn.setEnabled (proc.undo.canRedo());
    const float cpu = proc.cpuLoad.load();
    if (std::abs (cpu - cpuShown) > 0.0005f) { cpuShown = cpu; view.repaint (cpuBox); }
}

void AZVerbEditor::showPresetMenu()
{
    PopupMenu m; String cat;
    const auto& f = presets::factory();
    for (int i = 0; i < (int) f.size(); ++i)
    {
        if (f[(size_t) i].category != cat) { cat = f[(size_t) i].category; m.addSectionHeader (cat); }
        m.addItem (i + 1, f[(size_t) i].name, true, i == proc.currentPreset);
    }
    m.showMenuAsync (PopupMenu::Options().withTargetComponent (&view).withTargetScreenArea (view.localAreaToGlobal (presetBox)),
                     [this] (int r) { if (r > 0) { proc.loadPreset (r - 1); view.repaint(); } });
}

void AZVerbEditor::MainView::mouseUp (const MouseEvent& e)
{
    if (ed.presetBox.withTrimmedRight (64).contains (e.getPosition())) ed.showPresetMenu();
}

void AZVerbEditor::MainView::paint (Graphics& g)
{
    auto& p = ed.proc;
    const float W = (float) kW, H = (float) kH;
    g.fillAll (ui::bg0);
    ColourGradient bgG (ui::bg2, W * 0.5f, 0, ui::bg0, W * 0.5f, H * 0.95f, true);
    bgG.addColour (0.45, ui::bg1);
    g.setGradientFill (bgG); g.fillRect (getLocalBounds());
    g.setColour (Colours::white.withAlpha (0.018f));
    for (int y = 0; y < kH; y += 3) g.drawHorizontalLine (y, 0, W);

    // sarlavha
    g.setColour (Colours::white.withAlpha (0.05f)); g.drawHorizontalLine (56, 0, W);
    auto mark = Rectangle<float> (20, 13, 30, 30);
    g.setColour (ui::redHot.withAlpha (0.25f)); g.fillRoundedRectangle (mark.expanded (4), 11);
    g.setGradientFill (ColourGradient (Colour (0xffff5a3a), mark.getX(), mark.getY(), Colour (0xff9a1020), mark.getRight(), mark.getBottom(), false));
    g.fillRoundedRectangle (mark, 8);
    spaced (g, "AZ", mark, 13, Colours::white, Justification::centred, 700, 0);
    g.setFont (ui::font (18, 700, 0.18f)); g.setColour (ui::text);
    g.drawText ("AZ", Rectangle<float> (60, 12, 40, 22), Justification::centredLeft);
    const float azW = GlyphArrangement::getStringWidth (ui::font (18, 700, 0.18f), "AZ ");
    g.setColour (ui::red); g.drawText ("VERB", Rectangle<float> (60 + azW, 12, 80, 22), Justification::centredLeft);
    spaced (g, "AZIMXODJAYEV MUSIC", { 60, 34, 200, 12 }, 9.5f, ui::dim, Justification::centredLeft, 500, 0.22f);

    // preset qutisi
    auto pb = ed.presetBox.toFloat();
    g.setColour (Colours::white.withAlpha (0.03f)); g.fillRoundedRectangle (pb, 9);
    g.setColour (Colours::white.withAlpha (0.07f)); g.drawRoundedRectangle (pb.reduced (0.5f), 9, 1);
    const auto& pr = presets::factory()[(size_t) p.currentPreset];
    const float tagW = GlyphArrangement::getStringWidth (ui::font (9.5f, 500, 0.15f), pr.category) + 14;
    auto tag = Rectangle<float> (pb.getX() + 10, pb.getCentreY() - 9, tagW, 18);
    g.setColour (ui::red.withAlpha (0.35f)); g.drawRoundedRectangle (tag, 5, 1);
    spaced (g, pr.category, tag, 9.5f, ui::red, Justification::centred, 500, 0.15f);
    spaced (g, pr.name, { tag.getRight() + 10, pb.getY(), pb.getRight() - tag.getRight() - 76, pb.getHeight() }, 12.5f, ui::text, Justification::centredLeft, 400, 0);

    // CPU
    auto cb = ed.cpuBox.toFloat();
    spaced (g, "CPU", { cb.getX(), cb.getY(), 32, cb.getHeight() }, 10.5f, ui::dim, Justification::centredLeft, 500, 0.12f);
    auto bar = Rectangle<float> (cb.getX() + 34, cb.getCentreY() - 2.5f, 46, 5);
    g.setColour (Colours::white.withAlpha (0.06f)); g.fillRoundedRectangle (bar, 2.5f);
    const float cpu = jlimit (0.0f, 1.0f, ed.cpuShown);
    const Colour cc = cpu < 0.3f ? ui::green : cpu < 0.6f ? Colour (0xffffc04a) : ui::red;
    g.setColour (cc); g.fillRoundedRectangle (bar.withWidth (jmax (4.0f, 46 * cpu)), 2.5f);
    spaced (g, String (cpu * 100.0f, 1) + "%", { cb.getX() + 84, cb.getY(), 40, cb.getHeight() }, 10.5f, cc, Justification::centredRight, 500, 0.06f);

    // bo'lim yozuvlari
    spaced (g, "ALGORITM", { 18, 70, 120, 12 }, 9.5f, ui::dimmer, Justification::centredLeft, 500, 0.24f);
    spaced (g, "OUTPUT", { 846, 70, 110, 12 }, 9.5f, ui::dimmer, Justification::centredLeft, 500, 0.24f);

    // pastki qism
    g.setColour (Colours::white.withAlpha (0.05f)); g.drawHorizontalLine (418, 0, W); g.drawHorizontalLine (528, 0, W);
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.016f), 0, 528, Colours::transparentWhite, 0, H, false));
    g.fillRect (0.0f, 529.0f, W, H - 529.0f);
    g.setColour (Colours::white.withAlpha (0.05f));
    g.drawVerticalLine (22 + 5 * 98 + 6, 556, 652); g.drawVerticalLine (22 + 8 * 98 + 12 + 6, 556, 652);
}
