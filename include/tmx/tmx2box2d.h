/*********************************************************************
Matt Marchant 2013 - 2015
SFML Tiled Map Loader - https://github.com/bjorn/tiled/wiki/TMX-Map-Format
http://trederia.blogspot.com/2013/05/tiled-map-loader-for-sfml.html

Utility class for creating box2D bodies from tmx map objects. You
can optionally include this file and tmx2box2d.cpp in your project if
you plan to use box2d - http://www.box2d.org for processing physics.

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

#ifndef TMX_BOX2D_H_
#define TMX_BOX2D_H_

#include <tmx/MapObject.h>

#include <Box2D/Dynamics/b2World.h>
#include <Box2D/Common/b2Math.h>
#include <Box2D/Dynamics/b2Fixture.h>

#include <vector>
#include <queue>

namespace tmx
{
	b2Vec2 SfToBoxVec(const sf::Vector2f& vec);
	sf::Vector2f BoxToSfVec(const b2Vec2& vec);
	float SfToBoxFloat(float val);
	float BoxToSfFloat(float val);
	float SfToBoxAngle(float degrees);
	float BoxToSfAngle(float rads);

	class BodyCreator
	{
	public:
		typedef std::vector<sf::Vector2f> Shape;
		typedef std::vector<Shape> Shapes;
		typedef std::queue<Shape> ShapeQueue;

		//adds the object to the b2World. Returns a pointer to the body
		//created so that its properties my be modified. Bodies are static by default
		static b2Body* Add(const MapObject& object, b2World& world, b2BodyType bodyType = b2_staticBody);

	private:
		static void m_Split(const MapObject& object, b2Body* body);
		static Shapes m_ProcessConcave(const Shape& points);
		static Shapes m_ProcessConvex(const Shape& points);
		static sf::Vector2f m_HitPoint(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3, const sf::Vector2f& p4);
		static bool m_OnLine(const sf::Vector2f& p, const sf::Vector2f& start, const sf::Vector2f& end);
		static bool m_OnSeg(const sf::Vector2f& p, const sf::Vector2f& start, const sf::Vector2f& end);
		static bool m_PointsMatch(const sf::Vector2f& p1, const sf::Vector2f& p2);
		static float m_GetWinding(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3);
		static void m_CreateFixture(const Shape& points, b2Body* body);
		static bool m_CheckShape(MapObject& object);
	};
}

#endif