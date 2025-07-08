# Step 7 Requirements - Add Dynamics

## User Requirements Documentation
**Date**: 2025-07-08
**Step**: 7 - Add Dynamics (Turn Static EQ into Dynamic EQ)

## 1. Dynamics Parameters per Band

### Threshold Range: -60dB to 0dB
- **Rationale**: Industry standard range covering full practical scope
- **Benefits**: Allows subtle dynamics on quiet signals and aggressive processing on loud signals
- **Implementation**: Linear dB scale with smooth parameter mapping

### Ratio Range: 1:1 to 20:1 (with ∞:1 option)
- **1:1 to 10:1**: Covers most musical applications
- **20:1**: Approaches limiting behavior
- **∞:1 or "LIMIT"**: True limiting setting
- **Implementation**: Logarithmic scaling for musical control

### Attack Time Range: 0.1ms to 300ms
- **0.1ms**: Very fast peak limiting
- **300ms**: Slow, musical compression
- **Scaling**: Logarithmic for intuitive control
- **Implementation**: Smooth parameter interpolation

### Release Time Range: 1ms to 3000ms (3 seconds)
- **1ms**: Fast pumping effects
- **3 seconds**: Very slow, program-dependent release
- **Special Feature**: Include "AUTO" mode that adapts to signal content
- **Implementation**: Logarithmic scaling with auto-release detection

### Knee Type: Adjustable knee (0-10)
- **0**: Hard knee (precise threshold)
- **10**: Very soft knee (gradual transition)
- **Benefits**: More flexible than binary hard/soft options
- **Implementation**: Smooth knee curve interpolation

## 2. Detection Type: User Selectable

### Three Detection Methods:
1. **Peak**: Fast, responsive, good for transient control
2. **RMS**: Smoother, more musical, better for program material  
3. **Blend**: Allow mixing between detection methods for optimal behavior

**Implementation**: Per-band detection type selection

## 3. Gain Reduction Display: Comprehensive Visualization

### All Display Options:
1. **Numeric display**: Precise dB values next to each band
2. **Meter visualization**: Small vertical meters per band
3. **Frequency response overlay**: Colored regions showing active gain reduction
4. **Global GR meter**: Overall gain reduction across all bands

**Implementation**: Real-time visual feedback for all dynamics activity

## 4. Dynamics Behavior: Multiple Selectable Modes

### Four Operation Modes:
1. **Compressive (default)**: Reduce gain when signal exceeds threshold
2. **Expansive**: Increase gain when signal exceeds threshold (presence enhancement)
3. **De-esser mode**: Specialized for harsh frequency reduction
4. **Gate mode**: Cut gain below threshold (downward expansion)

### Additional Features:
- **Per-band bypass**: Essential for A/B comparison
- **Implementation**: Mode selection per band

## 5. GUI Integration: Below Original Parameters

### Layout Requirements:
- **Position**: Add dynamics controls below existing EQ parameters
- **Organization**: Keep dynamics grouped per band
- **Visibility**: Always visible (no separate panel/tab needed)
- **Integration**: Maintain existing 4-band layout structure

## Implementation Priority
1. **Phase 1**: Core dynamics parameters (threshold, ratio, attack, release, knee)
2. **Phase 2**: Detection type selection
3. **Phase 3**: Gain reduction display
4. **Phase 4**: Multiple behavior modes
5. **Phase 5**: GUI integration and polish

## Technical Notes
- **Envelope Detection**: Implement smooth envelope followers
- **Parameter Smoothing**: Essential for audio-rate parameter changes
- **CPU Optimization**: Efficient dynamics processing per band
- **Real-time Safety**: No audio dropouts during parameter changes

## Success Criteria
- [ ] All dynamics parameters functional per band
- [ ] Smooth real-time parameter control
- [ ] Accurate gain reduction metering
- [ ] No audio artifacts or dropouts
- [ ] Intuitive GUI layout below existing controls
- [ ] Professional audio quality maintained