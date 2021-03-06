#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Ogre HLMS Physically Based Shading for OpenGL 3.3+ and D3D11 build

PROJECT(OgreHlmsInk)

file(
	GLOB HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Cubemaps/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/InstantRadiosity/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Common/include/*.h"
)
file(
	GLOB SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Cubemaps/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/InstantRadiosity/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Common/src/*.cpp"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common/include)

ogre_add_library(OgreHlmsInk ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreHlmsInk PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreHlmsInk OgreMain)

ogre_config_framework(OgreHlmsInk)
ogre_config_component(OgreHlmsInk)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Hlms/Ink
)
