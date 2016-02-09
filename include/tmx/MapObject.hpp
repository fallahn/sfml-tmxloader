/*********************************************************************
Matt Marchant 2013 - 2016
SFML Tiled Map Loader - https://github.com/bjorn/tiled/wiki/TMX-Map-Format
						http://trederia.blogspot.com/2013/05/tiled-map-loader-for-sfml.html

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

#ifndef MAP_OBJECT_HPP_
#define MAP_OBJECT_HPP_

#include <tmx/Helpers.hpp>
#include <tmx/DebugShape.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/System/NonCopyable.hpp>

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <memory>

namespace tmx
{
	class TileQuad;

	enum MapObjectShape
	{
		Rectangle,
		Ellipse,
		Circle,
		Polygon,
		Polyline,
		Tile
	};

	//map object class.
	class TMX_EXPORT_API MapObject final
	{
	private:
		struct Segment
		{
			Segment(const sf::Vector2f& start, const sf::Vector2f& end)
				: Start (start) , End(end){}

			bool intersects(const Segment& segment);

			sf::Vector2f Start;
			sf::Vector2f End;
		};
	public:
		MapObject();

		//**accessors**//
		//returns empty string if property not found
		std::string getPropertyString(const std::string& name);
		//returns top left corner of bounding rectangle
		sf::Vector2f getPosition() const {return m_position;}
		//returns precomputed centre of mass, or zero for polylines
		sf::Vector2f getCentre() const {return m_centrePoint;};
		//returns the type of shape of the object
		MapObjectShape getShapeType() const {return m_shape;};
		//returns and object's name
		std::string getName() const {return m_name;};
		//returns the object's type
		std::string getType() const {return m_type;};
		//returns the name of the object's parent layer
		std::string getParent() const {return m_parent;};
		//returns the objects AABB in world coordinates
		sf::FloatRect getAABB() const {return m_AABB;};
		//returns visibility
		bool visible() const {return m_visible;}
		//sets a property value, or adds it if property doesn't exist
		void setProperty(const std::string& name, const std::string& value);
		//sets the object position in world coords
		void setPosition(float x, float y);
		void setPosition(const sf::Vector2f& position);
		//moves the object by given amount
		void move(float x, float y);
		void move(const sf::Vector2f& distance);
		//sets the width and height of the object
		void setSize(const sf::Vector2f& size){m_size = size;};
		//sets the object's name
		void setName(const std::string& name){m_name = name;}
		//sets the object's type
		void setType(const std::string& type){m_type = type;};
		//sets the name of the object's parent layer
		void setParent(const std::string& parent){m_parent = parent;};
		//sets the shape type
		void setShapeType(MapObjectShape shape){m_shape = shape;};
		//sets visibility
		void setVisible(bool visible){m_visible = visible;};
		//adds a point to the list of polygonal points. If calling this manually
		//call CreateDebugShape() afterwards to rebuild debug output
		void addPoint(const sf::Vector2f& point){m_polypoints.push_back(point);};

		//checks if an object contains given point in world coords.
		//Always returns false for polylines.
		bool contains(sf::Vector2f point) const;
		//checks if two objects intersect, including polylines
		bool intersects(const MapObject& object) const;
		//creates a shape used for debug drawing - points are in world space
		void createDebugShape(const sf::Color& colour);
		//draws debug shape to given target
		void drawDebugShape(sf::RenderTarget& rt) const;
		//returns the first point of poly point member (if any)
		sf::Vector2f firstPoint() const;
		//returns the last point of poly point member (if any)
		sf::Vector2f lastPoint() const;
		//returns a unit vector normal to the polyline segment if intersected
		//takes the start and end point of a trajectory
		sf::Vector2f collisionNormal(const sf::Vector2f& start, const sf::Vector2f& end) const;
		//creates a vector of segments making up the poly shape
		void createSegments();
		//returns if an objects poly shape is convex or not
		bool convex() const;
		//returns a reference to the array of points making up the object
		const std::vector<sf::Vector2f>& polyPoints() const;
		//reversing winding of object points
		void reverseWinding();
		//sets the quad used to draw the tile for tile objects
		void setQuad(TileQuad* quad);

private:
		//object properties, reflects those which are part of the tmx format
		std::string m_name, m_type, m_parent; //parent is name of layer to which object belongs
		//sf::FloatRect m_rect; //width / height property of object plus position in world space
		sf::Vector2f m_position, m_size;
		std::map <std::string, std::string> m_properties;//map of custom name/value properties
		bool m_visible;
		std::vector<sf::Vector2f> m_polypoints; //list of points defining any polygonal shape
		MapObjectShape m_shape;
		DebugShape m_debugShape;
		sf::Vector2f m_centrePoint;

		std::vector<Segment> m_polySegs; //segments which make up shape, if any
		TileQuad* m_tileQuad;
		
		float m_furthestPoint; //furthest distance from centre of object to vertex - used for intersection testing
		//AABB created from polygonal shapes, used for adding MapObjects to a QuadTreeNode.
		//Note that the position of this box many not necessarily match the MapObject's position, as polygonal
		//points may have negative values relative to the object's world position.
		sf::FloatRect m_AABB; 

		//returns centre of poly shape if available, else centre of
		//bounding rectangle in world space
		sf::Vector2f calcCentre() const;
		//precomputes centre point and furthest point to be used in intersection testing
		void calcTestValues();
		//creates an AABB around the object based on its polygonal points, in world space
		void createAABB();
	};
	using MapObjects =  std::vector<MapObject>;

	//represents a single tile on a layer
	struct TMX_EXPORT_API MapTile final
	{
		//returns the base centre point of sprite / tile
		sf::Vector2f getBase() const
		{
			return sf::Vector2f(sprite.getPosition().x + (sprite.getLocalBounds().width / 2.f),
				sprite.getPosition().y + sprite.getLocalBounds().height);
		}
		sf::Sprite sprite;
		sf::Vector2i gridCoord;
		sf::RenderStates renderStates; //used to perform any rendering with custom shaders or blend mode
	};
	using MapTiles =  std::vector<MapTile>;
};

#endif //MAP_OBJECT_HPP_
