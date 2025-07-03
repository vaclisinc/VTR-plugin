# Dynamic EQ Plugin - Project Roadmap

## What We're Building
Dynamic EQ = Static EQ + Level-based Gain Control
- Each band acts as frequency-specific compressor
- Build static EQ first, add dynamics later

## Technology Stack
1. **JUCE Framework** - Core infrastructure
2. **chowdsp_utils** - Professional filters from Step 3
3. **ff_meters** - Add visualization (Step 8+)

## Implementation Process

### Every Step Follows This Pattern:
1. **Gather Requirements** (15-30 min)
   - Use requirements-template.md
   - Document answers
2. **Create Plan** (15-30 min)
   - Break into subtasks (30-60 min each)
   - Get user approval
3. **Implement** (varies)
   - Execute approved subtasks
   - Test each subtask

## Project Steps

### Step 1: Audio Pass-Through [Simple: 1 session] ✅
- Create JUCE plugin structure
- Verify audio flows correctly
- ~1 hour total

### Step 2: Parameter System [Simple: 1 session] ✅
- Add parameter management
- Test with master gain
- ~1-2 hours total

### Step 3: chowdsp Integration & Single Band EQ [Medium: 3-4 sessions]
- Integrate chowdsp_utils library
- Implement professional filter using chowdsp
- Add frequency, gain, Q controls
- Test filtering works with high quality
- ~4-5 hours total

### Step 4: Filter Types [Medium: 2-3 sessions]
- Add Bell, High/Low Shelf using chowdsp
- Add High/Low Pass filters
- Parameter management for filter types
- Verify each type works correctly
- ~3-4 hours total

### Step 5: Multi-Band System [Medium: 2-3 sessions]
- Expand to 4 bands
- Band management architecture
- Parallel processing setup
- Band enable/disable controls
- ~4-5 hours total

### Step 6: Basic GUI [Complex: 3-5 sessions]
- Frequency response display
- Control layout for all bands
- Real-time updates
- Basic visual feedback
- ~6-8 hours total

### Step 7: Add Dynamics [Complex: 3-5 sessions]
- Envelope detection per band
- Threshold/ratio controls
- Gain reduction metering
- Attack/release controls
- ~6-8 hours total

### Step 8: Enhanced Features & Visualization [Complex: 3-5 sessions]
- Spectrum analyzer (ff_meters)
- Gain reduction visualization
- Sidechain support
- Advanced metering
- ~6-8 hours total

### Step 9: Optimization [Medium: 2-3 sessions]
- CPU profiling and optimization
- SIMD optimizations if needed
- Oversampling option
- Lookahead for dynamics
- ~4-5 hours total

### Step 10: Testing & Polish [Large: 5+ sessions]
- Complete test suite
- User documentation
- Preset system
- Final UI polish
- ~8-10 hours total

## Key Architecture
```cpp
// Static EQ with chowdsp (Steps 3-6)
class Band {
    chowdsp::EQ::EQFilter<float> filter;  // Professional filter
    float freq, gain, q;
    FilterType type;  // Bell, Shelf, Pass
};

// Dynamic EQ (Steps 7+)
class DynamicBand : public Band {
    EnvelopeFollower envelope;
    float threshold, ratio;
    float attack, release;
};
```

## Important Notes
- **Quality > Speed**: Take time to do it right
- **Ask > Assume**: Always clarify with user
- **Test Everything**: Each subtask needs verification
- **Document Decisions**: Future agents need context
- **chowdsp Integration**: Using professional filters from the start avoids rewriting later