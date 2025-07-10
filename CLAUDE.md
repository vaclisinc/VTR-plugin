# 🚨 AGENT README - READ THIS FIRST!

## Project Overview
This is the VTR-plugin integration project - integrating Vaclis Tone Replication model (github.com/vaclisinc/vaclis_tone_replication) with the existing plugin architecture.

## File Structure
```
agent/
├── PROJECT-ROADMAP.md       # The VTR integration roadmap
├── progress-tracker.md      # Current status
└── requirements-template.md # Questions to ask user
CLAUDE.md                    # This file (quick start guide)
docs/                        # Step documentation goes here
Source/                      # All code goes here
```

# How To Contribute

## Before Writing/Editing Code

Before you create or modify code **in any way**, you **MUST**:

1. Query the **Context7 MCP server** for up‑to‑date, verified information.
2. If the MCP query returns insufficient data, perform a detailed web search **with citations**.
3. If the web search still lacks clarity, **pause and request clarification**.

### Note About Assumptions

- **NEVER** assume you know how to implement or debug a feature without first researching via Context7 MCP.

**_The correct Context7 Library ID for JUCE is /juce-framework/juce_**

---

## Plugin / Application Builds

- **Always** build the JUCE application or plug‑in for the user.
- After every build, **move** the binaries to the correct macOS paths:
  - **AU:** `~/Library/Audio/Plug-Ins/Components/`
  - **VST3:** `~/Library/Audio/Plug-Ins/VST3/`
- Local builds target the host arch only. CI tag builds (`v*`) must produce a universal
  binary (arm64;x86_64) so the plug-in loads on both Apple-silicon and Intel hosts.

---

## Continuous Validation Standard

Every JUCE plug‑in in this repo **must pass** the following headless tests **on every commit**:

1. **pluginval smoke / stress**

   - Command: `pluginval --strictness high --validate-in-process <plugin>`
   - **Minimum version:** `≥ 0.6.7`

2. **Offline audio render regression**

   - A console host pumps a known WAV through the plug‑in (48 kHz, 128‑sample buffer).
   - Python/NumPy analysis confirms:
     - Expected DSP behaviour (e.g., octave‑up peak for shimmer).
     - No NaNs, INFs, or denormals.

3. **Real‑time CPU & glitch benchmark**

   - Measure `processBlock` time; **fail** if it exceeds 90 % of the buffer period.
   - Detect discontinuities, zipper noise, or runaway feedback.

4. **UI Screenshot Sanity**

   - Run `juce_pluginhost --headless --capture-ui <plugin> /tmp/ui.png`
   - Fail if the PNG width < 200 px or height < 100 px.

🗂️ **CI Artifacts:** If any test fails, upload rendered WAVs, logs, and timing CSVs to _GitHub Actions » Artifacts » `ci_failures`_ for inspection.

---

## Smoke Build Gate

Before any other checklist task runs the assistant **must**:

1. Configure the project for the host CPU only (no universal build).
2. Compile with Ninja.
3. Copy the binary to the local VST3/AU paths.
4. Run `pluginval --strictness high --validate-in-process`.
5. Render 2 s of pink noise through the plug-in (48 kHz, 256-sample buffer) and confirm  
   RMS difference < 0.1 dB (only for bypass mode; skip once DSP tasks begin).

If any step fails, **abort the checklist** and ask Lex for guidance.

---

## GUI & Front-end Design ( JUCE Components Only )

1. **Layout rules**

   - Use JUCE `FlexBox` or `Grid` for all primary layouts; avoid absolute pixel positioning except for micro-alignments.
   - Maintain **8 px** internal padding and **16 px** between unrelated control groups.
   - No component may overlap another at any window size; verify with `Component::getBounds()` in GUI tests.

2. **Look & Feel**

   - Derive a custom `LookAndFeel_V4` subclass for consistent colours, fonts, and knob/slider skins.
   - Respect OS Hi-DPI scaling (JUCE handles per-display scale factors for you)—never hard-code pixel sizes that break on retina/4K monitors.

3. **Z-ordering**

   - Call `toFront()` or `setAlwaysOnTop()` _only_ for modal pop-ups; otherwise rely on natural child order.
   - Run GUI unit test `z_order_test.cpp` to assert interactive components are not obscured.

4. **Responsiveness**

   - All plug-ins must be **resizable**. Handle `resized()` to recompute layout via FlexBox/Grid; avoid magic numbers.
   - Target 60 fps GUI repaint budget; throttle `repaint()` on meters/visualisers (max 30 Hz).

5. **Accessibility & usability**

   - Minimum text size: **11 pt** at 100 % scale.
   - Sliders/knobs must include a `TooltipWindow` description and keyboard focus traversal.
   - Colour-contrast ratio ≥ 4.5:1 for text vs. background.

6. **Automated GUI checks**

   - Every build runs `tests/gui_layout_test.cpp` to assert:
     1. No overlapping bounds.
     2. All interactive components fit within the parent window.
     3. Z-order of active controls is unobstructed.
   - Fail CI if any assertion fails.

7. **Performance guard-rails**
   - Avoid `repaint()` inside `sliderValueChanged`; instead set a flag and repaint in a timer callback (`Timer::startTimerHz (30)`).
   - Use `PathStrokeType::mitered` only for static SVG paths; prefer `renderCachedImage()` for repeated vector drawings.

---

Follow these rules and the automated pipeline will catch crashes, silence, and CPU spikes **before** you ever open the plug‑in in a DAW.
