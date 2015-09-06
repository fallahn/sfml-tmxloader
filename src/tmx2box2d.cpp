/*********************************************************************
Matt Marchant 2013
SFML Tiled Map Loader - https://github.com/bjorn/tiled/wiki/TMX-Map-Format
http://trederia.blogspot.com/2013/05/tiled-map-loader-for-sfml.html

Utility class for creating box2D bodies from tmx map objects. You
can optionally include this file and tmx2box2d.cpp in your project if
you plan to use box2d - http://www.box2d.org for processing physics.

Some of the hull decimation code is a based on an ActionScript class
by Antoan Angelov. See:
http://www.emanueleferonato.com/2011/09/12/create-non-convex-complex-shapes-with-box2d/

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

#include <tmx/tmx2box2d.h>

#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Collision/Shapes/b2CircleShape.h>
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <Box2D/Collision/Shapes/b2EdgeShape.h>
#include <Box2D/Collision/Shapes/b2ChainShape.h>

#include <cassert>
#include <array>

namespace
{
	//this scales tmx units into box2D metres
	//value is units per metre.
	const float worldScale = 100.f;
	//used in point calcs. Don't change this.
	const float pointTolerance = 0.1f;
}

//---------------------------------------------
b2Vec2 tmx::SfToBoxVec(const sf::Vector2f& vec)
{
	return b2Vec2(vec.x / worldScale, -vec.y / worldScale);
}

sf::Vector2f tmx::BoxToSfVec(const b2Vec2& vec)
{
	return sf::Vector2f(vec.x, -vec.y) * worldScale;
}

float tmx::SfToBoxFloat(float val)
{
	return val / worldScale;
}

float tmx::BoxToSfFloat(float val)
{
	return val * worldScale;
}

float tmx::SfToBoxAngle(float degrees)
{
	return -degrees * 0.0174533f;
}

float tmx::BoxToSfAngle(float rads)
{
	return -rads * 57.29578f;
}

//----------------------------------------------

using namespace tmx;

//public
b2Body* BodyCreator::Add(const MapObject& object, b2World& world, b2BodyType bodyType)
{
	assert(object.PolyPoints().size() > 1u);

	b2BodyDef bodyDef;
	bodyDef.type = bodyType;

	b2Body* body = nullptr;

	MapObjectShape shapeType = object.GetShapeType();
	sf::Uint16 pointCount = object.PolyPoints().size();

	if (shapeType == Polyline || pointCount < 3)
	{
		bodyDef.position.Set(0.f, 0.f);
		bodyDef.position = SfToBoxVec(object.GetPosition());
		body = world.CreateBody(&bodyDef);

		const Shape& shape = object.PolyPoints();
		b2FixtureDef f;
		f.density = 1.f;

		if (pointCount < 3)
		{
			b2EdgeShape es;
			es.Set(SfToBoxVec(shape[0]), SfToBoxVec(shape[1]));
			f.shape = &es;
			body->CreateFixture(&f);
		}
		else
		{
			std::unique_ptr<b2Vec2[]> vertices(new b2Vec2[pointCount]);
			for (auto i = 0u; i < pointCount; i++)
				vertices[i] = SfToBoxVec(shape[i]);

			b2ChainShape cs;
			cs.CreateChain(vertices.get(), pointCount);
			f.shape = &cs;
			body->CreateFixture(&f);
		}
	}
	else if (shapeType == Circle ||
		(object.Convex() && pointCount <= b2_maxPolygonVertices))
	{
		b2FixtureDef f;
		f.density = 1.f;
		bodyDef.position = SfToBoxVec(object.GetCentre());
		if (shapeType == Circle)
		{
			//make a circle
			b2CircleShape c;
			c.m_radius = SfToBoxFloat(object.GetAABB().width / 2.f);
			f.shape = &c;

			body = world.CreateBody(&bodyDef);
			body->CreateFixture(&f);
		}
		else if (shapeType == Rectangle)
		{
			b2PolygonShape ps;
			ps.SetAsBox(SfToBoxFloat(object.GetAABB().width / 2.f), SfToBoxFloat(object.GetAABB().height / 2.f));
			f.shape = &ps;

			body = world.CreateBody(&bodyDef);
			body->CreateFixture(&f);
		}
		else //simple convex polygon
		{
			bodyDef.position.Set(0.f, 0.f);
			bodyDef.position = SfToBoxVec(object.GetPosition());
			body = world.CreateBody(&bodyDef);

			const Shape& points = object.PolyPoints();
			m_CreateFixture(points, body);
		}
	}
	else //complex polygon
	{
		//break into smaller convex shapes
		bodyDef.position.Set(0.f, 0.f);
		bodyDef.position = SfToBoxVec(object.GetPosition());
		body = world.CreateBody(&bodyDef);

		m_Split(object, body);
	}

	return body;
}

//private
void BodyCreator::m_Split(const MapObject& object, b2Body* body)
{
	//check object shapes is valid
	if (!m_CheckShape(const_cast<MapObject&>(object))) return;

	const Shape& points = object.PolyPoints();
	Shapes shapes;
	if (object.Convex())
		shapes = m_ProcessConvex(points);
	else
		shapes = m_ProcessConcave(points);

	for (auto& shape : shapes)
	{
		sf::Uint16 s = shape.size();

		if (s > b2_maxPolygonVertices)
		{
			//break into smaller and append
			Shapes moreShapes = m_ProcessConvex(shape);
			for (auto& anotherShape : moreShapes)
				m_CreateFixture(anotherShape, body);

			continue; //skip shape because it's too big
		}
		m_CreateFixture(shape, body);
	}
}

BodyCreator::Shapes BodyCreator::m_ProcessConvex(const Shape& points)
{
	assert(points.size() > b2_maxPolygonVertices);
	Shapes output;
	const sf::Uint16 maxVerts = b2_maxPolygonVertices - 2u;
	const sf::Uint16 shapeCount = static_cast<sf::Uint16>(std::floor(points.size() / maxVerts));
	const sf::Uint16 s = points.size();

	for (auto i = 0u; i <= shapeCount; i++)
	{
		Shape shape;
		sf::Uint16 total = 0u;
		for (auto j = 0u; j < maxVerts; j++)
		{
			sf::Uint16 index = i * maxVerts + j;
			if (i) index--;
			if (index < s)
				shape.push_back(points[index]);
			else
				break;
		}
		output.push_back(shape);
	}


	//join ends
	Shape& shape = output[output.size() - 2];
	output.back().push_back(output.front().front());
	output.back().push_back(shape.back());

	return output;
}

BodyCreator::Shapes BodyCreator::m_ProcessConcave(const Shape& points)
{
	ShapeQueue q;
	q.push(points);

	Shapes output;

	while (!q.empty())
	{
		const auto& shape = q.front();
		sf::Uint16 s = shape.size();
		bool convex = true;

		for (auto i = 0u; i < s; i++)
		{
			sf::Uint16 i1 = i;
			sf::Uint16 i2 = (i < s - 1u) ? i + 1u : i + 1u - s;
			sf::Uint16 i3 = (i < s - 2u) ? i + 2u : i + 2u - s;

			sf::Vector2f p1 = shape[i1];
			sf::Vector2f p2 = shape[i2];
			sf::Vector2f p3 = shape[i3];

			float direction = m_GetWinding(p1, p2, p3);
			if (direction < 0.f)
			{
				convex = false;
				float minLen = FLT_MAX;
				sf::Int16 j1 = 0;
				sf::Int16 j2 = 0;
				sf::Vector2f v1;
				sf::Vector2f v2;
				sf::Int16 h = 0;
				sf::Int16 k = 0;
				sf::Vector2f hitPoint;

				for (auto j = 0u; j < s; j++)
				{
					if (j != i1 && j != i2)
					{
						j1 = j;
						j2 = (j < s - 1u) ? j + 1u : 0;

						v1 = shape[j1];
						v2 = shape[j2];

						sf::Vector2f hp = m_HitPoint(p1, p2, v1, v2);
						bool b1 = m_OnSeg(p2, p1, hp);
						bool b2 = m_OnSeg(hp, v1, v2);

						if (b1 && b2)
						{
							sf::Vector2f dist = p2 - hp;

							float t = Helpers::Vectors::dot(dist, dist); //aka length squared

							if (t < minLen)
							{
								h = j1;
								k = j2;
								hitPoint = hp;
								minLen = t;
							}
						}
					}
				}

				assert(minLen != FLT_MAX);

				Shape shape1;
				Shape shape2;

				j1 = h;
				j2 = k;
				v1 = shape[j1];
				v2 = shape[j2];

				if (!m_PointsMatch(hitPoint, v2))
					shape1.push_back(hitPoint);

				if (!m_PointsMatch(hitPoint, v1))
					shape2.push_back(hitPoint);


				h = -1;
				k = i1;
				while (true)
				{
					if (k != j2)
					{
						shape1.push_back(shape[k]);
					}
					else
					{
						assert(h >= 0 && h < s);
						if (!m_OnSeg(v2, shape[h], p1))
						{
							shape1.push_back(shape[k]);
						}
						break;
					}

					h = k;
					if ((k - 1) < 0)
					{
						k = s - 1;
					}
					else
					{
						k--;
					}
				}

				std::reverse(shape1.begin(), shape1.end());

				h = -1;
				k = i2;
				while (true)
				{
					if (k != j1)
					{
						shape2.push_back(shape[k]);
					}
					else
					{
						assert(h >= 0 && h < s);
						if ((k == j1) && !m_OnSeg(v1, shape[h], p2))
						{
							shape2.push_back(shape[k]);
						}
						break;
					}

					h = k;
					if (k + 1 > s - 1)
					{
						k = 0;
					}
					else
					{
						k++;
					}
				}
				q.push(shape1);
				q.push(shape2);
				q.pop();

				break;
			}
		}

		if (convex)
		{
			output.push_back(q.front());
			q.pop();
		}
	}
	return output;
}

sf::Vector2f BodyCreator::m_HitPoint(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3, const sf::Vector2f& p4)
{
	sf::Vector2f g1 = p3 - p1;
	sf::Vector2f g2 = p2 - p1;
	sf::Vector2f g3 = p4 - p3;

	float t = Helpers::Vectors::cross(g3, g2);
	float a = Helpers::Vectors::cross(g3, g1) / t;

	return sf::Vector2f(p1.x + a * g2.x, p1.y + a * g2.y);
}

bool BodyCreator::m_OnLine(const sf::Vector2f& p, const sf::Vector2f& start, const sf::Vector2f& end)
{
	if (end.x - start.x > pointTolerance || start.x - end.x > pointTolerance)
	{
		float a = (end.y - start.y) / (end.x - start.x);
		float newY = a * (p.x - start.x) + start.y;
		float diff = std::abs(newY - p.y);
		return (diff < pointTolerance);
	}

	return (p.x - start.x < pointTolerance || start.x - p.x < pointTolerance);
}

bool BodyCreator::m_OnSeg(const sf::Vector2f& p, const sf::Vector2f& start, const sf::Vector2f& end)
{
	bool a = (start.x + pointTolerance >= p.x && p.x >= end.x - pointTolerance)
		|| (start.x - pointTolerance <= p.x && p.x <= end.x + pointTolerance);
	bool b = (start.y + pointTolerance >= p.y && p.y >= end.y - pointTolerance)
		|| (start.y - pointTolerance <= p.y && p.y <= end.y + pointTolerance);
	return ((a && b) && m_OnLine(p, start, end));
}

bool BodyCreator::m_PointsMatch(const sf::Vector2f& p1, const sf::Vector2f& p2)
{
	float x = std::abs(p2.x - p1.x);
	float y = std::abs(p2.y - p1.y);
	return (x < pointTolerance && y < pointTolerance);
}

float BodyCreator::m_GetWinding(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3)
{
	return p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p1.y * p2.x - p2.y * p3.x - p3.y * p1.x;
}

void BodyCreator::m_CreateFixture(const Shape& points, b2Body* body)
{
	bool clockwise = (m_GetWinding(points[0], points[1], points[2]) > 0.f);
	sf::Uint16 s = points.size();
	std::unique_ptr<b2Vec2[]> vertices(new b2Vec2[s]);
	for (auto i = 0u; i < s; i++)
	{
		//reverse direction if verts wound clockwise
		int index = (clockwise) ? (s - 1) - i : i;
		vertices[i] = SfToBoxVec(points[index]);
	}

	b2PolygonShape ps;
	ps.Set(vertices.get(), points.size());
	b2FixtureDef f;
	f.density = 1.f;
	f.shape = &ps;

	body->CreateFixture(&f);
}

bool BodyCreator::m_CheckShape(MapObject& object)
{
	const Shape& points = object.PolyPoints();
	sf::Uint16 s = points.size();
	bool b1 = false;
	bool b2 = false;

	for (auto i = 0; i < s; i++)
	{
		sf::Int16 i2 = (i < s - 1) ? i + 1 : 0;
		sf::Int16 i3 = (i > 0) ? i - 1 : s - 1;

		for (auto j = 0; j < s; j++)
		{
			if (j != i && j != i2)
			{
				if (!b1)
				{
					float direction = m_GetWinding(points[i], points[i2], points[j]);
					if (direction > 0.f) b1 = true;
				}

				if (j != i3)
				{
					sf::Int16 j2 = (j < s - 1) ? j + 1 : 0;
					sf::Vector2f hp = m_HitPoint(points[i], points[i2], points[j], points[j2]);
					if (m_OnSeg(hp, points[i], points[i2])
						&& m_OnSeg(hp, points[j], points[j2]))
					{
						std::cerr << "Object " << object.GetName() << " found with possible self intersection." << std::endl;
						std::cerr << "Skipping body creation." << std::endl;
						return false;
					}
				}
			}
		}
		if (!b1) b2 = true;
	}

	if (b2)
	{
		std::cerr << "Object " << object.GetName() << " vertices are wound in the wrong direction." << std::endl;
		std::cerr << "Attempting to fix..." << std::endl;
		object.ReverseWinding();
	}
	return true;
}
