# Third-party notices

The Windows packages of this fork contain the components listed below in addition to Engauge
Digitizer itself. Nothing in this fork is code-signed, and no third-party component has been
modified. The list matches the contents of the released packages; the exact file list is inside each
package, and `SHA256SUMS.txt` covers the package files.

## Engauge Digitizer (this application)

- File: `Engauge.exe`, `documentation/engauge.qch`, `documentation/engauge.qhc`, `translations/engauge_*.qm`
- License: GNU General Public License version 2 or, at your option, any later version (GPL-2.0-or-later)
- License text: `LICENSE` in the package root and in this repository
- Source: this repository, and upstream <https://github.com/akhuettel/engauge-digitizer>
  (original author: Mark Mitchell; source restored and maintained by Andreas K. Hüttel and other
  contributors)

## Qt 6 (dynamic libraries, plugins and Qt translation catalogs)

The packages contain the Qt 6 libraries and plugins deployed by `windeployqt`:

| Files | Note |
|:--|:--|
| `Qt6Core.dll`, `Qt6Gui.dll`, `Qt6Widgets.dll`, `Qt6Help.dll`, `Qt6Sql.dll`, `Qt6Svg.dll`, `Qt6Xml.dll`, `Qt6PrintSupport.dll` | Qt 6 modules |
| `platforms/qwindows.dll`, `styles/qmodernwindowsstyle.dll`, `imageformats/qgif.dll`, `imageformats/qico.dll`, `imageformats/qjpeg.dll`, `imageformats/qsvg.dll`, `iconengines/qsvgicon.dll`, `generic/qtuiotouchplugin.dll`, `help/helpplugin.dll`, `sqldrivers/qsqlite.dll` | Qt 6 plugins (the GIF/JPEG/SVG image support comes from here) |
| `opengl32sw.dll` | Qt 6 software OpenGL fallback |
| `translations/qt_*.qm` | Qt 6 translation catalogs |

- Version: Qt 6.8.3 (MSVC 2022 64-bit), the version used by the build workflows
- Copyright: The Qt Company Ltd. and contributors
- License: GNU Lesser General Public License version 3 (LGPL-3.0)
- License text: `licenses/LGPL-3.0.txt` in the package and <https://www.gnu.org/licenses/lgpl-3.0.txt>

The LGPL-3.0 obligations are met as follows:

- Qt is used as a **dynamically linked** library and was **not modified**.
- The complete license text is shipped with the package (`licenses/LGPL-3.0.txt`).
- Because the Qt libraries are separate DLLs next to `Engauge.exe`, the Qt 6 libraries in the package
  can be **replaced by a compatible build of Qt 6** of your own, which is the relinking mechanism
  required by LGPL-3.0 section 4 (d)(1). Replacing them changes only the library, not the application.
- Qt 6 source code is available from <https://download.qt.io/official_releases/qt/> and
  <https://code.qt.io/>.

## Microsoft components

| File | Note |
|:--|:--|
| `vc_redist.x64.exe` | Microsoft Visual C++ Redistributable. The `Setup.exe` installer runs it with `/install /quiet /norestart` before Engauge starts; in the portable package it has to be run once by hand if the Visual C++ runtime is not installed yet. |
| `d3dcompiler_47.dll`, `dxcompiler.dll`, `dxil.dll` | Microsoft Direct3D shader compilers deployed by `windeployqt` as part of the Qt graphics stack. |

These files are redistributed under Microsoft's license terms for the Visual C++ Redistributable and
the Windows SDK; the license text cannot be redistributed here, see
<https://visualstudio.microsoft.com/license-terms/>. No Microsoft runtime DLL is bundled separately
from `vc_redist.x64.exe`.

## FFTW 3

- File: `libfftw3-3.dll`
- Version: 3.3.5
- Copyright: Matteo Frigo and Steven G. Johnson
- License: GNU General Public License version 2 or later (GPL-2.0-or-later)
- License text: `LICENSE` in the package (the GPL version 2 text), and
  <https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
- Source: <https://www.fftw.org/>

## Not included

PDF input (Poppler), JPEG 2000 input (OpenJPEG) and network features are not part of the Windows
packages: no Poppler, OpenJPEG, Qt Network or TLS library is shipped, and the build compiles the
network code out (`ENGAUGE_ENABLE_NETWORK=OFF`). Poppler is optional in the upstream source and can be
enabled in a build of your own (see `DEPENDENCIES.md`).

## Reporting

If a component is missing from this list, please open an issue — a complete notice is a release
requirement, not a formality.
