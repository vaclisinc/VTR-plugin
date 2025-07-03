# Step 3 Implementation Plan - chowdsp Integration & Single Band EQ

## Overview
Integrate chowdsp_utils library and implement a single band EQ using State Variable Filters with Bell/Peak response.

## Subtasks (30-60 min each)

### Subtask 1: Add chowdsp_utils as Git Submodule (~30 min)
**Deliverable:** chowdsp_utils integrated as submodule
- Research latest stable chowdsp_utils version
- Add as git submodule to `third-party/` directory
- Verify submodule is properly initialized
- Test basic inclusion

### Subtask 2: CMake Integration (~45 min)
**Deliverable:** Build system recognizes chowdsp
- Modify CMakeLists.txt to include chowdsp
- Configure chowdsp modules (focus on EQ/filters)
- Ensure proper linking and compilation
- Test build succeeds

### Subtask 3: Single Band EQ Implementation (~60 min)
**Deliverable:** Working Bell filter with chowdsp State Variable Filter
- Create EQBand class using chowdsp::StateVariableFilter
- Implement Bell/Peak filter configuration
- Add processBlock method for audio processing
- Integration with existing audio processing chain

### Subtask 4: Parameter Integration (~45 min)
**Deliverable:** Parameters control chowdsp filter
- Connect frequency parameter (20Hz-20kHz)
- Connect gain parameter (±12dB)
- Connect Q parameter (0.1-10)
- Use existing parameter smoothing system
- Real-time parameter updates

### Subtask 5: Testing & Verification (~30 min)
**Deliverable:** Confirmed working EQ
- Test frequency response
- Verify gain control
- Test Q factor adjustment
- Confirm no audio artifacts
- Performance check

## Potential Challenges
1. **chowdsp Version Compatibility:** May need specific version for stability
2. **CMake Configuration:** chowdsp may have specific build requirements
3. **Parameter Mapping:** Ensuring proper parameter scaling for chowdsp
4. **Audio Thread Safety:** chowdsp filter updates on audio thread

## Success Criteria
- [ ] Plugin builds successfully with chowdsp
- [ ] Single band EQ audibly affects frequency response
- [ ] All three parameters (freq/gain/Q) work correctly
- [ ] No audio dropouts or artifacts
- [ ] Performance remains acceptable

## Estimated Total Time: 3.5-4 hours

## Next Steps After Completion
- Document implementation details
- Update progress tracker
- Prepare for Step 4 (Multiple Filter Types)