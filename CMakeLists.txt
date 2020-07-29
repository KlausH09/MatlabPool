cmake_minimum_required(VERSION  3.10.0 )

set(CMAKE_CXX_STANDARD  17)
set(CMAKE_CXX_STANDARD_REQUIRED  True)

project(MatlabPool)

find_package(Matlab REQUIRED ENG_LIBRARY MX_LIBRARY MAT_LIBRARY MAIN_PROGRAM) # TODO version

# library paths
link_directories(${Matlab_LIBRARIES})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib_static)

# functions
function(getLibPath libpath libname)
    set(${libpath} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}${libname}${CMAKE_SHARED_LIBRARY_SUFFIX}" PARENT_SCOPE)
endfunction()

# flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -fwrapv -DNDEBUG") # TODO

# includes
include_directories(${Matlab_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/include)

# definitions
getLibPath(MatlabPool_lib_path "MatlabPool")
add_definitions(-DMATLABPOOL_DISP_WORKER_OUTPUT
                -DMATLABPOOL_DISP_WORKER_ERROR
                -DMATLABPOOL_DLL_PATH="${MatlabPool_lib_path}")

# TODO static libary fuer ${PROJECT_SOURCE_DIR}/src/MatlabPool

# MatlabPool Library
aux_source_directory(${PROJECT_SOURCE_DIR}/src/MatlabPoolLib source)
add_library(MatlabPool MODULE ${source})
target_link_libraries(MatlabPool ${Matlab_LIBRARIES})


# MatlabPool MEX
matlab_add_mex(NAME MatlabPoolMEX SRC ${PROJECT_SOURCE_DIR}/src/MatlabPoolMEX/MatlabPoolMEX.cpp)


# C++ tests
set(target matlabpool_test)
add_executable(${target} ${PROJECT_SOURCE_DIR}/src/${target}.cpp)
target_link_libraries(${target} ${Matlab_LIBRARIES})
add_custom_target(cpp_test ${PROJECT_BINARY_DIR}/${target}${CMAKE_EXECUTABLE_SUFFIX})

# Matlab tests
file(COPY ${PROJECT_SOURCE_DIR}/matlabpool_test.m DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/MatlabPool.m DESTINATION ${PROJECT_BINARY_DIR})
add_custom_target(matlab_test ${Matlab_MAIN_PROGRAM} -nosplash -nojvm -r "matlabpool_test")
