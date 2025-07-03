# Step 3 Requirements - chowdsp Integration & Single Band EQ

## User Answers

### 1. chowdsp Integration Method
**Answer:** Git submodule
- Easier updates and version management
- Keeps external dependencies separate

### 2. Initial Filter Type
**Answer:** Bell/Peak filter
- Most versatile for EQ applications
- Good starting point for single band implementation

### 3. Parameter Ranges
**Answer:** 
- Frequency: 20Hz-20kHz (standard - no change needed)
- Gain: ±12dB (reduced from typical ±18dB)
- Q factor: 0.1-10 (standard - no change needed)

### 4. chowdsp Feature Focus
**Answer:**
- **Primary:** State Variable Filters with higher-order options
- **Secondary:** SIMD optimizations (often automatic)
- **Skip:** Parameter smoothing (use existing system)

## Implementation Notes
- Use chowdsp's State Variable Filter implementation
- Leverage higher-order filter capabilities for professional sound
- Maintain existing parameter smoothing system from Step 2
- SIMD optimizations should be automatic with chowdsp

## Technical Decisions
- Filter Type: Bell/Peak using chowdsp State Variable Filter
- Parameter System: Integrate with existing ParameterManager
- Build System: Add chowdsp as git submodule to CMake
- Performance: Leverage chowdsp's built-in optimizations