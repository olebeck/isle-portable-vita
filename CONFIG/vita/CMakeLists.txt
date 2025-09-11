cmake_minimum_required(VERSION 3.25...4.0 FATAL_ERROR)

project(isle-config LANGUAGES CXX C VERSION 0.1)

include("${VITASDK}/share/vita.cmake" REQUIRED)

include(FetchContent)
FetchContent_Declare(
  ScePaf_External
  URL https://github.com/olebeck/ScePaf/releases/download/v21/ScePaf-1.0.0.zip
  URL_HASH SHA256=357b914a5c99ea17afe0edc8787a05cbf2ecce2f1d73bb9be69f371a294b8943
  UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(ScePaf_External)


## iniparser modified to work with PAF

add_library(iniparser_paf
  iniparser_paf/src/dictionary.c
  iniparser_paf/src/iniparser.c
)

target_compile_options(iniparser_paf PRIVATE -O3)

target_link_libraries(iniparser_paf
  ScePafStdc_stub
  SceLibKernel_stub
  SceLibc_stub
  SceFios2_stub
)

target_include_directories(iniparser_paf PUBLIC
  iniparser_paf/src/
)

target_compile_options(iniparser_paf PRIVATE
  -Wl,-q -Wall -fno-builtin -fshort-wchar -Wno-unused-function -Wno-sign-compare
)


## isle config

add_executable(isle-config
  src/app.cpp
  src/main.cpp
)

set_target_properties(isle-config PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(isle-config PRIVATE
  -fno-rtti -fno-exceptions -Wl,-q -Wall -fno-builtin -fshort-wchar -Wno-unused-function -Wno-sign-compare -fno-use-cxa-atexit
)

target_link_options(isle-config PRIVATE
  -nostartfiles -nostdlib
)

target_link_libraries(isle-config PRIVATE
  # sce
  SceAppMgr_stub
  SceLibKernel_stub
  SceSysmodule_stub
  # paf
  ScePafToplevel_stub
  ScePafResource_stub
  ScePafWidget_stub
  ScePafCommon_stub
  ScePafStdc_stub
  SceAppSettings_stub
  # other
  iniparser_paf
)

vita_create_self(isle-config.self isle-config
  CONFIG exports.yml
  UNSAFE
  STRIPPED
  REL_OPTIMIZE
)


## isle downloader

add_executable(isle-downloader
  downloader/paf_main.cpp
  downloader/paf_loader.cpp  

  downloader/App.cpp
  downloader/GameFiles.cpp
  downloader/Page.cpp
  downloader/Dialog.cpp
  downloader/util.cpp

  downloader/StartupPage.cpp
  downloader/DownloadPage.cpp
  downloader/VerifyPage.cpp
  
  downloader/DownloadJob.cpp
  downloader/VerifyJob.cpp

  downloader/sha256.c
)

set_target_properties(isle-downloader PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(isle-downloader PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti -fno-exceptions>
  -Wl,-q -Wall -fno-builtin -fshort-wchar -Wno-unused-function -Wno-sign-compare -fno-use-cxa-atexit
)

target_link_options(isle-downloader PRIVATE
  -nostartfiles -nostdlib
)

target_link_libraries(isle-downloader PRIVATE
  # sce
  SceAppMgr_stub
  SceLibKernel_stub
  SceSysmodule_stub
  SceIofilemgr_stub
  SceHttp_stub
  SceNet_stub
  SceNetCtl_stub
  # paf
  ScePafToplevel_stub
  ScePafResource_stub
  ScePafWidget_stub
  ScePafCommon_stub
  ScePafStdc_stub
  ScePafThread_stub
  ScePafMisc_stub
  # other
  iniparser_paf
  gcc
)

vita_create_self(isle-downloader.self isle-downloader
  CONFIG exports.yml
  UNSAFE
  STRIPPED
  REL_OPTIMIZE
)


## compile rcos

include(${scepaf_external_SOURCE_DIR}/rco.cmake)
make_rco(cxml/config_plugin.xml config_plugin.rco)
make_rco(cxml/downloader_plugin.xml downloader_plugin.rco)
