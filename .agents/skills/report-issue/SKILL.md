---
name: report-issue
description: File a GitHub issue on the repository that causes a problem - evan, guillaume, mais, utility or this application. Use as soon as you find a bug, a build or documentation problem, a missing feature or anything else worth fixing while working here, including problems you worked around and problems unrelated to the current task.
---

# Report an issue where its cause is

When you find a problem, file an issue on the repository whose code has to
change. Do this even when you worked around it and even when it is not what
you were asked to do: a workaround that is not reported is lost.

Create the issue without asking first, then give its link in your answer. Do
not stop the current task for it, and do not fix a library from this
repository.

## 1. Find the repository of the cause

Decide from evidence (a file and line, a log line, a failing command), not
from where the symptom shows.

| The code that has to change is in | Repository |
| --- | --- |
| `src/`, `scripts/`, `tests/`, `cmake/`, `tools/`, docs or CI of this repository | `etib-corp/getting-started` |
| `build/_deps/evan-src/` (window, Vulkan, rendering, shaders) | `etib-corp/evan` |
| `build/_deps/guillaume-src/` (widgets, layout, scenes, theme, storage) | `etib-corp/guillaume` |
| `build/_deps/mais-src/` (Python runtime, hooks, errors) | `etib-corp/mais` |
| `build/_deps/utility-src/` (resources, fonts, logging, events, math) | `etib-corp/utility` |

- A log line names its source: `[evan::...]`, `[guillaume::...]`,
  `[utility::...]`, and the file path in debug lines.
- A mismatch between two libraries goes to the one that has to change. When
  that is unclear, file it on the consumer of the two and say so.
- When the cause is not established, file it on `etib-corp/getting-started`
  with what is known, and say that the cause is not confirmed.
- pybind11, GLFW, Vulkan and other third-party code: file it on the XIDER
  library that uses them, not upstream.

## 2. Check it is not already reported

```sh
gh issue list --repo etib-corp/<repo> --state all --search "<two or three key words>"
```

If an issue exists, add a comment with the new facts instead of opening a
second one. If it was closed as fixed, check the pinned revision in
`cmake/Dependencies.cmake`: the fix may only need a newer revision (see the
`bump-libraries` skill).

## 3. Write it

Use the repository's own types. They are the same in the four repositories.

| Kind | Title prefix | Label |
| --- | --- | --- |
| Something does not work | `bug:` | `bug` |
| Configure, build or CI problem | `build:` | `build` |
| Wrong or missing documentation | `docs:` | `documentation` |
| Missing capability | `feat:` | `enhancement` |
| Code that should be restructured | `refactor:` | `enhancement` |

The body follows the bug template of the libraries:

```markdown
## Context

What was being done, the exact command or code, and how to reproduce it.
Library revision (from cmake/Dependencies.cmake), OS, compiler, Python
version when relevant.

## Proofs

The error or log lines (twenty at most), with file:line in the library.
Expected result and actual result.

## Ways to fix

The suspected cause and a possible fix, if known. The workaround used in
getting-started, if any, and where it is.

---
Found while working on etib-corp/getting-started. Reported by an AI agent.
```

Rules for the text:

- One problem per issue. State facts you observed; mark every guess as a
  guess.
- Reproduce it before reporting when that is possible, and say whether you
  did.
- Remove anything private: user names and home directories in paths
  (write `<repo>/src/...`), tokens, e-mail addresses, anything from the
  user's other projects. The repositories are public.
- No assignee, no milestone, no priority label: maintainers decide those.

## 4. Create it

```sh
gh issue create --repo etib-corp/<repo> \
  --title "bug: <short description>" \
  --label bug \
  --body-file <file with the body>
```

If `gh` is missing, not logged in, or the label does not exist, do not drop
the report: retry without the label, and if it still fails, give the user
the repository, the title and the full body so they can file it.

## 5. Leave a trace here

- Give the issue link in your final answer.
- When this repository contains a workaround, put the link in a comment next
  to it, so it can be removed once the issue is closed:
  `// Workaround for https://github.com/etib-corp/<repo>/issues/<n>`
