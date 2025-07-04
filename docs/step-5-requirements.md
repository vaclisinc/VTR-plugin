# Step 5 Requirements - Multi-Band System

## User Specifications

### 1. Band Configuration
**4 bands with specific frequency ranges:**
- **Low**: 20Hz - 200Hz
- **Low-Mid**: 200Hz - 800Hz  
- **High-Mid**: 800Hz - 3kHz
- **High**: 3kHz - 20kHz

### 2. Band Management Features
- **Enable/Disable Toggle**: Each band must have independent enable/disable control
- **Solo Feature**: Each band must have solo capability with independent parameters
- **Linkable Bands**: Useful for linking L and R channels (stereo linking)

### 3. Crossover Implementation
- **Technology**: Use chowdsp::EQ for implementation
- **Crossover Type**: User-adjustable crossover points with overlapping bands
- **Flexibility**: Allow frequency range adjustments per band

### 4. Control Architecture
- **Global Controls**: Only input/output level controls are global
- **Independent Controls**: All EQ parameters (Filter Type, Frequency, Gain, Q) are independent per band
- **Per-Band Parameters**: Each band has its own complete parameter set

## Technical Implementation Notes

### Current Architecture Integration
- Build upon existing single-band system from Step 4
- Maintain ParameterManager scalability for 4x parameter count
- Preserve filter type system (Bell, Shelf, High/Low Pass)

### Parameter Count Impact
- Single band: 5 parameters (Input, Filter Type, Freq, Gain, Q, Output)
- Multi-band: ~22 parameters (2 global + 4 bands × 5 parameters each)
- Plus band controls (Enable/Disable, Solo, Link): +12 parameters
- **Total**: ~34 parameters (well within ParameterManager capacity)

### Processing Architecture
- Maintain stereo processing capability
- Support overlapping frequency ranges
- Ensure clean band isolation when needed
- Implement efficient crossover filtering

## Implementation Priority
1. Core 4-band EQ processing
2. Enable/disable functionality
3. Solo feature implementation
4. User-adjustable crossover points
5. Stereo linking capability (if time permits)

## Success Criteria
- [ ] 4 independent EQ bands processing audio
- [ ] Each band can be enabled/disabled
- [ ] Solo feature works per band
- [ ] Crossover points are user-adjustable
- [ ] All filter types work in each band
- [ ] Stereo processing maintained
- [ ] No audio artifacts between bands
- [ ] Plugin remains stable and responsive