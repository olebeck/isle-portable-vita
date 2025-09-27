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


FetchContent_Declare(
  iniparser_paf
  GIT_REPOSITORY "https://gitlab.com/iniparser/iniparser.git"
  GIT_TAG "v4.2.6"
  UPDATE_DISCONNECTED TRUE
  EXCLUDE_FROM_ALL
  PATCH_COMMAND patch -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/iniparser_paf.patch
)
block()
  set(BUILD_DOCS off)
  set(BUILD_SHARED_LIBS off)
  FetchContent_MakeAvailable(iniparser_paf)
endblock()

add_executable(isle-config
  src/paf_runtime.cpp  
  src/app.cpp
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
  SceFios2_stub
  SceLibc_stub

  iniparser_paf-static
)

vita_create_self(isle-config.self isle-config
  CONFIG exports.yml
  UNSAFE
  STRIPPED
  REL_OPTIMIZE
)


## isle downloader

add_executable(isle-downloader
  downloader/paf_runtime.cpp  

  downloader/main.cpp
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
  downloader/DeleteFilesJob.cpp
)

set_target_properties(isle-downloader PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(isle-downloader PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti -fno-exceptions>
  -Wl,-q -Wall -fshort-wchar -Wno-unused-function -Wno-sign-compare -fno-use-cxa-atexit
)

target_link_options(isle-downloader PRIVATE
  -nostdlib
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
  SceSsl_stub
  # paf
  ScePafToplevel_stub
  ScePafResource_stub
  ScePafWidget_stub
  ScePafCommon_stub
  ScePafStdc_stub
  ScePafThread_stub
  ScePafMisc_stub
  SceCommonGuiDialog_stub
  # other
  iniparser_paf-static
  gcc
)

vita_create_self(isle-downloader.self isle-downloader
  CONFIG exports.yml
  UNSAFE
  STRIPPED
  REL_OPTIMIZE
)

include(${scepaf_external_SOURCE_DIR}/rco.cmake)
make_rco(cxml/config_plugin.xml config_plugin.rco)
make_rco(cxml/downloader_plugin.xml downloader_plugin.rco)
