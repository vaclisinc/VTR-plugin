# JUCE Plug-in spec.md Template

## ULTRA THINK DIRECTIVE
Ultra-think through every file in `/research/` and the repository-level `CLAUDE.md` before writing.  
Factor in: recurring patterns, best-practice notes, relevance scores, licenses, known_issues, and Continuous Validation standards.

---

## TASK
Distill those insights into a concise, senior-engineer `spec.md` that will guide checklist generation and coding.

---

## OUTPUT
Create exactly **one** Markdown file named `spec.md` in the current working directory.  
Write **nothing else** to stdout.

---

## spec.md REQUIRED SECTIONS (in order)

### 1. Plugin Purpose & UX Vision
Plain-English statement (≤120 words) of what the plug-in does and why it matters.

---

### 2. Core Functional Requirements
Bullet list (≤8) of non-negotiable audio/DSP behaviours.

---

### 3. Technical Architecture Overview
- ASCII or Mermaid diagram of major modules.
- Bullets naming recommended DSP algorithms / external libs with short rationale.

---

### 4. Repo & Paper Leverage Matrix
YAML list (top 8 artefacts) with keys:
- `source` (repo\|paper)
- `name`
- `url`
- `role_in_project`
- `reason_for_rank`

Rank using the `relevance`, `license`, and `known_issues` fields from research.  
If any artefact is GPL/AGPL, mark `license_risk: high`.

---

### 5. Build Feasibility & Risk Assessment

| Area        | Complexity (1–5) | Risk (1–5) | Mitigation         |
|-------------|------------------|------------|---------------------|
| Example DSP | 4                | 3          | Use fallback lib    |

Reference any `known_issues` and Continuous Validation constraints.

---

### 6. Validation Strategy
- Outline an LLM-driven test harness that employs the three Continuous Validation tests (`pluginval`, offline render, CPU/glitch) plus any custom unit tests implied by the research.
- Treat “Smoke Build Gate” (compile + pluginval + pink-noise check + UI screenshot) as the **first red test**; it must pass before any DSP or GUI code is enabled.

---

### 7. Open Questions
≤ 5 succinct bullets listing decisions needed before the checklist phase.

---

## GLOBAL RULES
- Entire file ≤ 700 words.
- **No** step-by-step checklist yet.
- **No** code snippets beyond the architecture diagram.
- Cite or weigh pre-2015 sources only if still state-of-the-art.
- Prioritise clarity, implementability, and pragmatic trade-offs.
- For any DSP block or GUI subsystem with JUCE source reference, Claude **MUST** call this out explicitly in `Open Questions`.
- If any required dependency is present, flag this in the **Architecture** section under "Dependencies Required".