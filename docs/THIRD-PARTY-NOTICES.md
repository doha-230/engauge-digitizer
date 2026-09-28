# Third-party notices

The Windows packages of this fork contain the components listed below in addition to Engauge
Digitizer itself. Nothing in this fork is code-signed, and no component has been modified.

## Engauge Digitizer (this application)

- License: GNU General Public License version 2 or, at your option, any later version (GPL-2.0-or-later)
- License text: `LICENSE` in the package root and in this repository
- Source: this repository, and upstream <https://github.com/akhuettel/engauge-digitizer>
  (original author: Mark Mitchell; source restored and maintained by Andreas K. Hüttel and
  other contributors)

## Qt 6 (dynamic libraries, plugins and Qt translation catalogs)

The packages contain the Qt 6 libraries deployed by `windeployqt` — `Qt6Core.dll`, `Qt6Gui.dll`,
`Qt6Widgets.dll`, `Qt6Help.dll`, `Qt6Sql.dll` and any further Qt 6 module library that
`windeployqt` deploys — together with the platform plugin `platforms\qwindows.dll`, the image format
and SQLite plugins needed by the help engine, and the Qt translation catalogs `translations\qt_*.qm`.

- Version: Qt 6.8.3 (MSVC 2022 64-bit), the version used by the build workflows
- Copyright: The Qt Company Ltd. and contributors
- License: GNU Lesser General Public License version 3 (LGPL-3.0)
- License text: `licenses/LGPL-3.0.txt` in the package and
  <https://www.gnu.org/licenses/lgpl-3.0.txt>

The LGPL-3.0 obligations are met as follows:

- Qt is used as a **dynamically linked** library and was **not modified**.
- The complete license text is shipped with the package (`licenses/LGPL-3.0.txt`).
- Because the Qt libraries are separate DLLs next to `Engauge.exe`, the Qt 6 libraries in the package
  can be **replaced by a compatible build of Qt 6** of your own, which is the relinking mechanism
  required by LGPL-3.0 section 4 (d)(1). Replacing them changes only the library, not the application.
- Qt 6 source code is available from <https://download.qt.io/official_releases/qt/> and
  <https://code.qt.io/>.

## FFTW 3

The packages contain `libfftw3-3.dll`.

- Version: 3.3.5
- Copyright: Matteo Frigo and Steven G. Johnson
- License: GNU General Public License version 2 or later (GPL-2.0-or-later)
- License text: `LICENSE` in the package (the GPL version 2 text), and <https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
- Source: <https://www.fftw.org/>

## Microsoft Visual C++ runtime

`windeployqt --compiler-runtime` copies the Microsoft Visual C++ runtime DLLs (`vcruntime140.dll`,
`msvcp140.dll` and related files) next to the executable. They are redistributed under the
Microsoft Visual C++ Redistributable license terms, which permit distribution with applications
built with Visual Studio; the license text cannot be redistributed here, see
<https://visualstudio.microsoft.com/license-terms/>.

## Not included

PDF, JPEG 2000 and network features are not part of the Windows packages: no Poppler, OpenJPEG or
Qt Network libraries are shipped. Poppler is optional in the upstream source and can be enabled in
a build of your own (`CONFIG+=pdf` for qmake, see `DEPENDENCIES.md`).

## Reporting

If a component is missing from this list, please open an issue — a complete notice is a release
requirement, not a formality.
