cmake_minimum_required(VERSION 3.25.2)

project(Power_meter
  LANGUAGES CXX CUDA
)

find_package(CUDAToolkit REQUIRED)

set(RAPL_UTILS_SRCS
  src/rapl_utils.cc
  src/nvml_utils.cc
  src/msr_reader.cc
  src/power_meter.cc
)

add_library(Power_meter SHARED)
target_include_directories(Power_meter 
    PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>)
target_sources(Power_meter PRIVATE ${RAPL_UTILS_SRCS})
target_link_libraries(Power_meter CUDA::nvml)

include(GNUInstallDirs)

install(TARGETS Power_meter
    EXPORT Power_meterTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Power_meterConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Power_meterConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Power_meter
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Power_meterConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Power_meter
)

install(EXPORT Power_meterTargets
    FILE Power_meterTargets.cmake
    NAMESPACE Power_meter::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Power_meter
    )