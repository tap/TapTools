# Fork provenance

This directory is a fork of **RtAudio**, the real-time audio I/O C++ library.

| | |
|---|---|
| Upstream | https://github.com/thestk/rtaudio |
| Forked at commit | `e5f0774b2156082ec3db998bd6b2a94b66ade8ac` |
| Upstream commit date | 2026-02-27 |
| Fork created | 2026-06-22 |

## Why this lives in a subdirectory

This fork is currently developed inside the `TapTools` repository under
`rtaudio/` because the working environment can only push to that repository.
The tree is intentionally **self-contained** (its own build files, tests, CI
under `rtaudio/.github/`, and config) so it can be lifted into a standalone
`rtaudio` repository at any time.

## Extracting into a standalone repository

```sh
# From the TapTools repo root, create a branch containing only rtaudio/ history:
git subtree split --prefix=rtaudio -b rtaudio-only

# Create an empty github.com/<you>/rtaudio repo, then:
git push git@github.com:<you>/rtaudio.git rtaudio-only:main
```

Once standalone, the workflows under `.github/workflows/` activate
automatically (move `rtaudio/.github` to the repo root if needed).

## Syncing with upstream

```sh
git remote add upstream https://github.com/thestk/rtaudio.git
git fetch upstream
# review/merge upstream changes into rtaudio/ as appropriate
```

## Fork changelog

See `CHANGES.fork.md` for the list of changes this fork makes on top of the
upstream baseline.
