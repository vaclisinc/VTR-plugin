# 🚨 AGENT README - READ THIS FIRST!

## Your Workflow
1. Read this file
2. Check `agent/progress-tracker.md` for current status
3. step by step follow the implementation plan in tracker


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