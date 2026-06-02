# $Id: CMakeLists.txt 68058 2013-03-13 14:47:43Z gcosmo $
# usage: cmake -DGeant4_DIR=$G4HOME ../oppac_1

#----------------------------------------------------------------------------
# Setup the project
#

cmake_minimum_required(VERSION 3.16...3.27)
project(scinti)

#set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ Standard Version")

#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()


#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
#include_directories(${PROJECT_SOURCE_DIR}/include)


# Find ROOT (required package)
#
find_package(ROOT REQUIRED)
#include_directories(${ROOT_INCLUDE_DIRS})
#link_directories(${ROOT_LIBRARY_DIR})
#add_definitions(${ROOT_CXX_FLAGS})
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/shared/include
                    ${Geant4_INCLUDE_DIR}
                    ${ROOT_INCLUDE_DIRS})

# Add ROOT-specific definitions to avoid conflicts
#add_definitions(-DROOT_R__NO_STRING_VIEW)  # Prevent conflicts with std::string_view
#add_definitions(${ROOT_CXX_FLAGS})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(exe_scinti scinti.cc ${sources} ${headers})
#target_link_libraries(exe_scinti ${Geant4_LIBRARIES})
target_link_libraries(exe_scinti ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} )
#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build scinti. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#

file(GLOB MACRO_FILES
  "*.mac"
)
file(COPY ${MACRO_FILES} DESTINATION ${PROJECT_BINARY_DIR})

set(EXAMPLEscinti_SCRIPTS
  )

foreach(_script ${EXAMPLEscinti_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# For internal Geant4 use - but has no effect if you build this
# example standalone
#
add_custom_target(scinti DEPENDS exe_scinti)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS exe_scinti DESTINATION bin )

