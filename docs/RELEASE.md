# Release checklist (Windows fork)

Every release is produced by `.github/workflows/release-windows.yml` from a tag, and published as a
draft that is inspected before it goes public. Work through the list in order.

## 1. Prepare the version

The version string lives in more than one place, and a mismatch fails the release job's
"Validate release version" step. Update all of them:

| File | What to change |
|:--|:--|
| `CMakeLists.txt` | `project(EngaugeDigitizer VERSION x.y.z LANGUAGES CXX)` |
| `src/util/Version.cpp` | `VERSION_NUMBER` |
| `dev/windows/engauge_qt6.iss` | `MyAppVersion` (`x.y.z`) and `MyAppFileVersion` (`x.y.z.0`) |
| `dev/windows/package_cmake_qt6.ps1` | the `$Version` default |
| `README.md` | the badge and download sections stay version-free on purpose — check the asset names and the feature list instead |

While the version is being bumped, re-check the numbers that appear in prose: the unit test count in
`README.md` and in the `ci-test.yml` job name must match `src/build_and_run_all_cli_tests`, and the
GUI regression count must match `test/*.test.commandline`.

## 2. Green main

- `.github/workflows/ci-test.yml` green on `main` (both jobs, ~7 minutes).
- `python3 tools/check_shortcut_conflicts.py` green (it runs in CI as well).

## 3. Screenshots (needed once, then keep them current)

`README.md` currently has no screenshot: the images that ship with the help file are old-style
windows and crops of the graph area only. Capture these on Windows and add them as
`docs/images/`:

1. `docs/images/auto-curve-detection.png` — the main window right after one click with the **Auto
   Curve Detection** tool on a graph with scan gaps (points created along the whole curve).
2. `docs/images/quality-report.png` — the `Digitize > Quality Report...` dialog with a few listed
   candidates.

Then insert at the top of both the English and the Korean section:

```markdown
![Auto Curve Detection](docs/images/auto-curve-detection.png)
```

## 4. Build the release

1. Run the release workflow manually first, with the planned tag as the ref and no tag pushed:
   `Actions > Build Windows release > Run workflow` → it produces artifacts and stops before
   publishing. This validates the packaging script, Inno Setup packaging and the install test.
2. Push the tag (`vX.Y.Z`) and let the workflow build the draft release.
3. Check the draft release:
   - exactly three assets: `Engauge-Digitizer-X.Y.Z-Windows-x64-Setup.exe`,
     `Engauge-Digitizer-X.Y.Z-Windows-x64-Portable.zip`, `SHA256SUMS.txt`
   - `SHA256SUMS.txt` matches the two artifacts (`sha256sum -c`)
   - unpacking the portable package shows the documents in its root (`LICENSE`, `README.md`,
     `OFFLINE_UPDATE.md`, `THIRD-PARTY-NOTICES.md`, `licenses/LGPL-3.0.txt`) and `vc_redist.x64.exe`
   - release notes name the features and the upstream commits that came in
4. Install the `Setup.exe` once by hand: silent install, launch, uninstall all run in CI, but a
   human should see the Start Menu entry and the version in `Help > About`.

## 5. Publish and follow up

- Publish the draft release.
- After publishing, re-check the upstream pull requests and issues that came out of this fork's work
  (duplicate shortcut, regression harnesses, Segment Fill menu entry, help build) and rebase them on
  the current upstream `master` if they did not land.
