# Engauge Digitizer — Windows Fork

[English](#english) | [한국어](#한국어)

---

## English

Engauge Digitizer converts an image of a graph into numbers: import the picture, define the axes,
and digitize curves by hand, with Segment Fill, with Point Match, or with the **Auto Curve Detection**
mode of this fork.

The upstream project does not ship recent Windows binaries, and the machines this fork is built for
have no network at all. So this repository provides:

- **Ready-to-use Windows packages** — an installer and a portable zip, both self-contained (Qt, FFTW,
  the help file and all translations included), with published SHA256 checksums
- **A closed-network build** — the network code is compiled out (`ENGAUGE_ENABLE_NETWORK=OFF`), so the
  application only ever touches local files
- **Extra features, all opt-in** — a fresh installation behaves exactly like the upstream release
  until each option is turned on

### Download

Open the [latest release](../../releases/latest) and choose one of:

- **Setup.exe** — standard Windows installation with Start Menu shortcuts and uninstall support
- **Portable.zip** — extract and run `Engauge.exe`; no installation is required

The SHA256 checksums are published with each release, and `docs/OFFLINE_UPDATE.md` (also inside the
packages) describes the offline update and rollback procedure. These community binaries are not
code-signed, so Windows SmartScreen may ask for confirmation.

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

### Quality gates

- 27 unit test suites (plus the qmake build, product checks and portable assembly) run on every push
  in a Windows CI with parallel build and test jobs and build caching
- Static shortcut conflict check on every push
- Every release asset is checksummed and install-tested before it is published

### Project links

- [Official project website](https://akhuettel.github.io/engauge-digitizer/)
- [Official upstream repository](https://github.com/akhuettel/engauge-digitizer)
- [Releases](../../releases)
- [Build workflows](../../actions)

The original application was created and maintained by Mark Mitchell. The upstream source was
restored and is maintained by Andreas K. Hüttel and other contributors. This fork retains the
upstream copyright and licensing notices.

---

## 한국어

Engauge Digitizer는 그래프 이미지를 숫자로 바꾸는 도구입니다. 이미지를 불러오고, 축을 정의하고,
커브를 손으로 찍거나 Segment Fill·Point Match·이 포크의 **자동 곡선 검출** 모드로 디지타이즈합니다.

업스트림 프로젝트는 최신 버전의 Windows 실행 파일을 제공하지 않고, 이 포크가 대상으로 하는 컴퓨터는
네트워크가 아예 없습니다. 그래서 이 저장소는 다음을 제공합니다:

- **바로 쓸 수 있는 Windows 패키지** — 설치본과 포터블 zip. 둘 다 자체 완결형(Qt·FFTW·도움말·전체 번역 포함)이며 SHA256 체크섬을 공개합니다
- **폐쇄망(에어갭) 빌드** — 네트워크 코드를 빌드에서 제거(`ENGAUGE_ENABLE_NETWORK=OFF`)해 애플리케이션이 로컬 파일만 만집니다
- **추가 기능 전부 옵트인** — 설정을 켜기 전까지는 업스트림 릴리스와 동일하게 동작합니다

### 다운로드

[최신 릴리스](../../releases/latest)에서 다음 중 하나를 고르세요:

- **Setup.exe** — 시작 메뉴 단축키와 제거 지원이 있는 표준 설치
- **Portable.zip** — 압축을 풀고 `Engauge.exe` 실행, 설치 불필요

각 릴리스에 SHA256 체크섬이 게시되고, `docs/OFFLINE_UPDATE.md`(패키지에도 동봉)에 폐쇄망 업데이트·롤백 절차가
정리되어 있습니다. 커뮤니티 빌드는 코드 서명이 없어 Windows SmartScreen가 확인을 요청할 수 있습니다.

### 이 포크의 추가 기능

| 기능 | 위치 | 내용 |
|---|---|---|
| **자동 곡선 검출** | `Digitize > Auto Curve Detection Tool` | 커브 아무 곳이나 한 번 클릭하면 연결된 조각 전체를 따라 점이 생성 — 스캔 공백을 일일이 채울 필요가 없습니다. 두 관문(최대 갭 3픽셀, 최대 방향 전환 45°)이 사슬을 커브 위에 유지하고, 선택 가능한 "함수 가정"이 x 역행을 절단합니다. 5000점 상한이 폭주를 막습니다. |
| **품질 리포트** | `Digitize > Quality Report...` | 다시 볼 가치가 있는 점을 나열: x 반복·역전, 급격한 기울기 변화(robust MAD), 다른 커브와의 중복, 축 범위 밖. 행을 더블클릭하면 해당 점으로 이동. CSV 저장 지원. |
| **밴드 중심** | `Settings > Segments` | 채점된 점을 추적 경로 대신 커브 밴드의 중앙(픽셀 런 중심 또는 어두운 정도 가중 중심)으로 이동. 기본값은 추적 경로 유지. |
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

### 품질 게이트

- 모든 푸시마다 **유닛 테스트 27종**(qmake 빌드·산출물 검사·포터블 조립 포함)을 병렬 빌드·테스트 잡과 빌드 캐시가 있는 Windows CI에서 실행
- 모든 푸시에 단축키 충돌 정적 검사
- 릴리스 에셋은 체크섬 + 설치 테스트 후 공개

### 프로젝트 링크

- [공식 프로젝트 웹사이트](https://akhuettel.github.io/engauge-digitizer/)
- [공식 업스트림 저장소](https://github.com/akhuettel/engauge-digitizer)
- [릴리스](../../releases)
- [빌드 워크플로](../../actions)

원래 애플리케이션은 Mark Mitchell이 만들고 유지했습니다. 업스트림 소스는 Andreas K. Hüttel과 기여자들이
복원·유지합니다. 이 포크는 업스트림의 저작권·라이선스 표기를 유지합니다.

---

## License / 라이선스

Engauge Digitizer is distributed under the GNU General Public License version 2 or, at your option,
any later version. See [LICENSE](LICENSE).
Engauge Digitizer는 GNU General Public License 2판 또는 (선택에 따라) 그 이상의 라이선스로 배포됩니다.
[LICENSE](LICENSE)를 참고하세요.
