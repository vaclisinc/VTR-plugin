## | ULTRA THINK DIRECTIVE

Read `checklist.md` and `./CLAUDE.md` in the current directory. Ultra-think through dependencies, test requirements, and Continuous Validation rules before writing any code. Consult `spec.md` for architectural details, algorithm choices, and risk notes.

---

## TASK

Execute every task in `checklist.md` sequentially (respecting deps), using strict TDD:

1.  For each unchecked bullet:
    1.  Write the failing test(s) listed in `tests:`.
    2.  Run the test to confirm it fails.
    3.  Implement the minimal code to make the test pass.
    4.  Refactor if safe.
    5.  Run all unit tests. If the task is SB1-SB4, run pluginval and pink-noise check. GUI-only tasks run the UI screenshot sanity test; DSP or build-system tasks run the full Continuous Validation trio.
    6.  When tests pass, edit `checklist.md` in place, changing the task's checkbox from `[ ]` to `[x]`.

2.  After marking the item complete, git commit immediately:
    ```
    git add .
    git commit -m "✓ {task-ID} {objective (short)}"
    ```

3.  Proceed to the next task until every bullet is checked.

---

## CONTEXT

* Adhere to all conventions in `../CLAUDE.md` (research-first, universal binary, Continuous Validation).
* If authoritative detail is missing, query the Context7 MCP server; otherwise perform web search with citations.
* If both fail, pause execution and request clarification.

---

## OUTPUT

While running, output only:

* The test results summary.
* A one-line confirmation after each commit:
    `Committed {task-ID} — {status}`

Do not flood stdout with code listings (they're in the repo diffs).

When all tasks are complete, output: `BUILD COMPLETE — all checklist items checked`.

---

## | GLOBAL RULES

* Never skip a failing test.
* Never mark a checklist item complete until all tests (unit + validation) pass.
* Keep each commit focused on a single task.
* If CI or Continuous Validation fails, fix issues **before** moving on.
* Do not overwrite existing files unless required by the task.
* Do not push to remote; local commits are sufficient.