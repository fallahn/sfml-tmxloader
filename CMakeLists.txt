cmake_minimum_required(VERSION 2.8)

project(tmx-loader)

# Set options
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)")
set(BUILD_SHARED_LIBS TRUE CACHE BOOL "TRUE to build light as shared libraries, FALSE to build it as static libraries")
set(SFML_STATIC_LIBS FALSE CACHE BOOL "Choose whether SFML is linked statically or not.")
set(TMX_STATIC_STD_LIBS FALSE CACHE BOOL "Use statically linked standard/runtime libraries? This option must match the one used for SFML.")
set(TMX_BUILD_EXAMPLE FALSE BOOL CACHE BOOL "TRUE to build the tmx-loader example, FALSE to ignore them")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# tmx-loader uses C++11 features
if(CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# on MacOSX you need to use the correct stdlib for c++11 features
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# Make sure that the runtime library gets link statically
if(TMX_STATIC_STD_LIBS)
	if(NOT SFML_STATIC_LIBS)
		message("\n-> If you check TMX_STATIC_STD_LIBS, you also need to check SFML_STATIC_LIBRARIES.")
		message("-> It would lead to multiple runtime environments which result in undefined behavior.\n")
	elseif(WIN32 AND MSVC)
		# Change all MSVC compiler flags to /MT
		foreach(flag CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
			if(${flag} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
			endif()
		endforeach()
	elseif(CMAKE_COMPILER_IS_GNUCXX)
		# Note: Doesn't work for TDM compiler, since it's compiling the runtime libs statically by default
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
	endif()
endif()

# Make sure that FindSFML.cmake searches for the static libraries
if(SFML_STATIC_LIBS)
    add_definitions(-DSFML_STATIC)
	set(SFML_STATIC_LIBRARIES TRUE)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(pugi_HDRS
        include/pugixml/pugiconfig.hpp
        include/pugixml/pugixml.hpp)

set(pugi_SRCS
        src/pugixml/pugixml.cpp)

set(tmx_HDRS
        include/tmx/MapLayer.h
        include/tmx/MapLoader.h
        include/tmx/MapObject.h
        include/tmx/QuadTreeNode.h
        include/tmx/DebugShape.h
        include/tmx/Helpers.h
        include/tmx/Log.h)

set(tmx_SRCS
		src/DebugShape.cpp
        src/MapLoaderPublic.cpp
        src/MapLoaderPrivate.cpp
        src/MapLayer.cpp
        src/MapObject.cpp
        src/QuadTreeNode.cpp)

set(maps_SRCS "${PROJECT_SOURCE_DIR}/maps/")
set(fonts_SRCS "${PROJECT_SOURCE_DIR}/fonts/")

add_library(pugi ${pugi_HDRS} ${pugi_SRCS})
add_library(${PROJECT_NAME} ${tmx_HDRS} ${tmx_SRCS})


find_package(SFML 2 REQUIRED graphics window system)

if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
elseif(NOT SFML_FOUND)
	set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
	message("\n-> SFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
	message("-> Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

find_package(ZLIB REQUIRED)

if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
elseif(NOT ZLIB_FOUND)
    set(ZLIB_ROOT "" CACHE PATH "zlib top-level directory")
	message("\n-> zlib directory not found. Set ZLIB_ROOT to zlib's top-level path (containing \"include\" and \"lib\" directories).")
	message("-> Make sure the zlib libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()


find_package(BOX2D)

if(BOX2D_FOUND)
    include_directories(${BOX2D_INCLUDE_DIR})
elseif(NOT BOX2D_FOUND)
    set(BOX2D_INCLUDE_DIR "" CACHE PATH "Box2D include directory")
    set(BOX2D_LIBRARIES "" CACHE PATH "Box2D lib directory")
	message("\n-> Box2D directories not found. Set BOX2D_INCLUDE_DIR to Box2D's include path and BOX2D_LIBRARIES to Box2D's lib directory path.")
	message("-> Make sure the Box2D libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

#target_link_libraries(pugi ${ZLIB_LIBRARIES})
target_link_libraries(tmx-loader pugi ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${ZLIB_LIBRARIES})

# Adjust the output file prefix/suffix to match our conventions
if(BUILD_SHARED_LIBS)
    if(WIN32)
        set_target_properties(pugi PROPERTIES DEBUG_POSTFIX -d)
        set_target_properties(tmx-loader PROPERTIES DEBUG_POSTFIX -d)
        
        if(CMAKE_COMPILER_IS_GNUCXX)
            # on Windows/gcc get rid of "lib" prefix for shared libraries,
            # and transform the ".dll.a" suffix into ".a" for import libraries
            set_target_properties(pugi PROPERTIES PREFIX "")
            set_target_properties(tmx-loader PROPERTIES PREFIX "")
            set_target_properties(pugi PROPERTIES IMPORT_SUFFIX ".a")
            set_target_properties(tmx-loader PROPERTIES IMPORT_SUFFIX ".a")
        endif()
    endif()
else()
    set_target_properties(pugi PROPERTIES DEBUG_POSTFIX -s-d)
    set_target_properties(tmx-loader PROPERTIES DEBUG_POSTFIX -s-d)
    set_target_properties(pugi PROPERTIES RELEASE_POSTFIX -s)
    set_target_properties(tmx-loader PROPERTIES RELEASE_POSTFIX -s)
    set_target_properties(pugi PROPERTIES MINSIZEREL_POSTFIX -s)
    set_target_properties(tmx-loader PROPERTIES MINSIZEREL_POSTFIX -s)
endif()

# Install pugi
install(
    TARGETS pugi
    RUNTIME DESTINATION bin COMPONENT bin
    LIBRARY DESTINATION lib COMPONENT bin
    ARCHIVE DESTINATION lib COMPONENT dev
)

# Install tmx-loader
install(
    TARGETS tmx-loader
    RUNTIME DESTINATION bin COMPONENT bin
    LIBRARY DESTINATION lib COMPONENT bin
    ARCHIVE DESTINATION lib COMPONENT dev
)
install(DIRECTORY include DESTINATION .)

# Build and install the examples
if(TMX_BUILD_EXAMPLE)
	add_executable(BenchMark examples/Benchmark.cpp)
	target_link_libraries(BenchMark ${PROJECT_NAME} pugi ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${ZLIB_LIBRARIES})
	install(TARGETS BenchMark RUNTIME DESTINATION share/tmx/examples)

	add_executable(DrawWithDebug examples/DrawMapWithDebug.cpp)
	target_link_libraries(DrawWithDebug ${PROJECT_NAME} pugi ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${ZLIB_LIBRARIES})
	install(TARGETS DrawWithDebug RUNTIME DESTINATION share/tmx/examples)

	add_executable(Isometric examples/IsometricWithConvertCoords.cpp)
	target_link_libraries(Isometric ${PROJECT_NAME} pugi ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${ZLIB_LIBRARIES})
	install(TARGETS Isometric RUNTIME DESTINATION share/tmx/examples)

	add_executable(QuadTree examples/MapWithQuadTree.cpp)
	target_link_libraries(QuadTree ${PROJECT_NAME} pugi ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${ZLIB_LIBRARIES})
	install(TARGETS QuadTree RUNTIME DESTINATION share/tmx/examples)

	add_executable(ShaderEffects examples/ShaderEffects.cpp)
	target_link_libraries(ShaderEffects ${PROJECT_NAME} pugi ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${ZLIB_LIBRARIES})
	install(TARGETS ShaderEffects RUNTIME DESTINATION share/tmx/examples)
	
	if(BOX2D_FOUND)	
		add_executable(Box2D examples/Box2D.cpp src/tmx2box2d.cpp)
		target_link_libraries(Box2D ${PROJECT_NAME} pugi ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${ZLIB_LIBRARIES} ${BOX2D_LIBRARIES})
		install(TARGETS Box2D RUNTIME DESTINATION share/tmx/examples)
	endif()

    install(DIRECTORY maps/
		    DESTINATION share/tmx/examples/maps)
    install(DIRECTORY fonts/
		    DESTINATION share/tmx/examples/fonts)
endif()