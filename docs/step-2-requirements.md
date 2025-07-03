# Step 2 Requirements - Parameter System

## User Requirements Gathered: 2025-07-03

### Total Parameter Planning
- **Complete Plugin Parameters**: 60-80 total
  - Static EQ phase: ~20 parameters
  - Dynamic phase: ~40 parameters  
  - Additional features: ~10-20 parameters
- **Rationale**: Comprehensive feature set while maintaining manageable complexity

### Initial Implementation (Step 2)
- **Output Gain** (master level control)
- **Input Gain** (optional, but useful for gain staging)

### Parameter Value Ranges and Scaling

#### Gain Parameters
- **Master Gain**: -24 dB to +12 dB (linear in dB)
- **Band Gain**: ±18 dB (linear in dB)
- **Scaling**: Linear in decibels for intuitive mixing

### Parameter Smoothing
- **Smooth Parameters** (20-50ms ramp):
  - All gain changes
  - Frequency adjustments
  - Q modifications
  - Threshold changes

### Default Values (Reset State)

#### Master Section
- **Input Gain**: 0 dB (unity gain)
- **Output Gain**: 0 dB (unity gain)

### Technical Implementation Details
- **Parameter Management**: AudioProcessorValueTreeState
- **Thread Safety**: Built-in with APVTS
- **Parameter IDs**: String-based (e.g., "master_gain", "band1_freq")
- **Automation**: All parameters fully automatable
- **Value Conversion**: Custom formatters for user-friendly display
- **Preset System**: Design for forward compatibility

### Implementation Notes
- Keep future expansion in mind
- Functionalize and clear expandable coding style
- Design for 60-80 total parameters eventually