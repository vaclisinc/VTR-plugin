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

#### For Step 3-4 (Static EQ Implementation):
- [ ] **Number of bands?** (2, 3, 4, 5, or user-selectable?)
- [ ] **Filter types per band?**
  - Low shelf?
  - High shelf?
  - Bell/Peak?
  - Notch?
  - High-pass?
  - Low-pass?
  - All-pass?
- [ ] **Parameter ranges?**
  - Frequency range? (20Hz-20kHz or different?)
  - Gain range? (±12dB, ±18dB, ±24dB?)
  - Q/Bandwidth range? (0.1-10, 0.5-5, other?)
- [ ] **Filter quality?**
  - Cramped filters (analog-like)?
  - Linear frequency response?
  - Oversampling needed?

#### For Step 5 (Multi-band):
- [ ] Fixed number of bands or dynamic?
- [ ] Band enable/disable feature?
- [ ] Band solo feature?
- [ ] Band linking feature?
- [ ] Global gain/output control?

#### For Step 6 (GUI):
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
  - Spectrum analyzer?
  - Level meters?
  - Gain reduction meters?
- [ ] **Size constraints?**
  - Fixed size or resizable?
  - Minimum/maximum dimensions?

#### For Step 7 (Dynamics):
- [ ] **Dynamics parameters per band?**
  - Threshold range?
  - Ratio range? (1:1 to ∞:1?)
  - Attack time range? (0.1ms - 100ms?)
  - Release time range? (1ms - 1000ms?)
  - Knee type? (hard/soft)
- [ ] **Detection type?**
  - Peak?
  - RMS?
  - User selectable?
- [ ] **Sidechain?**
  - Internal only?
  - External input?
  - Per-band sidechain?

#### For Step 8 (Advanced Features):
- [ ] Which advanced features are priority?
  - M/S processing?
  - Linear phase mode?
  - Oversampling?
  - Auto-gain compensation?
  - Spectrum matching?

### Documentation Requirements

For each step, create:
1. `docs/step-X-requirements.md` - User's answers to all questions
2. `docs/step-X-explanation.md` - Technical implementation details
3. `docs/step-X-assumptions.md` - Any assumptions made (should be minimal!)

### Example Requirements Gathering

```markdown
# Step 3 Requirements - Static EQ Implementation

## Questions for User:

1. **How many EQ bands would you like for the initial implementation?**
   - Options: 2, 3, 4, 5, or should it be user-configurable?

2. **Which filter types should be available for each band?**
   - Low shelf
   - High shelf  
   - Bell/Peak
   - Notch
   - High-pass
   - Low-pass
   - Or should each band have a fixed type?

3. **What parameter ranges would you prefer?**
   - Frequency: 20Hz-20kHz or wider?
   - Gain: ±12dB, ±18dB, or ±24dB?
   - Q factor: 0.1-10 or different range?

4. **Do you have any specific sound quality preferences?**
   - Analog-modeled (cramped) filters?
   - Transparent/clinical sound?
   - Specific filter topology preference?

Please answer these questions so I can implement exactly what you need.
```

### IMPORTANT REMINDERS:

1. **NEVER PROCEED WITHOUT ANSWERS**
2. **IF USER SAYS "YOU DECIDE"** - Give options with pros/cons
3. **DOCUMENT EVERYTHING** - Future agents need to understand decisions
4. **ASK FOLLOW-UP QUESTIONS** if answers are unclear
5. **NO ASSUMPTIONS** - When in doubt, ask!