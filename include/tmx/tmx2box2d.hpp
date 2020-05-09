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

#ifndef TMX_BOX2D_HPP_
#define TMX_BOX2D_HPP_

#include <tmx/MapObject.hpp>

#include <box2d/b2_world.h>
#include <box2d/b2_math.h>
#include <box2d/b2_fixture.h>

#include <vector>
#include <queue>

namespace tmx
{
    /*!
    \brief Converts an sfml vector in world units to Box2D units
    */
    TMX_EXPORT_API b2Vec2 sfToBoxVec(const sf::Vector2f& vec);
    /*!
    \brief Converts a Box2D vector to sfml vector
    */
    TMX_EXPORT_API sf::Vector2f boxToSfVec(const b2Vec2& vec);
    /*!
    \brief Converts a float in sfml world units to Box2D world units
    */
    TMX_EXPORT_API float sfToBoxFloat(float val);
    /*!
    \brief Converts a float in Box2D world units to sfml world units
    */
    TMX_EXPORT_API float boxToSfFloat(float val);
    /*!
    \brief Converts an sfml angle in degrees to radians used by Box2D
    */
    TMX_EXPORT_API float sfToBoxAngle(float degrees);
    /*!
    \brief Converts a Box2D angle in radians to degrees used by sfml
    */
    TMX_EXPORT_API float boxToSfAngle(float rads);

    /*!
    \brief Utility class for creating Box2D bodies from tmx object data
    */
    class TMX_EXPORT_API BodyCreator final
	{
	public:
		using Shape = std::vector<sf::Vector2f>;
		using Shapes = std::vector<Shape>;
		using ShapeQueue = std::queue<Shape>;

		/*!
        \brief Adds the object to the b2World.
        Returns a pointer to the body created so that its properties my be modified. Bodies are static by default
        */
		static b2Body* add(const MapObject& object, b2World& world, const sf::Vector2u& tileSize, b2BodyType bodyType = b2_staticBody);

	private:
		static void split(const MapObject& object, b2Body* body);
		static Shapes processConcave(const Shape& points);
		static Shapes processConvex(const Shape& points);
		static sf::Vector2f hitPoint(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3, const sf::Vector2f& p4);
		static bool onLine(const sf::Vector2f& p, const sf::Vector2f& start, const sf::Vector2f& end);
		static bool onSeg(const sf::Vector2f& p, const sf::Vector2f& start, const sf::Vector2f& end);
		static bool pointsMatch(const sf::Vector2f& p1, const sf::Vector2f& p2);
		static float getWinding(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3);
		static void createFixture(const Shape& points, b2Body* body);
		static bool checkShape(MapObject& object);
	};
}

#endif //TMX_BOX2D_HPP_