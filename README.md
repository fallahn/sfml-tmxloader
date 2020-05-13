NOTE: Development of this project is indefinitely suspended in favour of [tmxlite](https://github.com/fallahn/tmxlite)
which supports generic rendering across C++ frameworks such as SFML and SDL2, requires no external linkage and
has broader platform support, including mobile devices.

Tiled tmx map loader for SFML 2.0.0
-----------------------------------


/*********************************************************************

Matt Marchant 2013 - 2016  
SFML Tiled Map Loader - https://github.com/bjorn/tiled/wiki/TMX-Map-Format  

Zlib License:  
This software is provided 'as-is', without any express or  
implied warranty. In no event will the authors be held  
liable for any damages arising from the use of this software.  

Permission is granted to anyone to use this software for any purpose,  
including commercial applications, and to alter it and redistribute  
it freely, subject to the following restrictions:  

1. The origin of this software must not be misrepresented;  
   you must not claim that you wrote the original software.  
   If you use this software in a product, an acknowledgment  
   in the product documentation would be appreciated but  
   is not required.  

2. Altered source versions must be plainly marked as such,  
   and must not be misrepresented as being the original software.  

3. This notice may not be removed or altered from any  
   source distribution.  

*********************************************************************/  


This class is designed to load TILED .tmx format maps, compatible with  
TILED up to version 0.9.0  

http://trederia.blogspot.co.uk/2013/05/tiled-map-loader-for-sfml.html


What's Supported
----------------

Uses pugixml (included) to parse xml  
Supports orthogonal maps  
Supports isometric maps  
Supports conversion between orthogonal and isometric world coords  
Parses all types of layers (normal, object and image), layer properties  
Parses all types of object, object shapes, types, properties  
Option to draw debug output of objects and tile grid  
Supports multiple tile sets, including tsx files and collections of images  
Supports all layer encoding and compression: base64, csv, zlib, gzip and xml (requires zlib library, see /lib directory)  
Quad tree partitioning / querying of map object data  
Optional utility functions for converting tmx map objects into box2D body data  


What's not supported / limitations
----------------------------------

Parsing of individual tile properties  
Staggered isometric maps


Requirements
------------

pugixml (included)  
zlib (http://zlib.net/)  
SFML 2.x (http://sfml-dev.org)  
Minimal C++11 compiler support (tested with VS11 and GCC4.7)   


Usage
-----

With the Cmake file provided create a project for the compiler of your choice to build and
install the map loader as either a static or shared library. You can use cmake-gui (useful
for windows users) to see all the options, such as building the example applications.


To quickly get up and running create an instance of the MapLoader class

    tmx::MapLoader ml("path/to/maps");

load a map file

    ml.load("map.tmx");

and draw it in your main loop

    renderTarget.draw(ml);


Note that the constructor takes a path to the directory containing the map files as a parameter (with
or without the trailing '/') so that you only need pass the map name to `MapLoader::load()`. If you have
images and/or tileset data in another directory you may add it with:

    ml.addSearchPath(path);
    
*before* attempting to load the map file.

New maps can be loaded simply by calling the load function again, existing maps will be automatically
unloaded. `MapLoader::load()` also returns true on success and false on failure, to aid running the function
in its own thread for example. Conversion functions are provided for converting coordinate spaces between
orthogonal and isometric. For instance `MapLoader::orthogonalToIsometric()` will convert mouse coordinates from
screen space:

    0--------X
    |
    |
    |
    |
    |
    Y

to isometric space:

         0
        / \
       /   \
      /     \
     /       \
    Y         X

Layer information can be accessed through `MapLoader::getLayers()`
    
    bool collision;
    const auto& layers = ml.getLayers();
    for(const auto& layer : layers)
    {
        if(layer.name == "Collision")
        {
            for(const auto& object : layer.objects)
            {
                collision = object.contains(point);
            }
        }
    }


The quad tree is used to reduce the number of MapObjects used during a query. If a map contains
hundreds of objects which are used for collision it makes little sense to test them all if only
a few are ever within collision range. For example in your update loop first call:

    ml.updateQuadTree(myRect);

where `myRect` is the area representing the root node. You will probably only want to start with 
MapObjects which are visible on screen, so set `myRect` to your view area. Then query the quad tree
with another FloatRect representing the area for potential collision. This could be the bounding
box of your sprite:

    std::vector<MapObject*> objects = ml.queryQuadTree(mySprite.getGlobalBounds());

This returns a vector of pointers to MapObjects contained within any quads which are intersected
by your sprite's bounds. You can then proceed to perform any collision testing as usual.


Some utility functions are providied in tmx2box2d.h/cpp. If you use box2d for physics then add these 
files to you project, or set the box2d option to true when configuring the cmake file. You may then
create box2d physics bodies using the BodyCreator:

	sf::Vector2u tileSize(16, 16);
	b2Body* body = tmx::BodyCreator::add(mapObject, b2World, tileSize);
    
where b2World is a reference to a valid box2D physics world. The body creator is only a utility
class, so it is no problem letting it go out of scope once your bodies are all created. As box2d 
uses a different coordinate system to SFML there are 4 functions for converting from one space to
another:

        b2Vec2 sfToBoxVec(const sf::Vector2f& vec);
        sf::Vector2f boxToSfVec(const b2Vec2& vec);
        float sfToBoxFloat(float val);
        float boxToSfFloat(float val);

You should use these whenever you are trying to draw with SFML based on the physics output, or set
box2d parameters in SFML world values. When using box2d you may find that the build types of both
the map loader and box2d should match (ie both static or both shared libs).


Debugging output can be enabled with one of the following preprocessor directives:

    #define LOG_OUTPUT_CONSOLE

all output is directed to the console window
    
    #define LOG_OUTPUT_FILE

all output is written to a file named output.log in the executable directory
    
    #define LOG_OUTPUT_ALL

log output is directed to both the console and output.log
    
Logging is disabled by default. The level of log information can be set with

    tmx::setLogLevel()
    
by providing a bitmask for the level required. For instance to only log warnings
and errors use:

    tmx::setLogLevel(tmx::Logger::Warning | tmx::Logger::Error);



For more detailed examples see the source in the examples folder, and the wiki on github:
https://github.com/fallahn/sfml-tmxloader/wiki

Doxygen documentation can also be generated with the doxy file in the docs folder.
