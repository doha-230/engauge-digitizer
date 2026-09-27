cmake_minimum_required(VERSION 3.21.1)

project(EngaugeDigitizer VERSION 12.11.2 LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(WIN32)
  set(_engauge_null_logger_default ON)
else()
  set(_engauge_null_logger_default OFF)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(_engauge_network_default ON)
else()
  set(_engauge_network_default OFF)
endif()

option(ENGAUGE_USE_NULL_LOGGER "Build the bundled no-op log4cpp implementation" ${_engauge_null_logger_default})
option(ENGAUGE_ENABLE_NETWORK "Enable error-report networking (Linux default only)" ${_engauge_network_default})

set(_engauge_qt_components Core Gui PrintSupport Widgets Xml LinguistTools)
if(NOT APPLE)
  list(APPEND _engauge_qt_components Help)
endif()
if(ENGAUGE_ENABLE_NETWORK)
  list(APPEND _engauge_qt_components Network)
endif()
if(BUILD_TESTING)
  list(APPEND _engauge_qt_components Test)
endif()

find_package(Qt6 REQUIRED COMPONENTS ${_engauge_qt_components})
find_package(FFTW3 REQUIRED)

if(NOT APPLE)
  find_program(ENGAUGE_QHELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator qhelpgenerator6
    HINTS
      "${QT_HOST_PATH}/bin"
      "${Qt6_DIR}/../../../bin"
    PATHS
      "/usr/lib/qt6/bin"
      "/usr/lib/qt6/libexec"
      "/usr/lib/x86_64-linux-gnu/qt6/libexec"
      "/usr/libexec/qt6"
    REQUIRED
  )
endif()

file(GLOB_RECURSE engauge_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE engauge_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

# Optional modules remain owned by qmake during this migration. Test sources
# are added to separate executables below, never to the application core.
list(FILTER engauge_sources EXCLUDE REGEX "/(Jpeg2000|Test|log4cpp_null)/")
list(FILTER engauge_headers EXCLUDE REGEX "/(Jpeg2000|Test|log4cpp_null)/")

# An in-source qmake build leaves generated code in dot directories such as
# src/.moc and src/.objs. Those must never be compiled here: AUTOMOC generates its
# own copies of the same meta objects, and compiling both fails at link time with
# "multiple definition of <class>::staticMetaObject". This is what happens when
# qmake and CMake builds share a working tree, as the CI job does.
list(FILTER engauge_sources EXCLUDE REGEX "/\\.[^/]+/")
list(FILTER engauge_headers EXCLUDE REGEX "/\\.[^/]+/")

set(_engauge_pdf_sources
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Dlg/DlgImportCroppingPdf.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Import/ImportCroppingUtilPdf.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Pdf/Pdf.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Pdf/PdfCropping.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Pdf/PdfFrameHandle.cpp"
)
set(_engauge_pdf_headers
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Dlg/DlgImportCroppingPdf.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Import/ImportCroppingUtilPdf.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Pdf/Pdf.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Pdf/PdfCropping.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Pdf/PdfFrameHandle.h"
)
list(REMOVE_ITEM engauge_sources ${_engauge_pdf_sources})
list(REMOVE_ITEM engauge_headers ${_engauge_pdf_headers})

# This incomplete legacy implementation is intentionally absent from engauge.pro.
list(REMOVE_ITEM engauge_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/Import/ImportCropping.cpp")

if(NOT ENGAUGE_ENABLE_NETWORK)
  list(REMOVE_ITEM engauge_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/Network/NetworkClient.cpp")
  list(REMOVE_ITEM engauge_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/Network/NetworkClient.h")
endif()

if(APPLE)
  list(REMOVE_ITEM engauge_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Help/HelpBrowser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Help/HelpWindow.cpp"
  )
  list(REMOVE_ITEM engauge_headers
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Help/HelpBrowser.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Help/HelpWindow.h"
  )
endif()

set(_engauge_main_source "${CMAKE_CURRENT_SOURCE_DIR}/src/main/main.cpp")
list(REMOVE_ITEM engauge_sources "${_engauge_main_source}")

add_library(EngaugeCore OBJECT
  ${engauge_sources}
  ${engauge_headers}
  src/engauge.qrc
)

add_executable(Engauge WIN32 MACOSX_BUNDLE "${_engauge_main_source}")
target_link_libraries(Engauge PRIVATE EngaugeCore)

file(GLOB _engauge_source_children LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
set(_engauge_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/src")
foreach(_engauge_child IN LISTS _engauge_source_children)
  if(IS_DIRECTORY "${_engauge_child}")
    list(APPEND _engauge_include_dirs "${_engauge_child}")
  endif()
endforeach()
target_include_directories(EngaugeCore PUBLIC ${_engauge_include_dirs})

target_link_libraries(EngaugeCore PUBLIC
  FFTW3::fftw3
  Qt6::Core
  Qt6::Gui
  Qt6::PrintSupport
  Qt6::Widgets
  Qt6::Xml
)

if(NOT APPLE)
  target_link_libraries(EngaugeCore PUBLIC Qt6::Help)
endif()

if(ENGAUGE_ENABLE_NETWORK)
  target_compile_definitions(EngaugeCore PUBLIC NETWORKING)
  target_link_libraries(EngaugeCore PUBLIC Qt6::Network)
endif()

if(ENGAUGE_USE_NULL_LOGGER)
  file(GLOB _engauge_null_logger_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/log4cpp_null/src/*.cpp"
  )
  target_sources(EngaugeCore PRIVATE ${_engauge_null_logger_sources})
  target_include_directories(EngaugeCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/log4cpp_null/include")
else()
  find_package(Log4cpp REQUIRED)
  target_link_libraries(EngaugeCore PUBLIC Log4cpp::log4cpp)
endif()

target_compile_definitions(EngaugeCore PUBLIC
  $<$<CONFIG:Release>:QT_NO_DEBUG>
)

if(MSVC)
  target_compile_options(EngaugeCore PRIVATE /F32000000)
  target_compile_options(Engauge PRIVATE /F32000000)
endif()

if(WIN32)
  set(ENGAUGE_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/img/digitizer.ico")
  set(ENGAUGE_WINDOWS_VERSION_TWEAK "${PROJECT_VERSION_TWEAK}")
  if(ENGAUGE_WINDOWS_VERSION_TWEAK STREQUAL "")
    set(ENGAUGE_WINDOWS_VERSION_TWEAK 0)
  endif()
  configure_file(cmake/Engauge.rc.in "${CMAKE_CURRENT_BINARY_DIR}/Engauge.rc" @ONLY)
  target_sources(Engauge PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/Engauge.rc")
endif()

set(engauge_translation_sources
  translations/engauge_ar.ts
  translations/engauge_cs.ts
  translations/engauge_de.ts
  translations/engauge_en.ts
  translations/engauge_es.ts
  translations/engauge_fa_IR.ts
  translations/engauge_fr.ts
  translations/engauge_hi.ts
  translations/engauge_it.ts
  translations/engauge_ja.ts
  translations/engauge_kk.ts
  translations/engauge_ko.ts
  translations/engauge_nb.ts
  translations/engauge_nl.ts
  translations/engauge_pt.ts
  translations/engauge_ru.ts
  translations/engauge_zh_CN.ts
  translations/engauge_zh_TW.ts
)
list(TRANSFORM engauge_translation_sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set_source_files_properties(${engauge_translation_sources} PROPERTIES
  OUTPUT_LOCATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/translations"
)
if(Qt6_VERSION VERSION_LESS 6.7)
  qt_add_lrelease(Engauge
    TS_FILES ${engauge_translation_sources}
    QM_FILES_OUTPUT_VARIABLE engauge_translation_binaries
  )
else()
  qt_add_lrelease(
    TS_FILES ${engauge_translation_sources}
    QM_FILES_OUTPUT_VARIABLE engauge_translation_binaries
    LRELEASE_TARGET engauge_translations
  )
  add_dependencies(Engauge engauge_translations)
endif()

if(NOT APPLE)
  set(_engauge_documentation_output_directory
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/documentation"
  )
  set(_engauge_help_qch
    "${_engauge_documentation_output_directory}/engauge.qch"
  )
  set(_engauge_help_qhc
    "${_engauge_documentation_output_directory}/engauge.qhc"
  )
  file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/help/engauge.qhp"
    ENGAUGE_HELP_PROJECT
  )
  file(TO_CMAKE_PATH "${_engauge_help_qch}" ENGAUGE_HELP_QCH)
  file(GLOB_RECURSE _engauge_help_inputs CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/help/*.css"
    "${CMAKE_CURRENT_SOURCE_DIR}/help/*.html"
    "${CMAKE_CURRENT_SOURCE_DIR}/help/*.jpg"
    "${CMAKE_CURRENT_SOURCE_DIR}/help/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/help/*.qhp"
  )
  configure_file(
    cmake/EngaugeHelp.qhcp.in
    "${CMAKE_CURRENT_BINARY_DIR}/EngaugeHelp.qhcp"
    @ONLY
  )
  add_custom_command(
    OUTPUT "${_engauge_help_qch}" "${_engauge_help_qhc}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory
      "${_engauge_documentation_output_directory}"
    COMMAND "${ENGAUGE_QHELPGENERATOR_EXECUTABLE}"
      "${CMAKE_CURRENT_BINARY_DIR}/EngaugeHelp.qhcp"
      -o "${_engauge_help_qhc}"
    DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/EngaugeHelp.qhcp"
      ${_engauge_help_inputs}
    COMMENT "Generating Engauge help collection"
    VERBATIM
  )
  add_custom_target(EngaugeHelp ALL
    DEPENDS "${_engauge_help_qch}" "${_engauge_help_qhc}"
  )
  add_dependencies(Engauge EngaugeHelp)
endif()

if(WIN32 AND FFTW3_RUNTIME_LIBRARY)
  add_custom_command(TARGET Engauge POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      "${FFTW3_RUNTIME_LIBRARY}"
      "$<TARGET_FILE_DIR:Engauge>"
    VERBATIM
  )
endif()

if(WIN32)
  set(_engauge_runtime_install_directory ".")
  set(_engauge_translation_install_directory "translations")
  set(_engauge_documentation_install_directory "documentation")
  set(_engauge_license_install_directory ".")
else()
  set(_engauge_runtime_install_directory "${CMAKE_INSTALL_BINDIR}")
  set(_engauge_translation_install_directory
    "${CMAKE_INSTALL_BINDIR}/translations"
  )
  set(_engauge_documentation_install_directory
    "${CMAKE_INSTALL_DATAROOTDIR}/doc/engauge-digitizer"
  )
  set(_engauge_license_install_directory
    "${CMAKE_INSTALL_DATAROOTDIR}/doc/engauge-digitizer"
  )
endif()

install(TARGETS Engauge
  RUNTIME DESTINATION "${_engauge_runtime_install_directory}"
  BUNDLE DESTINATION "."
)
install(FILES ${engauge_translation_binaries}
  DESTINATION "${_engauge_translation_install_directory}"
)
if(NOT APPLE)
  install(FILES "${_engauge_help_qch}" "${_engauge_help_qhc}"
    DESTINATION "${_engauge_documentation_install_directory}"
  )
endif()
if(WIN32 AND FFTW3_RUNTIME_LIBRARY)
  install(FILES "${FFTW3_RUNTIME_LIBRARY}"
    DESTINATION "${_engauge_runtime_install_directory}"
  )
endif()
install(FILES LICENSE README.md
  DESTINATION "${_engauge_license_install_directory}"
)

if(BUILD_TESTING)
  set(_engauge_unit_tests
    TestAxisPointsValidator
    TestAutosaveRecovery
    TestCentipedeEndpoints
    TestCorrelation
    TestCrc32
    TestCurveProperties
    TestExport
    TestExportAlign
    TestFitting
    TestFormats
    TestGraphCoords
    TestGridLineLimiter
    TestGuidelines
    TestImportCoordinateSetup
    TestMatrix
    TestPointOrigin
    TestProjectedPoint
    TestQualityReport
    TestSegmentCenter
    TestSegmentChain
    TestSpline
    TestSplineDrawer
    TestTransformation
    TestValidators
    TestZoomTransition
  )

  foreach(_engauge_test IN LISTS _engauge_unit_tests)
    add_executable(${_engauge_test}
      "src/Test/${_engauge_test}.cpp"
      "src/Test/${_engauge_test}.h"
    )
    target_link_libraries(${_engauge_test} PRIVATE EngaugeCore Qt6::Test)
    if(MSVC)
      target_compile_options(${_engauge_test} PRIVATE /F32000000)
    endif()
    add_test(NAME "unit.${_engauge_test}" COMMAND ${_engauge_test} -o -,txt)
    set_tests_properties("unit.${_engauge_test}" PROPERTIES
      LABELS unit
      RUN_SERIAL TRUE
      TIMEOUT 120
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
  endforeach()
endif()

message(STATUS "Engauge null logger: ${ENGAUGE_USE_NULL_LOGGER}")
message(STATUS "Engauge networking:  ${ENGAUGE_ENABLE_NETWORK}")
message(STATUS "Engauge unit tests:  ${BUILD_TESTING}")
message(STATUS "Engauge help tool:   ${ENGAUGE_QHELPGENERATOR_EXECUTABLE}")
message(STATUS "Engauge PDF:         qmake only in this migration stage")
message(STATUS "Engauge JPEG2000:    qmake only in this migration stage")
