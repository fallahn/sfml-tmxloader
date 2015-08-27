/*********************************************************************

Matt Marchant 2013 - 2015
SFML Tiled Map Loader - https://github.com/bjorn/tiled/wiki/TMX-Map-Format

The zlib license has been used to make this software fully compatible
with SFML. See http://www.sfml-dev.org/license.php

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

Usage
-----

First download the zlib library which matches your compiler and link it to your
project, then add the map loader .cpp files to your project/command line. Only
add the tmx2box2d files if you are using box2D for physics and wish to convert
map object data to box2D physics bodies.


To quickly get up and running create an instance of the MapLoader class

    tmx::MapLoader ml("path/to/maps");

load a map file

    ml.Load("map.tmx");

and draw it in your main loop

    renderTarget.draw(ml);


Note that the constructor takes a path to the directory containing the map files as a parameter (with
or without the trailing '/') so that you only need pass the map name to MapLoader::Load().If you have
images and/or tileset data in another directory you may add it with:

    ml.AddSearchPath(path);
    
*before* attempting to load the map file.

New maps can be loaded simply by calling the load function again, existing maps will be automatically
unloaded. MapLoader::Load() also returns true on success and false on failure, to aid running the function
in its own thread for example. Conversion functions are provided for converting coordinate spaces between
orthogonal and isometric. For instance MapLoader::OrthogonalToIsometric will convert mouse coordinates from
screen space:

    0--------X
    |
    |
    |
    |
    |
    Y

to Isometric space:

         0
        / \
       /   \
      /     \
     /       \
    Y         X

Layer information can be accessed through MapLoader::GetLayers()
    
    bool collision;
    for(auto layer = ml.GetLayers().begin(); layer != ml.GetLayers().end(); ++layer)
    {
        if(layer->name == "Collision")
        {
            for(auto object = layer->objects.begin(); object != layer->objects.end(); ++object)
            {
                collision = object->Contains(point);
            }
        }
    }


The quad tree is used to reduce the number of MapObjects used during a query. If a map contains
hundreds of objects which are used for collision it makes little sense to test them all if only
a few are ever within collision range. For example in your update loop first call:

    ml.UpdateQuadTree(myRect);

where myRect is the area representing the root node. You will probably only want to start with 
MapObjects which are visible on screen, so set myRect to your view area. Then query the quad tree
with another floatRect, representing the area for potential collision. This could be the bounding
box of your sprite:

    std::vector<MapObject*> objects = ml.QueryQuadTree(mySprite.getGlobalBounds());

This returns a vector of pointers to MapObjects contained within any quads which are intersected
by your sprite's bounds. You can then proceed to perform any collision testing as usual.


Some utility functions are providied in tmx2box2d.h/cpp. If you use box2D for physics then add these 
files to you project and then create box2D physics bodies using:

    b2Body* body = tmx::BodyCreator::Add(mapObject, b2World);
    
where b2World is a reference to a valid box2D physics world. As box2D uses a different coordinate
system to SFML there are 4 functions for converting from one space to another:

        b2Vec2 SfToBoxVec(const sf::Vector2f& vec);
        sf::Vector2f BoxToSfVec(const b2Vec2& vec);
        float SfToBoxFloat(float val);
        float BoxToSfFloat(float val);

You should use these whenever you are trying to draw with SFML based on the physics output, or set
box2D parameters in SFML world values.


Debugging output can be enabled with one of the following preprocessor directives:

    #define LOG_OUTPUT_CONSOLE

all output is directed to the console window
    
    #define LOG_OUTPUT_FILE

all output is written to a file named output.log in the executable directory
    
    #define LOG_OUTPUT_ALL

log output is directed to both the console and output.log
    
Logging is diabled by default. The level of log information can be set with

    Logger::SetLogLevel()
    
by providing a bitmask for the level required. For instance to only log warnings
and errors use:

    Logger::SetLogLevel(Logger::Warning | Logger::Error);



For more detailed examples see the source in the examples folder, and the wiki on github:
https://github.com/fallahn/sfml-tmxloader/wiki



Requirements
------------

pugixml (included)
zlib (http://zlib.net/)
SFML 2.x (http://sfml-dev.org)
Minimal C++11 compiler support (tested with VS11 and GCC4.7)


Note about CMake file
---------------------

The tmx map loader was designed to have its source included as part of another project. The
CMake file is included for linux / mac platforms where building installing libraries is the
norm. The tmx source doesn't explicitly export any functions, so when building the tmx files
as a library under Windows (particularly Visual Studio) VC will not generate any lib files.
To work around this just include the tmx source files in your Visual Studio project (as well
as pugixml), so that they are compiled as part of the final executable.


Revision History
----------------

300513 0.1 Initial Release

140613 0.1a Bug fix compiling with MinGW/GCC

240613 0.2 Added: Quadtree support for partitioning of MapObjects, Getter for map size property

030713 0.3 Added: rendering of map via vertex array. Old style rendering is still available with Draw2()

190713 0.4 Updated: MapObject::SetPosition and MapObject::Move now update object's debug and AABB properties.
       Added: Map layers can be drawn by type (front, back, all or debug) and by index.
       Removed: debug param from draw function - debug info is now drawn separately
       Updated: minimum version of visual studio required is now VS11

060813 0.5 Updated: tileset images are cached and reused when the same tileset is used more than once

280813 0.6 Updated: optimised draw calls by checking if a tileset vertex array is actually visible before
            drawing
       Fixed: Textures not properly being flushed when loading new maps causing tile sets to be incorrectly
           displayed
           Added: MapLayer RenderStates property and setter for shader

041013 0.7 Added: Getters for object first / last points
       Removed: old Draw2 function
       Updated: constness of some function parameters
       Updated: inherited load class from sf::Drawable and sf::NonCopyable
       Updated: removed tile caching in favour of drawing objects via sprites with subrects for significantly
            improved loading times - particularly with large or multiple tile sets

291013 0.8 Updated: Refactored object class and cleaned up interface
       Added: utility functions to MapObject class for collision calculations

131113 0.9 Fixed: MapObject::Intersects failed due to const correctness
       Rehosted project on GitHub
       
