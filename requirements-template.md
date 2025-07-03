# Requirements Gathering Template for Dynamic EQ Plugin

## This template MUST be used by agents before implementing each step

### Step-Specific Requirements Checklist

#### For ANY Step:
- [ ] Have I asked about ALL technical specifications?
- [ ] Have I asked about design preferences?
- [ ] Have I asked about priority features?
- [ ] Have I documented all answers?
- [ ] Am I making ANY assumptions? If yes, have I asked the user?

#### For Step 1 (Project Setup):
- [ ] Which OS/Platform are we targeting?
- [ ] Which DAW will be used for testing?
- [ ] Preferred project name?
- [ ] VST3, AU, or both?
- [ ] Sample rate support (44.1k, 48k, 96k, etc.)?
- [ ] Mono, Stereo, or both?

#### For Step 2 (Parameter System):
- [ ] How many total parameters expected?
- [ ] Parameter value ranges?
- [ ] Linear or logarithmic scaling?
- [ ] Default values?
- [ ] Parameter smoothing requirements?

#### For Step 3 (chowdsp Integration & Single Band EQ):
- [ ] **chowdsp Integration Preferences:**
  - Use as git submodule or copy specific modules?
  - Include entire library or just EQ modules?
  - Any specific chowdsp features to leverage?
- [ ] **Initial Band Configuration:**
  - Start with which filter type? (Bell, Shelf, etc.)
  - Frequency range? (20Hz-20kHz standard?)
  - Gain range? (±18dB typical?)
  - Q/Bandwidth range? (0.1-10?)
- [ ] **Filter Quality Settings:**
  - Use chowdsp's analog modeling features?
  - Enable higher-order filters?
  - Any specific chowdsp filter variants?

#### For Step 4 (Filter Types):
- [ ] **Which filter types to implement?**
  - Bell/Peak (standard in chowdsp)
  - Low Shelf (with what slope?)
  - High Shelf (with what slope?)
  - High-pass (6, 12, 24, 48 dB/oct?)
  - Low-pass (6, 12, 24, 48 dB/oct?)
  - Notch (how narrow?)
  - All-pass (for phase correction?)
- [ ] **Filter behavior preferences:**
  - Butterworth, Bessel, Chebyshev characteristics?
  - Variable slope/order per filter?
  - Analog-modeled vs. digital precision?

#### For Step 5 (Multi-Band System):
- [ ] Total number of bands? (3, 4, 5, 6?)
- [ ] Fixed bands or user-configurable count?
- [ ] Band enable/disable feature?
- [ ] Band solo feature?
- [ ] Band linking feature?
- [ ] Per-band input/output meters?

#### For Step 6 (Basic GUI):
- [ ] **Visual style preference?**
  - Skeuomorphic (realistic)?
  - Flat/modern?
  - Dark theme, light theme, or both?
- [ ] **Control types?**
  - Knobs?
  - Sliders?
  - Number boxes?
  - Frequency graph interaction?
- [ ] **Display elements?**
  - Frequency response curve?
  - Individual band curves?
  - Combined response curve?
  - Grid overlay?
- [ ] **Size constraints?**
  - Fixed size or resizable?
  - Minimum/maximum dimensions?

#### For Step 7 (Add Dynamics):
- [ ] **Dynamics parameters per band?**
  - Threshold range? (-60dB to 0dB?)
  - Ratio range? (1:1 to ∞:1?)
  - Attack time range? (0.1ms - 100ms?)
  - Release time range? (1ms - 1000ms?)
  - Knee type? (hard/soft/adjustable?)
- [ ] **Detection type?**
  - Peak?
  - RMS?
  - User selectable?
- [ ] **Gain reduction display?**
  - Numeric?
  - Meter?
  - On frequency display?

#### For Step 8 (Enhanced Features & Visualization):
- [ ] **Spectrum analyzer preferences:**
  - FFT size options?
  - Window type?
  - Update rate?
  - Peak hold?
- [ ] **Sidechain support:**
  - External sidechain input?
  - Per-band sidechain filtering?
  - Sidechain listen mode?
- [ ] **Additional meters:**
  - Input/output spectrum?
  - Correlation meter?
  - Peak/RMS meters?

#### For Step 9 (Optimization):
- [ ] **Performance targets:**
  - Maximum CPU usage?
  - Target latency?
  - Support for older systems?
- [ ] **Optimization features:**
  - Oversampling options? (2x, 4x, 8x?)
  - SIMD optimizations?
  - Lookahead for dynamics?
  - Zero-latency mode?

#### For Step 10 (Testing & Polish):
- [ ] **Preset system:**
  - Factory presets needed?
  - User preset management?
  - A/B comparison?
- [ ] **Documentation needs:**
  - User manual?
  - Video tutorials?
  - Tooltips in GUI?
- [ ] **Testing requirements:**
  - Specific test signals?
  - Automated testing?
  - Beta testing plan?

### Documentation Requirements

For each step, create:
1. `docs/step-X-requirements.md` - User's answers to all questions
2. `docs/step-X-implementation-plan.md` - Detailed implementation plan with subtasks
3. `docs/step-X-explanation.md` - Technical implementation details after completion

### Example Requirements Gathering

```markdown
# Step 3 Requirements - chowdsp Integration & Single Band EQ

## Questions for User:

1. **How would you like to integrate chowdsp_utils?**
   - As a git submodule (easier updates)?
   - Copy specific modules into the project?
   - Use CMake FetchContent?

2. **For the initial single band implementation, which filter type should we start with?**
   - Bell/Peak (most versatile)
   - Low Shelf (good for bass control)
   - High Shelf (good for treble control)

3. **What parameter ranges would you prefer?**
   - Frequency: 20Hz-20kHz (standard) or wider?
   - Gain: ±12dB, ±18dB, or ±24dB?
   - Q factor: 0.1-10 (standard) or different?

4. **Should we enable any special chowdsp features?**
   - Analog-style saturation modeling?
   - Higher-order filter options?
   - Built-in parameter smoothing?

Please answer these questions so I can implement exactly what you need.
```

### IMPORTANT REMINDERS:

1. **NEVER PROCEED WITHOUT ANSWERS** - Get explicit user confirmation
2. **IF USER SAYS "YOU DECIDE"** - Give 2-3 specific options with pros/cons
3. **DOCUMENT EVERYTHING** - Future agents need to understand all decisions
4. **ASK FOLLOW-UP QUESTIONS** - If answers are unclear or incomplete
5. **NO ASSUMPTIONS** - When in doubt, always ask!
6. **TECHNICAL CONTEXT** - With chowdsp, we get professional DSP from the start