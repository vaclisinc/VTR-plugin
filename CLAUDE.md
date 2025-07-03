# 🚨 AGENT README - READ THIS FIRST!

## Your Workflow
1. Read this file
2. Check `agent/progress-tracker.md` for current status
3. Read `agent/PROJECT-ROADMAP.md` for step details
4. **ASK USER FOR REQUIREMENTS** before implementing
5. Document answers, then implement

## Critical Rules
- **NEVER ASSUME** - Always ask the user
- **ONE STEP AT A TIME** - Don't skip ahead
- **CLEAN CODE** - Prioritize expandability over features
- **TEST EVERYTHING** - Unit tests + Function tests + user confirmation



## File Structure
```
agent/
├── PROJECT-ROADMAP.md       # The implementation roadmap with time estimates
├── progress-tracker.md      # Current status
└── requirements-template.md # Questions to ask user
CLAUDE.md                    # This file (quick start guide)
docs/                        # Step documentation goes here
Source/                      # All code goes here
```

## Before Starting ANY Step

### 1. Check Progress
Open `agent/progress-tracker.md` and see what's next

### 2. Ask Requirements
Use `agent/requirements-template.md` to ask user about:
- Technical specifications
- Design preferences
- Feature priorities

### 3. Document Answers
Create `docs/step-X-requirements.md` with user's answers

### 4. Create Implementation Plan
Based on requirements, write `docs/step-X-implementation-plan.md`:
- Break step into subtasks (aim for 30-60 min each)
- Define clear deliverables
- Identify potential challenges
- Get user approval before proceeding

### 5. Only Then Implement
Execute the approved subtasks one by one

### 6. Provide instructions of how to test in DAW
Ask user if there is any unexpected outcome. If it really happens, go back to step 4 (Create Implementation Plan) to fix the issues. Only proceed to step 7 when user confirms no bugs.

### 7. Write trace code docs "step-X-explanation.md"
Use for trace code so should explain details about each new added function purpose

### 8. Update progress tracker

### 9. git add files, write commit message, and push (requires user authorization)
commit message NO NEED to say "written by claude code"
**Remember: Step 9 needs user authorization before proceeding**

## Example First Message
```
I've read the agent instructions and checked agent/progress-tracker.md.
I see we're ready for Step 1: Audio Pass-Through.

Before implementing, I need to ask you some questions:
[Insert relevant questions from agent/requirements-template.md]
```

## Remember
- Quality > Speed
- Ask > Assume
- Document > Memory