# Requirements Gathering Template for VTR-Plugin Integration

## This template MUST be used by agents before implementing each step

### Step-Specific Requirements Checklist

#### For ANY Step:
- [ ] Have I asked about ALL technical specifications?
- [ ] Have I asked about design preferences?
- [ ] Have I asked about priority features?
- [ ] Have I documented all answers?
- [ ] Am I making ANY assumptions? If yes, have I asked the user?

#### For Step 1 (VTR Model Analysis & Integration Planning):
- [ ] **VTR Model Details:**
  - What ML framework is the VTR model built with? (PyTorch, TensorFlow, ONNX, etc.)
  - What are the input/output dimensions and formats?
  - What sample rates does the VTR model support?
  - What is the model's expected latency/processing time?
  - Are there pre-trained weights available?
- [ ] **Integration Approach:**
  - Should we use Python bridge, C++ conversion, or ONNX runtime?
  - Preferred approach for real-time inference?
  - Memory requirements and constraints?
  - Threading requirements?

#### For Step 2 (VTR Model Integration Foundation):
- [ ] **Model Loading:**
  - Where will model files be stored? (embedded, external files, download?)
  - Multiple model support needed?
  - Model versioning requirements?
  - Fallback behavior if model fails to load?
- [ ] **Audio Processing:**
  - Buffer size requirements for VTR model?
  - Input preprocessing needed (normalization, windowing, etc.)?
  - Output postprocessing needed?
  - Handle variable buffer sizes?

#### For Step 3 (Real-time Processing Integration):
- [ ] **Performance Requirements:**
  - Target CPU usage percentage?
  - Maximum acceptable latency?
  - Minimum system requirements?
  - Quality vs. performance trade-offs?
- [ ] **Processing Architecture:**
  - Parallel processing with existing plugin features?
  - Separate audio thread for VTR processing?
  - Queue-based processing or direct processing?
  - Graceful degradation under high CPU load?

#### For Step 4 (Parameter Control System):
- [ ] **VTR Parameters:**
  - What parameters does the VTR model expose?
  - Parameter ranges and default values?
  - Parameter smoothing requirements?
  - Parameter automation support needed?
- [ ] **User Interface:**
  - How should VTR parameters be presented?
  - Grouping of related parameters?
  - Advanced vs. basic parameter views?
  - Parameter presets/scenes?

#### For Step 5 (Audio Processing Pipeline):
- [ ] **Signal Flow:**
  - VTR processing before or after existing EQ?
  - Parallel processing or serial?
  - Wet/dry blend control needed?
  - Bypass functionality requirements?
- [ ] **Audio Routing:**
  - Stereo, mono, or multi-channel support?
  - Sidechain input for VTR processing?
  - Multiple VTR instances per plugin?
  - Cross-band VTR processing?

#### For Step 6 (GUI Integration):
- [ ] **Visual Design:**
  - Separate VTR section or integrated with existing GUI?
  - Visual feedback for VTR processing?
  - Real-time parameter visualization?
  - VTR enable/disable controls?
- [ ] **User Experience:**
  - Collapsible VTR section?
  - Tooltip help for VTR parameters?
  - Visual indication of VTR processing state?
  - Error/status display for VTR model?

#### For Step 7 (Performance Optimization):
- [ ] **Optimization Targets:**
  - CPU usage goals?
  - Memory usage constraints?
  - Latency requirements?
  - Battery life considerations (mobile)?
- [ ] **Optimization Techniques:**
  - Model quantization acceptable?
  - Reduced precision processing?
  - Batch processing vs. real-time?
  - GPU acceleration desired?

#### For Step 8 (Testing & Validation):
- [ ] **Testing Requirements:**
  - Specific audio test signals?
  - A/B testing with reference implementations?
  - Automated testing needed?
  - Performance benchmarking?
- [ ] **Validation Criteria:**
  - How to measure tone replication accuracy?
  - Acceptable quality metrics?
  - Stress testing requirements?
  - Compatibility testing scope?

### Documentation Requirements

For each step, create:
1. `docs/step-X-requirements.md` - User's answers to all questions
2. `docs/step-X-implementation-plan.md` - Detailed implementation plan with subtasks
3. `docs/step-X-explanation.md` - Technical implementation details after completion

### Example Requirements Gathering

```markdown
# Step 1 Requirements - VTR Model Analysis & Integration Planning

## Questions for User:

1. **What ML framework is the VTR model built with?**
   - PyTorch (most common for research)
   - TensorFlow (good for production)
   - ONNX (framework-agnostic)
   - Custom implementation

2. **What are the VTR model's input/output specifications?**
   - Input: audio samples, features, or spectrograms?
   - Output: processed audio, parameters, or features?
   - Expected sample rate and buffer sizes?
   - Mono or stereo processing?

3. **What integration approach do you prefer?**
   - Python bridge (easier but potentially slower)
   - C++ conversion (faster but more complex)
   - ONNX runtime (good balance)
   - Other runtime/framework?

4. **What are the performance requirements?**
   - Maximum acceptable latency (ms)?
   - Target CPU usage percentage?
   - Memory constraints?
   - Real-time processing mandatory?

Please answer these questions so I can plan the VTR integration approach.
```

### IMPORTANT REMINDERS:

1. **NEVER PROCEED WITHOUT ANSWERS** - Get explicit user confirmation
2. **IF USER SAYS "YOU DECIDE"** - Give 2-3 specific options with pros/cons
3. **DOCUMENT EVERYTHING** - Future agents need to understand all decisions
4. **ASK FOLLOW-UP QUESTIONS** - If answers are unclear or incomplete
5. **NO ASSUMPTIONS** - When in doubt, always ask!
6. **VTR CONTEXT** - We're building on existing plugin foundation with ML integration
7. **REAL-TIME CRITICAL** - Audio processing has strict timing requirements