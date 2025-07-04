# Step 4 Requirements - Filter Types

## User Requirements (from conversation 2025-07-03)

### 1. Filter Types to Implement
- **Bell/Peak** (already working from Step 3)
- **High Shelf** 
- **Low Shelf**
- **High-pass**
- **Low-pass**
- All using chowdsp filters

### 2. Filter Behavior
- **Butterworth** characteristics (maximally flat passband)

### 3. UI Design
- Simple icon-based selection for easy user interaction
- User wants easy clicking interface rather than dropdown
- Icons should be intuitive for each filter type

## Implementation Notes
- Build on existing chowdsp integration from Step 3
- Use Butterworth response for all filter types
- Focus on simplicity in UI design
- Maintain professional audio quality with chowdsp

## Next Steps
1. Create implementation plan with subtasks
2. Design simple icon-based filter selection
3. Implement each filter type using chowdsp
4. Test all filter types work correctly