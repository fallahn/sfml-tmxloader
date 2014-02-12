project(tmx-loader)
cmake_minimum_required(VERSION 2.8)
include_directories(${CMAKE_SOURCE_DIR}/include)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

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
        include/tmx/Helpers.h)

set(tmx_SRCS
		src/DebugShape.cpp
        src/MapLoaderPublic.cpp
        src/MapLoaderPrivate.cpp
        src/MapLayer.cpp
        src/MapObject.cpp
        src/QuadTreeNode.cpp)

add_library(pugi SHARED ${pugi_HDRS} ${pugi_SRCS})
add_library(${PROJECT_NAME} SHARED ${tmx_HDRS} ${tmx_SRCS})
install(TARGETS pugi ${PROJECT_NAME} LIBRARY DESTINATION lib)
install(FILES ${tmx_HDRS} DESTINATION include/tmx)
install(FILES ${pugi_HDRS} DESTINATION include/pugixml)

FILE(GLOB maps "${CMAKE_CURRENT_SOURCE_DIR}/maps/*")
install(FILES ${maps} DESTINATION share/tmx/examples/maps)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(SFML 2 REQUIRED system window graphics)
find_package(ZLIB REQUIRED)
find_package(BOX2D)

if (SFML_FOUND)
	add_executable(BenchMark examples/Benchmark.cpp)
	target_link_libraries(BenchMark ${PROJECT_NAME})
	target_link_libraries(BenchMark pugi)
	target_link_libraries(BenchMark ${SFML_LIBRARIES})
	target_link_libraries(BenchMark ${ZLIB_LIBRARIES})
	install(TARGETS BenchMark RUNTIME DESTINATION share/tmx/examples)

	add_executable(DrawWithDebug examples/DrawMapWithDebug.cpp)
	target_link_libraries(DrawWithDebug ${PROJECT_NAME})
	target_link_libraries(DrawWithDebug pugi)
	target_link_libraries(DrawWithDebug ${SFML_LIBRARIES})
	target_link_libraries(DrawWithDebug ${ZLIB_LIBRARIES})
	install(TARGETS DrawWithDebug RUNTIME DESTINATION share/tmx/examples)

	add_executable(Isometric examples/IsometricWithConvertCoords.cpp)
	target_link_libraries(Isometric ${PROJECT_NAME})
	target_link_libraries(Isometric pugi)
	target_link_libraries(Isometric ${SFML_LIBRARIES})
	target_link_libraries(Isometric ${ZLIB_LIBRARIES})
	install(TARGETS Isometric RUNTIME DESTINATION share/tmx/examples)

	add_executable(QuadTree examples/MapWithQuadTree.cpp)
	target_link_libraries(QuadTree ${PROJECT_NAME})
	target_link_libraries(QuadTree pugi)
	target_link_libraries(QuadTree ${SFML_LIBRARIES})
	target_link_libraries(QuadTree ${ZLIB_LIBRARIES})
	install(TARGETS QuadTree RUNTIME DESTINATION share/tmx/examples)

	add_executable(ShaderEffects examples/ShaderEffects.cpp)
	target_link_libraries(ShaderEffects ${PROJECT_NAME})
	target_link_libraries(ShaderEffects pugi)
	target_link_libraries(ShaderEffects ${SFML_LIBRARIES})
	target_link_libraries(ShaderEffects ${ZLIB_LIBRARIES})
	install(TARGETS ShaderEffects RUNTIME DESTINATION share/tmx/examples)
	
	if(BOX2D_FOUND)	
		add_executable(Box2D examples/Box2D.cpp src/tmx2box2d.cpp)
		target_link_libraries(Box2D ${PROJECT_NAME})
		target_link_libraries(Box2D pugi)
		target_link_libraries(Box2D ${SFML_LIBRARIES})
		target_link_libraries(Box2D ${ZLIB_LIBRARIES})	
		target_link_libraries(Box2D ${BOX2D_LIBRARIES})
		install(TARGETS Box2D RUNTIME DESTINATION share/tmx/examples)
		
	endif (BOX2D_FOUND)
	
endif (SFML_FOUND)
