# Engauge Digitizer (Windows fork)

Engauge Digitizer is an open-source tool for extracting data points from graphical images.

The official project stopped providing pre-compiled Windows binaries for recent versions, offering
only source code. This repository provides ready-to-use Windows packages compiled from the latest
source, **for closed-network (air-gapped) machines**, with a few extra opt-in features on top of the
upstream application.

## Download

Open the [latest release](../../releases/latest) and choose one of these packages:

- **Setup.exe**: standard Windows installation with Start Menu shortcuts and uninstall support
- **Portable.zip**: extract and run `Engauge.exe`; no installation is required

Both packages include every runtime file (Qt, FFTW, the help file and the translations), so nothing
is downloaded or installed at run time. The SHA256 checksums are published with each release.

These community binaries are not code-signed, so Windows SmartScreen may request confirmation.

## Features added in this fork

Every feature below is **opt-in**: a fresh installation behaves exactly like the upstream release
until each option is turned on in the settings.

| Feature | Where | What it does |
|---|---|---|
| Quality report | `Digitize > Quality Report...` | Lists the digitized points that deserve a second look: repeated or reversed x values, sudden slope changes, points that overlap another curve, and points outside the axes range. Saved as CSV. |
| Point origin display | `Settings > General` | Draws points that were placed by the Segment Fill or Point Match tools with a dashed outline, so they can be told apart from hand-placed points. |
| Delete automated points | `Edit > Delete Automated Points` (after enabling in `Settings > General`) | Removes every automated point in one undoable step, keeping the hand-placed points. Re-digitize the curve afterwards. |
| Auto-save and recovery | `Settings > General` | Writes a recovery copy while the document is modified. After an unexpected exit, the next start offers to recover it. |
| Korean and other languages | `Help > Language` | Switches the user interface language at run time. |

See [help/forkfeatures.html](help/forkfeatures.html) (also in the built-in help index) for the full
descriptions.

## Closed-network notes

- Everything runs locally: no network access, no telemetry, no downloads.
- The recovery files and the quality report are written to the local application data directory (or
  next to the application for a portable installation).

## Project links

- [Official project website](https://akhuettel.github.io/engauge-digitizer/)
- [Official upstream repository](https://github.com/akhuettel/engauge-digitizer)
- [Releases](../../releases)
- [Build workflows](../../actions)
- [Citation DOI](https://zenodo.org/badge/latestdoi/26443394)

The original application was created and maintained by Mark Mitchell. The upstream source was
restored and is maintained by Andreas K. Hüttel and other contributors. This fork retains the
upstream copyright and licensing notices.

## License

Engauge Digitizer is distributed under the GNU General Public License version 2 or, at your option,
any later version. See [LICENSE](LICENSE).
