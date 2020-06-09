cmake_minimum_required(VERSION 3.16)

project(
  libmueb
  VERSION 2.0
  DESCRIPTION "Network library for Schönherz Mátrix packet sending"
  HOMEPAGE_URL "kszk.bme.hu"
  LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

find_package(
  Qt5
  COMPONENTS Core Gui Network
  REQUIRED)

add_compile_definitions(LIBMUEB_LIBRARY)
if(BUILD_SHARED_LIBS)
  add_compile_definitions(LIBMUEB_AS_DLL)
endif()

add_library(
  mueb
  src/muebtransmitter.cpp
  src/muebreceiver.cpp
  include/libmueb/mueb.h
  include/libmueb/libmueb_global.h
  include/libmueb/muebtransmitter.h
  include/libmueb/muebreceiver.h
  include/libmueb/libmuebconfig.h)

add_library(
  muebtransmitter
  src/muebtransmitter.cpp include/libmueb/libmueb_global.h
  include/libmueb/muebtransmitter.h include/libmueb/libmuebconfig.h)

add_library(
  muebreceiver src/muebreceiver.cpp include/libmueb/libmueb_global.h
               include/libmueb/muebreceiver.h include/libmueb/libmuebconfig.h)

add_library(libmueb::mueb ALIAS mueb)
add_library(libmueb::muebtransmitter ALIAS muebtransmitter)
add_library(libmueb::muebreceiver ALIAS muebreceiver)

target_include_directories(
  mueb
  PUBLIC $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}>)

target_include_directories(
  muebtransmitter
  PUBLIC $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}>)

target_include_directories(
  muebreceiver
  PUBLIC $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}>)

target_link_libraries(
  mueb
  PUBLIC Qt5::Core Qt5::Gui
  PRIVATE Qt5::Network)
target_link_libraries(
  muebtransmitter
  PUBLIC Qt5::Core Qt5::Gui
  PRIVATE Qt5::Network)
target_link_libraries(
  muebreceiver
  PUBLIC Qt5::Core Qt5::Gui
  PRIVATE Qt5::Network)

set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(version_config "${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${version_config}"
                                 COMPATIBILITY SameMajorVersion)
configure_package_config_file("cmake/Config.cmake.in" "${project_config}"
                              INSTALL_DESTINATION "${config_install_dir}")

install(
  TARGETS mueb
  EXPORT "${TARGETS_EXPORT_NAME}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  INCLUDES
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")

install(
  TARGETS muebreceiver
  EXPORT "${TARGETS_EXPORT_NAME}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  INCLUDES
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")

install(
  TARGETS muebtransmitter
  EXPORT "${TARGETS_EXPORT_NAME}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  INCLUDES
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")

install(DIRECTORY "include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(FILES "${project_config}" "${version_config}"
        DESTINATION "${config_install_dir}")

install(
  EXPORT "${TARGETS_EXPORT_NAME}"
  NAMESPACE "${namespace}"
  DESTINATION "${config_install_dir}")

export(
  EXPORT ${TARGETS_EXPORT_NAME}
  FILE "${TARGETS_EXPORT_NAME}.cmake"
  NAMESPACE "${namespace}")

set(CPACK_PACKAGE_VENDOR "PROJECT_HOMEPAGE_URL")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
# set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
include(CPack)
