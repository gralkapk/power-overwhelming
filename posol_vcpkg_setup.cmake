set(POSOL_VCPKG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg" CACHE PATH "Path to vcpkg.")
mark_as_advanced(FORCE POSOL_VCPKG_DIR)

if (NOT IS_DIRECTORY "${POSOL_VCPKG_DIR}")
  set(POSOL_VCPKG_DIR "${CMAKE_CURRENT_BINARY_DIR}/vcpkg" CACHE PATH "Path to vcpkg." FORCE)
endif ()

include(FetchContent)
mark_as_advanced(FORCE
  FETCHCONTENT_BASE_DIR
  FETCHCONTENT_FULLY_DISCONNECTED
  FETCHCONTENT_QUIET
  FETCHCONTENT_UPDATES_DISCONNECTED)

# Require git for download
find_package(Git REQUIRED)

FetchContent_Declare(vcpkg-download
  GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
  GIT_TAG "2023.07.21"
  #GIT_SHALLOW TRUE # Not supported by vcpkg
  SOURCE_DIR ${POSOL_VCPKG_DIR})
FetchContent_GetProperties(vcpkg-download)
if (NOT vcpkg-download_POPULATED)
  message(STATUS "Fetch vcpkg ...")
  FetchContent_Populate(vcpkg-download)
  mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_VCPKG-DOWNLOAD
    FETCHCONTENT_UPDATES_DISCONNECTED_VCPKG-DOWNLOAD)
endif ()

# vcpkg config
#set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vcpkg_ports")
#set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vcpkg_triplets") # Disable compiler tracking on Windows.
set(VCPKG_BOOTSTRAP_OPTIONS "-disableMetrics")
set(VCPKG_INSTALL_OPTIONS "--clean-after-build" "--no-print-usage")
set(CMAKE_TOOLCHAIN_FILE "${POSOL_VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(ENV{VCPKG_FORCE_DOWNLOADED_BINARIES} ON) # Always download tools (i.e. CMake) to have consistent versions on all systems.
