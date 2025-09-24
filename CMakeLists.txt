cmake_minimum_required(VERSION 3.25.2)

project(RAPL_utils
  LANGUAGES C
)

set(RAPL_UTILS_SRCS
  src/energy_utils.c
  src/msr_reader.c
)

add_library(RAPL_utils SHARED)
target_include_directories(RAPL_utils 
    PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>)
target_sources(RAPL_utils PRIVATE ${RAPL_UTILS_SRCS})

include(GNUInstallDirs)

install(TARGETS RAPL_utils
    EXPORT RAPL_utilsTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RAPL_utilsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/RAPL_utilsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RAPL_utils
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/RAPL_utilsConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RAPL_utils
)

install(EXPORT RAPL_utilsTargets
    FILE RAPL_utilsTargets.cmake
    NAMESPACE RAPL_utils::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RAPL_utils
    )