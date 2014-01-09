project(tmx-loader)
cmake_minimum_required(VERSION 2.8)
include_directories(${CMAKE_SOURCE_DIR}/include)

set(pugi_HDRS
        include/pugixml/pugiconfig.hpp
        include/pugixml/pugixml.hpp
)

set(pugi_SRCS
        src/pugixml/pugixml.cpp
)

add_library(pugi ${pugi_HDRS} ${pugi_SRCS})

set(tmx_HDRS

        include/tmx/MapLayer.h
        include/tmx/MapLoader.h
        include/tmx/MapObject.h
        include/tmx/QuadTreeNode.h
)

set(tmx_SRCS
	src/DebugShape.cpp
        src/MapLoaderPublic.cpp
        src/MapLoaderPrivate.cpp
        src/MapLayer.cpp
        src/MapObject.cpp
        src/QuadTreeNode.cpp
)

add_library(${PROJECT_NAME} ${tmx_HDRS} ${tmx_SRCS})

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(SFML 2 REQUIRED system windows graphics)
find_package(ZLIB REQUIRED)
find_package(BOX2D)

if (SFML_FOUND)
	add_executable(BenchMark examples/Benchmark.cpp)
	target_link_libraries(BenchMark ${PROJECT_NAME})
	target_link_libraries(BenchMark pugi)
	target_link_libraries(BenchMark ${SFML_LIBRARIES})
	target_link_libraries(BenchMark ${ZLIB_LIBRARIES})

	add_executable(DrawWithDebug examples/DrawMapWithDebug.cpp)
	target_link_libraries(DrawWithDebug ${PROJECT_NAME})
	target_link_libraries(DrawWithDebug pugi)
	target_link_libraries(DrawWithDebug ${SFML_LIBRARIES})
	target_link_libraries(DrawWithDebug ${ZLIB_LIBRARIES})

	add_executable(Isometric examples/IsometricWithConvertCoords.cpp)
	target_link_libraries(Isometric ${PROJECT_NAME})
	target_link_libraries(Isometric pugi)
	target_link_libraries(Isometric ${SFML_LIBRARIES})
	target_link_libraries(Isometric ${ZLIB_LIBRARIES})

	add_executable(QuadTree examples/MapWithQuadTree.cpp)
	target_link_libraries(QuadTree ${PROJECT_NAME})
	target_link_libraries(QuadTree pugi)
	target_link_libraries(QuadTree ${SFML_LIBRARIES})
	target_link_libraries(QuadTree ${ZLIB_LIBRARIES})

	add_executable(ShaderEffects examples/ShaderEffects.cpp)
	target_link_libraries(ShaderEffects ${PROJECT_NAME})
	target_link_libraries(ShaderEffects pugi)
	target_link_libraries(ShaderEffects ${SFML_LIBRARIES})
	target_link_libraries(ShaderEffects ${ZLIB_LIBRARIES})
	
	if(BOX2D_FOUND)	
		add_executable(Box2D examples/Box2D.cpp src/tmx2box2d.cpp)
		target_link_libraries(Box2D ${PROJECT_NAME})
		target_link_libraries(Box2D pugi)
		target_link_libraries(Box2D ${SFML_LIBRARIES})
		target_link_libraries(Box2D ${ZLIB_LIBRARIES})	
		target_link_libraries(Box2D ${BOX2D_LIBRARIES})
		
	endif (BOX2D_FOUND)
	
endif (SFML_FOUND)