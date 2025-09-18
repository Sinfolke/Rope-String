cmake_minimum_required(VERSION 3.16)
project(Rope LANGUAGES CXX VERSION 0.1.0)

# Header-only library
add_library(Rope STATIC)
target_precompile_headers(Rope PUBLIC)
target_compile_features(Rope PUBLIC cxx_std_23)

# Include directories for users
target_include_directories(Rope INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_include_directories(Rope PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Modules
file(GLOB CPPMODULES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.cppm"
)

target_sources(Rope PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${CPPMODULES}
)

# Install headers + modules
include(GNUInstallDirs)
install(TARGETS Rope
        EXPORT RopeTargets
        FILE_SET cxx_modules DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rope
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export CMake package
install(EXPORT RopeTargets
        NAMESPACE Rope::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Rope
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/RopeConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
)

configure_file(RopeConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/RopeConfig.cmake" @ONLY
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/RopeConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/RopeConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Rope
)

enable_testing()

file(GLOB TESTS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)
foreach(TEST_FILE IN LISTS TESTS)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME}_test ${TEST_FILE})
    target_compile_options(${TEST_NAME}_test PRIVATE -g)
    target_link_libraries(${TEST_NAME}_test PRIVATE Rope)
    add_test(NAME ${TEST_NAME}_test COMMAND ${TEST_NAME}_test)
endforeach()