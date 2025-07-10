# JUCE Plug-in Build Checklist Prompt

## ULTRA THINK DIRECTIVE
Ultra-think through `spec.md` in the current directory, reasoning exhaustively about all requirements, constraints, and algorithmic complexity **before** writing anything.

+ Assume a working JUCE plug-in skeleton already exists in the repository.  
+ Do **not** add tasks for generating or configuring the initial JUCE/Projucer/CMake project.

---

## TASK
Transform those insights into a phase-based **build checklist** that guarantees the JUCE plug-in is completed via strict **Test-Driven Development (TDD)**.

---

## AUTOMATIC GRANULARITY RULE

For any requirement that **either**:
1. demands novel DSP/math (e.g., reverb core, granular pitch-shift, modulation engine), **or**
2. is likely to exceed ≈ 50 lines of production code,  

**explode it into atomic sub-tasks** so that each bullet creates:
- at most one source + one test file, **or**
- adds ≤ 20 lines to an existing file.

> Routine plumbing (parameter binding, code-signing, docs) may remain a single high-level item.

Always include bullets for the **three Continuous Validation tests** defined in `/CLAUDE.md` (pluginval, offline render, CPU/glitch) inside the Integration phase.

---

## OUTPUT

Create `checklist.md` (the output file) in the working directory **without overwriting this prompt**.  
Write **nothing else** to stdout.

---

## checklist.md FORMAT

Group work into logical phases. The phases **MUST** appear **in this order**:
'''
SmokeBuild → Env → DSP → UI → Integration → Packaging
'''
> *Phase names are fixed; sub-phase splits inside each block are flexible—optimise for clarity.*

The checklist **must** start with a **SmokeBuild** phase containing tasks **SB1–SB4** that implement the Smoke Build Gate exactly.

---

### Checklist Bullet Structure

Every task—top-level or exploded—**must** start with `- [ ]` and include a unique ID (e.g., `T1.1`, `T2.3.b`).

Each checklist bullet **must** contain the following keyed lines **in exact order**:

- `objective:` one-sentence purpose  
- `tests:` file(s) to create that *initially fail*  
- `implementation_hint:` ≤ 1 line (no code)  
- `done_when:` unambiguous pass condition  
- `deps:` *(optional)* comma-separated IDs of prerequisite tasks

---

## GLOBAL RULES

- Tasks that only affect UI layout or cosmetics may group up to **100 LOC per bullet**; DSP or threading tasks must remain ≤ 20 LOC.
- Follow TDD rigorously: *failing test → minimal code → pass → refactor*
- If a task stays red after **three** test–fix cycles, pause execution and ask Lex.
- Cap total bullets at **≤ 120**; explode **only** when it adds clarity.
- Ground every task in validated best practice via Context7; avoid assumptions.
- **No prose outside checklist items.**