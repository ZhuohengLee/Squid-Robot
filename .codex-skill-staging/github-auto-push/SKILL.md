---
name: github-auto-push
description: Verify, document, commit, and push intended code changes to GitHub after editing. Use when a task involves modifying files in a git repository and the finished work should be compiled or built successfully, checked against the repository README, updated in the README if behavior or architecture changed, then committed and pushed to the user's GitHub remote.
---

# GitHub Auto Push

Follow this workflow after making code changes.

## Establish Git Context

Run `git status --short`, `git branch --show-current`, and `git remote -v` before editing.

Identify whether the worktree is already dirty before the task starts.

Separate pre-existing changes from the files touched for the current task. Never assume all modified files belong to the current request.

If the repository does not have an obvious GitHub remote, or pushing would require choosing between multiple plausible remotes, stop and ask the user.

## Read The README Contract

Read the repository's main `README.md` before finalizing the task. Treat it as the user-facing contract unless the user points to another authoritative document.

Identify the README sections that describe the code paths touched by the task, including behavior, architecture, setup, commands, wiring, interfaces, outputs, or operating modes.

Compare the final implementation against those README statements. If the code now behaves differently, update the README in the same task before committing.

Do not leave the repository in a state where the pushed code and README disagree about the behavior that the user is likely to rely on.

## Make Changes

Complete the requested code changes first.

## Verify Compilation Or Build

Run a real compile or build check before committing.

Prefer the build or compile commands documented in the README. If the README does not define them, infer the smallest reliable compile command from the repository's existing toolchain or project files.

If the task affects multiple documented targets, compile every affected target. If the target impact is unclear in a small multi-target firmware repository, prefer compiling all documented targets.

If compilation or build verification fails, fix the problem before creating a commit unless the user explicitly asks to push a known-broken state.

Run any additional validation that is relevant to the changed files after the compile check.

## Commit Safely

Run `git status --short` again before staging.

Stage only the files that belong to the current task. Prefer targeted `git add -- <path>` commands over staging the whole repository.

Do not include unrelated pre-existing edits in the commit. If unrelated edits are mixed into the same file and cannot be separated safely, stop and ask the user.

Write a concise commit message that describes the user-visible intent of the change.

Use non-interactive git commands only.

If there is nothing to commit for the current task, do not create an empty commit unless the user explicitly asks for one.

## Push Safely

Push only after the code changes are complete, the build or compile check passes, the README is aligned, and the commit is created.

Prefer `git push` when the current branch already tracks the correct remote branch.

If the current branch has no upstream and `origin` is the GitHub remote, use `git push -u origin <current-branch>`.

If push is rejected because the remote moved, inspect the state before taking action. Do not force-push unless the user explicitly asks for it.

## Report Back

Tell the user:

- which branch was pushed
- the new commit hash
- which compile or build commands ran
- whether the README was checked and updated
- whether validation ran
- whether the push succeeded

If push could not be completed, explain the blocker precisely.
