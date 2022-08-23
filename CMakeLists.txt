# Minimum CMake version required to generate
# our build system
cmake_minimum_required(VERSION 3.5)

# Project name
project("pygraver"
	VERSION "0.1.0"
	LANGUAGES CXX C
)

option(BUILD_TESTS "Build tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

INCLUDE(GNUInstallDirs)

find_package (Python3 COMPONENTS Interpreter Development REQUIRED)

if(SKBUILD)
  find_package(PythonExtensions REQUIRED)
endif()

# add pybind11 cmake dir
execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" -c
          "import pybind11; print(pybind11.get_cmake_dir())"
  OUTPUT_VARIABLE _tmp_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT)
list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")
find_package(pybind11 CONFIG REQUIRED)

# Find GEOS; geos_config.cmake doesn't define variables
# => we need to do it ourself
find_package(geos REQUIRED)
# get LD_FLAGS from geos-config command
execute_process(COMMAND geos-config --libs
  OUTPUT_VARIABLE GEOS_LD_FLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
# extract library dir from LD_FLAGS
STRING(REGEX MATCHALL "-L([^ ]+)" GEOS_LIB_FLAGS "${GEOS_LD_FLAGS}")
STRING(REGEX REPLACE "-L" "" GEOS_LIBRARY_DIR ${GEOS_LIB_FLAGS})
# find include dir
FIND_PATH(GEOS_INCLUDE_DIR geos_c.h)
message(GEOS " includes found in ${GEOS_INCLUDE_DIR}")
message(GEOS " library found in ${GEOS_LIBRARY_DIR}")
FIND_PACKAGE(VTK REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
find_package(Boost REQUIRED)

find_package(fmt)

add_definitions(${Boost_DEFINITIONS})

set(PYGRAVER_INCLUDE_DIRS ${LIBXML2_INCLUDE_DIR} ${Python3_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${GEOS_INCLUDE_DIR} ${pybind11_INCLUDE_DIR} ${VTK_INCLUDE_DIRS})

add_library (core SHARED
  src/types/point.cpp src/types/path.cpp src/types/pathgroup.cpp src/types/surface.cpp
  src/svg/file.cpp
  src/render/shape3d.cpp src/render/extrusion.cpp src/render/wire.cpp src/render/marker.cpp
  src/render/cylinder.cpp src/render/model.cpp src/render/vtkevents.cpp
  src/exports.cpp src/svg/exports.cpp src/render/exports.cpp
)
target_include_directories(core PUBLIC ${PYGRAVER_INCLUDE_DIRS})
target_link_directories(core PUBLIC ${Python3_LIBRARY_DIRS} ${GEOS_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})
target_link_libraries(core ${LIBXML2_LIBRARIES} ${Python3_LIBRARIES} ${Boost_LIBRARIES} ${VTK_LIBRARIES} geos fmt)
target_compile_definitions(core PRIVATE VERSION_INFO=${PROJECT_VERSION})

set_target_properties(core PROPERTIES PREFIX "")
set_target_properties(core PROPERTIES SUFFIX ".so")

install(TARGETS core LIBRARY DESTINATION .)

if (BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_executable(
    pygraver_test
    src/tests/types/point.cpp src/tests/types/path.cpp src/tests/types/pathgroup.cpp src/tests/types/surface.cpp
    src/tests/svg/arc.cpp src/tests/svg/bezier3.cpp src/tests/svg/line.cpp src/tests/svg/path.cpp
    src/tests/svg/file.cpp
    src/tests/render/extrusion.cpp src/tests/render/shape3d.cpp src/tests/render/marker.cpp
    src/tests/render/wire.cpp
  )
  target_include_directories(
    pygraver_test PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${PYGRAVER_INCLUDE_DIRS}
  )
  target_link_libraries(
    pygraver_test
    core
    gtest_main
  )
  include(GoogleTest)
  gtest_discover_tests(pygraver_test)
endif()
