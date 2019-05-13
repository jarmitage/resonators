cmake_minimum_required(VERSION 3.10)

project(resonators VERSION 0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

execute_process(COMMAND python -c "import sysconfig, os; print(os.path.join(sysconfig.get_config_var('LIBPL'), sysconfig.get_config_var('LIBRARY')))" OUTPUT_VARIABLE PYTHON_LIBRARY OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND python -c "import sysconfig; print(sysconfig.get_config_var('INCLUDEPY'))" OUTPUT_VARIABLE PYTHON_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
find_package(PythonLibs)
message(STATUS "PYTHON_LIBRARY: ${PYTHON_LIBRARY}")
message(STATUS "PYTHON_LIBRARIES: ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_INCLUDE_PATH: ${PYTHON_INCLUDE_PATH}")
message(STATUS "PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}")
message(STATUS "PYTHONLIBS_VERSION_STRING: ${PYTHONLIBS_VERSION_STRING}")

include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(cpp include)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/py)
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/py)
set(CMAKE_SWIG_FLAGS "")

add_library(resonatorscpp SHARED include/JSON.h include/JSON.cpp include/JSONValue.h include/JSONValue.cpp cpp/Model.h cpp/Resonators.h cpp/Resonators.cpp)

set_source_files_properties(py/resonators.i PROPERTIES CPLUSPLUS ON)

swig_add_library(resonators LANGUAGE python SOURCES py/resonators.i)
swig_link_libraries(resonators ${PYTHON_LIBRARIES} resonatorscpp)
