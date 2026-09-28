# Engauge Digitizer — Windows Fork

[English](#english) | [한국어](#한국어)

![CI](https://github.com/doha-230/engauge-digitizer/actions/workflows/ci-test.yml/badge.svg)
![Latest release](https://img.shields.io/github/v/release/doha-230/engauge-digitizer)
![License](https://img.shields.io/badge/license-GPL--2.0%2B-blue)

**Ready-to-use Windows 64-bit build of Engauge Digitizer** — a self-contained installer and a portable
zip for machines that have no network access, with opt-in extras such as one-click curve detection.

**[Download the latest release](../../releases/latest)** — pick the asset named
`Engauge-Digitizer-<version>-Windows-x64-Setup.exe` or `Engauge-Digitizer-<version>-Windows-x64-Portable.zip`
(both releases also publish `SHA256SUMS.txt`).

---

## English

Engauge Digitizer converts an image of a graph into numbers: import the picture, define the axes, and
digitize curves by hand, with Segment Fill, with Point Match, or with the **Auto Curve Detection** mode
of this fork.

### Fork chain and upstream

This repository is a fork of a fork:

1. **Mark Mitchell** wrote and maintained Engauge Digitizer; the original repository is gone.
2. [akhuettel/engauge-digitizer](https://github.com/akhuettel/engauge-digitizer) restored the source and
   keeps it building on modern Linux — that is the upstream, and this fork tracks it.
3. [Jinta0Li/engauge-digitizer](https://github.com/Jinta0Li/engauge-digitizer) started Windows packaging.
4. **This fork** continues that work: newer version, current Windows packages, and the opt-in features below.

Fixes and improvements that are not Windows specific are prepared as upstream pull requests
(duplicate-shortcut fix, regression-harness fixes, the missing Segment Fill menu entry, the help
build fix).

### What this repository provides

- **Ready-to-use Windows packages** — an installer and a portable zip, both self-contained (Qt, FFTW,
  the help file and all translations included), with published SHA256 checksums
- **A closed-network build** — the network code is compiled out (`ENGAUGE_ENABLE_NETWORK=OFF`), so the
  application only ever touches local files
- **Extra features, all opt-in** — a fresh installation behaves exactly like the upstream release
  until each option is turned on

### Quick start

1. Run the installer, or unzip the portable package and start `Engauge.exe`.
2. `File > Open` an image of a graph.
3. Click the four axis points and type their values (`Digitize > Axis Points`).
4. Pick a tool — `Segment Fill`, `Point Match`, or `Auto Curve Detection` — and click the curve.
5. `File > Export` the points as CSV, and run `Digitize > Quality Report...` to review them.

### Requirements

- Windows 10 or Windows 11, 64-bit. No Python, no Qt and no other runtime installation is needed.
- The portable package needs no installation and no administrator rights.
- The installer needs administrator rights; it adds Start Menu shortcuts and an uninstaller.
- Both packages ship Microsoft's Visual C++ Redistributable (`vc_redist.x64.exe`). The installer runs
  it automatically; with the portable package, run it once by hand if Windows reports a missing
  runtime.

### Download

Open the [latest release](../../releases/latest) and choose one of:

- **`Engauge-Digitizer-<version>-Windows-x64-Setup.exe`** — standard Windows installation with Start
  Menu shortcuts and uninstall support
- **`Engauge-Digitizer-<version>-Windows-x64-Portable.zip`** — extract and run `Engauge.exe`; no
  installation is required

The `SHA256SUMS.txt` file in the same release carries the checksums, and `OFFLINE_UPDATE.md` — it is
in the root of both packages, next to `LICENSE`, `README.md` and `THIRD-PARTY-NOTICES.md` — describes
the offline update and rollback procedure. These community binaries are not code-signed, so Windows
SmartScreen may ask for confirmation.

### Features added in this fork

| Feature | Where | What it does |
|---|---|---|
| **Auto Curve Detection** | `Digitize > Auto Curve Detection Tool` | Click any piece of a curve and points are created along every connected piece — no more filling each scan gap by hand. Two gates keep the chain on the curve: a maximum gap (3 px default) and a maximum turn (45° default), plus an optional "the curve is a function" assumption. A 5000 point limit guards against runaway detections. |
| **Quality report** | `Digitize > Quality Report...` | Lists points that deserve a second look: repeated or reversed x values, sudden slope changes (robust MAD), points that overlap another curve, and points outside the axes. Double click a row to jump to that point. Save as CSV. |
| **Band centering** | `Settings > Segments` | Moves filled points to the middle of the curve band (pixel run center, or darkness weighted center) instead of the traced path. The default keeps the traced path. |
| **Axis point review** | `Settings > General` | Warns before a coordinate system is defined from axis points that look wrong: duplicate coordinates, or x/y values that go backwards. |
| **Point origin display** | `Settings > General` | Dashed outline for points created by Segment Fill, Point Match or Auto Curve Detection, so automated and hand-placed points are distinguishable. |
| **Delete automated points** | `Edit > Delete Automated Points` | Removes every automated point in one undoable step, keeping hand-placed points. |
| **Auto-save and recovery** | `Settings > General` | Writes a recovery copy while the document is modified; the next start offers to recover it after an unexpected exit. |
| **Batch processing** | Command line | `Engauge -batchtemplate <template.dig> [-batchout <dir>] [-batchcontinue] <images...>` digitizes a whole folder from a template document. |
| **New From Template** | `File > New From Template...` | Digitize a new image with the axis points, filters and export format of a template document. |
| **Export presets** | `Settings > Export Format` | Save and restore the complete export format under a name. |
| **Themes and high DPI** | `Settings > General` | Light and Dark themes (user curve colors are never touched), crisp icons at 150% and 200% scaling. |
| **Compact zoom** | `Settings > General` | Replaces the twenty entry zoom menu with a combo plus fill. |
| **About and diagnostics** | `Help > About`, `Help > Copy Diagnostics` | Version, Qt runtime and build date; diagnostics to the clipboard or a file — nothing is sent anywhere. |
| **Korean UI** | `Help > Language` | Full Korean translation (and the other bundled languages) switchable at run time. |

See [help/forkfeatures.html](help/forkfeatures.html) — it is also in the built-in help index — for
the long descriptions.

### Closed-network notes

- Everything runs locally: no network access, no telemetry, no downloads, no update checks.
- Recovery files, reports and exports are written next to the documents or to the local application
  data directory.
- Updating is a manual, documented procedure: [docs/OFFLINE_UPDATE.md](docs/OFFLINE_UPDATE.md).

### Limitations

- Windows only — this repository ships no Linux or macOS binaries. Use the
  [upstream source](https://github.com/akhuettel/engauge-digitizer) for other platforms.
- The binaries are not code-signed: SmartScreen may require a confirmation, and no automatic update
  is offered.
- Auto Curve Detection stops at 5000 points per run; use it repeatedly on long curves.
- Everything is off by default: a fresh installation behaves like the upstream release until an
  option is enabled in `Settings`.

### Quality gates

- Every push: **26 command-line unit suites** (`ctest`), a static duplicate-shortcut check, product
  verification and portable deployment assembly — in two parallel Windows jobs with build caching
- Every release: silent install, launch and uninstall test, and SHA-256 checksums before the release
  is published
- The 83 GUI regression cases under `test/` are run locally with `src/build_and_run_all_gui_tests`

### Support

- Bugs and questions: use the [issue tracker](../../issues). Please include the Windows version, the
  release tag, and the output of `Help > Copy Diagnostics`.
- Upstream project (source, Linux): [akhuettel/engauge-digitizer](https://github.com/akhuettel/engauge-digitizer)

### Building from source (Windows)

See [BUILDING.md](BUILDING.md) and [DEPENDENCIES.md](DEPENDENCIES.md); [CMakePresets.json](CMakePresets.json)
carries the exact configuration used by CI. The build is automated by
[the CI workflow](../../actions/workflows/ci-test.yml) and releases are produced by
[the release workflow](../../actions/workflows/release-windows.yml); [docs/RELEASE.md](docs/RELEASE.md)
is the release checklist.

### Project links

- [Official project website](https://akhuettel.github.io/engauge-digitizer/)
- [Official upstream repository](https://github.com/akhuettel/engauge-digitizer)
- [Releases](../../releases)
- [Build workflows](../../actions)

### Citation

If you publish results obtained with Engauge Digitizer, please cite the original project, which is
archived on Zenodo: [DOI 10.5281/zenodo.597553](https://doi.org/10.5281/zenodo.597553) (this DOI
always resolves to the latest archived version).

### Attribution

The original application was created and maintained by Mark Mitchell. The upstream source was
restored and is maintained by Andreas K. Hüttel and other contributors. This fork retains the
upstream copyright and licensing notices, and ships the license texts of every bundled component —
see [docs/THIRD-PARTY-NOTICES.md](docs/THIRD-PARTY-NOTICES.md).

---

## 한국어

Engauge Digitizer는 그래프 이미지를 숫자로 바꾸는 도구입니다. 이미지를 불러오고, 축을 정의하고,
커브를 손으로 찍거나 Segment Fill·Point Match·이 포크의 **자동 곡선 검출** 모드로 디지타이즈합니다.

**네트워크가 없는 Windows 64비트 PC를 위한 바로 쓸 수 있는 빌드**입니다. 설치본과 포터블 zip을
제공하며, 한 번 클릭으로 곡선 전체를 찍는 자동 곡선 검출 같은 추가 기능은 모두 옵트인입니다.

**[최신 릴리스 다운로드](../../releases/latest)** — `Engauge-Digitizer-<버전>-Windows-x64-Setup.exe`
또는 `Engauge-Digitizer-<버전>-Windows-x64-Portable.zip` 파일을 고르세요(`SHA256SUMS.txt` 동봉).

### 포크 계보와 업스트림

이 저장소는 포크의 포크입니다.

1. **Mark Mitchell**이 Engauge Digitizer를 만들고 유지했고, 원래 저장소는 삭제되었습니다.
2. [akhuettel/engauge-digitizer](https://github.com/akhuettel/engauge-digitizer)가 소스를 복원해 최신
   Linux에서 빌드되도록 유지합니다 — 이쪽이 업스트림이며 이 포크가 따라갑니다.
3. [Jinta0Li/engauge-digitizer](https://github.com/Jinta0Li/engauge-digitizer)가 Windows 패키징을 시작했습니다.
4. **이 포크**는 그 작업을 이어갑니다: 더 새로운 버전, 현재의 Windows 패키지, 아래의 옵트인 기능들.

Windows 전용이 아닌 수정·개선은 업스트림 PR로 정리해 되돌려 보냅니다(단축키 중복 수정, 회귀 하네스 수정,
누락된 Segment Fill 메뉴 항목, 도움말 빌드 수정).

### 이 저장소가 제공하는 것

- **바로 쓸 수 있는 Windows 패키지** — 설치본과 포터블 zip. 둘 다 자체 완결형(Qt·FFTW·도움말·전체 번역 포함)이며 SHA256 체크섬을 공개합니다
- **폐쇄망(에어갭) 빌드** — 네트워크 코드를 빌드에서 제거(`ENGAUGE_ENABLE_NETWORK=OFF`)해 애플리케이션이 로컬 파일만 만집니다
- **추가 기능 전부 옵트인** — 설정을 켜기 전까지는 업스트림 릴리스와 동일하게 동작합니다

### 빠른 시작

1. 설치본을 실행하거나, 포터블 zip을 풀고 `Engauge.exe`를 실행합니다.
2. `File > Open`으로 그래프 이미지를 엽니다.
3. 축 점 네 개를 클릭하고 값을 입력합니다(`Digitize > Axis Points`).
4. 도구를 고르고 — `Segment Fill`, `Point Match`, `Auto Curve Detection` — 커브를 클릭합니다.
5. `File > Export`로 CSV로 내보내고, `Digitize > Quality Report...`로 점을 검토합니다.

### 요구 사항

- Windows 10 또는 11, 64비트. Python·Qt 등 별도 런타임 설치가 필요 없습니다.
- 포터블 패키지는 설치도 관리자 권한도 필요 없습니다.
- 설치본은 관리자 권한이 필요하며 시작 메뉴 단축키와 제거 프로그램을 만듭니다.
- 두 패키지 모두 Microsoft Visual C++ 재배포 패키지(`vc_redist.x64.exe`)를 포함합니다. 설치본은 자동으로
  실행하고, 포터블 패키지는 런타임이 없다는 메시지가 나오면 한 번 직접 실행하세요.

### 다운로드

[최신 릴리스](../../releases/latest)에서 다음 중 하나를 고르세요:

- **`Engauge-Digitizer-<버전>-Windows-x64-Setup.exe`** — 시작 메뉴 단축키와 제거 지원이 있는 표준 설치
- **`Engauge-Digitizer-<버전>-Windows-x64-Portable.zip`** — 압축을 풀고 `Engauge.exe` 실행, 설치 불필요

같은 릴리스의 `SHA256SUMS.txt`에 체크섬이 있고, `OFFLINE_UPDATE.md`(두 패키지 모두 루트에 `LICENSE`,
`README.md`, `THIRD-PARTY-NOTICES.md`와 함께 들어 있습니다)에 폐쇄망 업데이트·롤백 절차가 정리되어 있습니다. 커뮤니티 빌드는 코드 서명이 없어 Windows SmartScreen가
확인을 요청할 수 있습니다.

### 이 포크의 추가 기능

| 기능 | 위치 | 내용 |
|---|---|---|
| **자동 곡선 검출** | `Digitize > Auto Curve Detection Tool` | 커브 아무 곳이나 한 번 클릭하면 연결된 조각 전체를 따라 점이 생성 — 스캔 공백을 일일이 채울 필요가 없습니다. 두 관문(최대 갭 3픽셀, 최대 방향 전환 45°)이 사슬을 커브 위에 유지하고, 선택 가능한 "함수 가정"이 x 역행을 절단합니다. 5000점 상한이 폭주를 막습니다. |
| **품질 리포트** | `Digitize > Quality Report...` | 다시 볼 가치가 있는 점을 나열: x 반복·역전, 급격한 기울기 변화(robust MAD), 다른 커브와의 중복, 축 범위 밖. 행을 더블클릭하면 해당 점으로 이동. CSV 저장 지원. |
| **밴드 중심** | `Settings > Segments` | Segment Fill로 채운 점을 추적 경로 대신 커브 밴드의 중앙(픽셀 런 중심 또는 어두운 정도 가중 중심)으로 이동. 기본값은 추적 경로 유지. |
| **축 점 검토** | `Settings > General` | 좌표계 정의 전에 의심스러운 축 점(중복 좌표, x/y 역행)을 경고합니다. |
| **점 출처 표시** | `Settings > General` | Segment Fill·Point Match·자동 곡선 검출이 만든 점에 점선 외곽선 — 자동 점과 수동 점을 구별합니다. |
| **자동 점 삭제** | `Edit > Delete Automated Points` | 자동 점 전부를 Undo 가능한 한 번에 제거하고 수동 점은 보존합니다. |
| **자동 저장·복구** | `Settings > General` | 문서 수정 중 복구 사본을 기록하고, 비정상 종료 후 다음 시작 때 복구를 제안합니다. |
| **배치 처리** | 명령줄 | `Engauge -batchtemplate <template.dig> [-batchout <dir>] [-batchcontinue] <이미지들...>` — 템플릿 문서로 폴더 전체를 디지타이즈합니다. |
| **템플릿으로 새 문서** | `File > New From Template...` | 템플릿 문서의 축 점·필터·내보내기 형식으로 새 이미지를 디지타이즈합니다. |
| **내보내기 프리셋** | `Settings > Export Format` | 내보내기 형식 전체를 이름으로 저장·복원합니다. |
| **테마·고해상도** | `Settings > General` | Light·Dark 테마(사용자 커브 색은 불변), 150%·200% 배율에서 선명한 아이콘. |
| **컴팩트 줌** | `Settings > General` | 20개 항목 줌 메뉴를 콤보+Fill로 교체합니다. |
| **정보·진단** | `Help > About`, `Help > Copy Diagnostics` | 버전·Qt 런타임·빌드 날짜, 클립보드/파일 진단 — 어디로도 전송하지 않습니다. |
| **한국어 UI** | `Help > Language` | 완전한 한국어 번역(타 언어 포함)을 실행 중에 전환합니다. |

자세한 설명은 [help/forkfeatures.html](help/forkfeatures.html)(내장 도움말 인덱스에도 포함)을 참고하세요.

### 폐쇄망 관련

- 전부 로컬 동작: 네트워크 접속·텔레메트리·다운로드·업데이트 확인 없음
- 복구 파일·리포트·내보내기는 문서 옆 또는 로컬 앱 데이터 디렉터리에 기록
- 업데이트는 수동 절차: [docs/OFFLINE_UPDATE.md](docs/OFFLINE_UPDATE.md)

### 제한 사항

- Windows 전용입니다 — Linux·macOS 바이너리는 이 저장소에서 제공하지 않습니다. 다른 플랫폼은
  [업스트림 소스](https://github.com/akhuettel/engauge-digitizer)를 사용하세요.
- 코드 서명이 없어 SmartScreen 확인이 뜰 수 있고, 자동 업데이트 기능은 없습니다.
- 자동 곡선 검출은 한 번에 5000점에서 멈춥니다. 긴 곡선은 여러 번 나눠 찍으세요.
- 모든 추가 기능은 기본 꺼짐입니다: `Settings`에서 켜기 전까지는 업스트림 릴리스와 동일하게 동작합니다.

### 품질 게이트

- 모든 푸시: **커맨드라인 유닛 스위트 26종**(`ctest`)·단축키 충돌 정적 검사·산출물 검증·포터블 조립 — 빌드 캐시가 있는 Windows 병렬 2잡
- 모든 릴리스: 무인 설치·실행·제거 테스트와 SHA-256 체크섬을 통과한 뒤 공개
- `test/`의 GUI 회귀 케이스 83종은 `src/build_and_run_all_gui_tests`로 로컬에서 실행합니다

### 지원

- 버그·질문: [이슈 트래커](../../issues)를 사용하세요. Windows 버전, 릴리스 태그, `Help > Copy Diagnostics`
  결과를 함께 적어주세요.
- 업스트림 프로젝트(소스, Linux): [akhuettel/engauge-digitizer](https://github.com/akhuettel/engauge-digitizer)

### 소스에서 빌드(Windows)

[BUILDING.md](BUILDING.md)와 [DEPENDENCIES.md](DEPENDENCIES.md)를 보세요. [CMakePresets.json](CMakePresets.json)에
CI가 쓰는 설정이 그대로 들어 있습니다. 빌드는 [CI 워크플로](../../actions/workflows/ci-test.yml)가,
릴리스는 [릴리스 워크플로](../../actions/workflows/release-windows.yml)가 자동화하며,
[docs/RELEASE.md](docs/RELEASE.md)가 릴리스 체크리스트입니다.

### 프로젝트 링크

- [공식 프로젝트 웹사이트](https://akhuettel.github.io/engauge-digitizer/)
- [공식 업스트림 저장소](https://github.com/akhuettel/engauge-digitizer)
- [릴리스](../../releases)
- [빌드 워크플로](../../actions)

### 인용

Engauge Digitizer로 얻은 결과를 발표한다면 원 프로젝트를 인용해 주세요. Zenodo에 아카이브되어 있습니다:
[DOI 10.5281/zenodo.597553](https://doi.org/10.5281/zenodo.597553) (이 DOI는 항상 최신 아카이브 버전으로 연결됩니다).

### 귀속

원래 애플리케이션은 Mark Mitchell이 만들고 유지했습니다. 업스트림 소스는 Andreas K. Hüttel과 기여자들이
복원·유지합니다. 이 포크는 업스트림의 저작권·라이선스 표기를 유지하고, 포함된 모든 구성 요소의 라이선스
전문을 함께 배포합니다 — [docs/THIRD-PARTY-NOTICES.md](docs/THIRD-PARTY-NOTICES.md).

---

## License / 라이선스

Engauge Digitizer is distributed under the GNU General Public License version 2 or, at your option,
any later version. See [LICENSE](LICENSE). Third-party components bundled with the Windows packages
are listed in [docs/THIRD-PARTY-NOTICES.md](docs/THIRD-PARTY-NOTICES.md).
Engauge Digitizer는 GNU General Public License 2판 또는 (선택에 따라) 그 이상의 라이선스로 배포됩니다.
[LICENSE](LICENSE)를 참고하세요. Windows 패키지에 포함된 서드파티 구성 요소는
[docs/THIRD-PARTY-NOTICES.md](docs/THIRD-PARTY-NOTICES.md)에 정리되어 있습니다.
