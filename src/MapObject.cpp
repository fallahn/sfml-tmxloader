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
#include <tmx/MapObject.hpp>
#include <tmx/MapLayer.hpp>
#include <tmx/Log.hpp>

using namespace tmx;

///--------poly segment--------///
bool MapObject::Segment::intersects(const MapObject::Segment& segment)
{
	sf::Vector2f s1 = End - Start;
	sf::Vector2f s2 = segment.End - segment.Start;

	float s, t;
	s = (-s1.y * (Start.x - segment.Start.x) + s1.x * (Start.y - segment.Start.y)) / (-s2.x * s1.y + s1.x * s2.y);
	t = ( s2.x * (Start.y - segment.Start.y) - s2.y * (Start.x - segment.Start.x)) / (-s2.x * s1.y + s1.x * s2.y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		// Collision detected

		//point at which lines intersect - do what you will with this
		sf::Vector2f intersection(Start.x + (t * s1.x), Start.y + (t * s1.y));
		return true;
	}
	return false;
}



///------map object-------///

//ctor
MapObject::MapObject()
	 : m_visible	(true),
	 m_shape		(Rectangle),
	 m_tileQuad		(nullptr),
	 m_furthestPoint(0.f)
{

}

//public
std::string MapObject::getPropertyString(const std::string& name)
{
    if (m_properties.find(name) != m_properties.end())
    {
        return m_properties[name];
    }
    else
    {
        return std::string();
    }
}

void MapObject::setProperty(const std::string& name, const std::string& value)
{
	m_properties[name] = value;
}

void MapObject::setVisible(bool visible)
{
    m_visible = visible;
    if (m_tileQuad)
    {
        m_tileQuad->setVisible(visible);
    }
}

bool MapObject::contains(sf::Vector2f point) const
{
	if(m_shape == Polyline) return false;

	//check if enough poly points
	if(m_polypoints.size() < 3) return false;

	//else raycast through points - assuming given point is in world coords
    const auto& transform = getTransform();
	unsigned int i, j;
	bool result = false;
	for (i = 0, j = m_polypoints.size() - 1; i < m_polypoints.size(); j = i++)
	{
        sf::Vector2f pointI = transform.transformPoint(m_polypoints[i]);
        sf::Vector2f pointJ = transform.transformPoint(m_polypoints[j]);
        
        if (((pointI.y > point.y) != (pointJ.y > point.y)) &&
            (point.x < (pointJ.x - pointI.x) * (point.y - pointI.y)
            / (pointJ.y - pointI.y) + pointI.x))
				result = !result;
	}
	return result;
}

bool MapObject::intersects(const MapObject& object) const
{
	//check if distance between objects is less than sum of furthest points
	float distance = Helpers::Vectors::getLength(m_centrePoint + object.m_centrePoint);
	if(distance > (m_furthestPoint + object.m_furthestPoint)) return false;

	//check intersection if either object contains a point of the other
    const auto& otherTransform = object.getTransform();
	for(auto& p : object.m_polypoints)
		if(contains(otherTransform.transformPoint(p))) return true;

    const auto& transform = getTransform();
	for(auto& p : m_polypoints)
		if(object.contains(transform.transformPoint(p))) return true;

	return false;
}

void MapObject::createDebugShape(const sf::Color& colour)
{
	if(m_polypoints.size() == 0)
	{
		LOG("Unable to create debug shape, object data missing.", Logger::Type::Error);
		LOG("Check image file paths referenced by tmx file.", Logger::Type::Error);
		return;
	}

	//reset any existing shapes incase new points have been added
	m_debugShape.reset();

	for(const auto& p : m_polypoints)
		m_debugShape.addVertex(sf::Vertex(p, colour));
	
	if(m_shape != Polyline) m_debugShape.closeShape();

	//precompute shape values for intersection testing
	calcTestValues();

	//create the AABB for quad tree testing
	createAABB();
}

void MapObject::drawDebugShape(sf::RenderTarget& rt) const
{
	rt.draw(m_debugShape, getTransform());
}

sf::Vector2f MapObject::firstPoint() const
{
    if (!m_polypoints.empty())
    {
        return getTransform().transformPoint(m_polypoints[0]);
    }
    else
    {
        return sf::Vector2f();
    }
}

sf::Vector2f MapObject::lastPoint() const
{
    if (!m_polypoints.empty())
    {
return(getTransform().transformPoint(m_polypoints.back()));
    }
    else
    {
        return sf::Vector2f();
    }
}

sf::Vector2f MapObject::collisionNormal(const sf::Vector2f& start, const sf::Vector2f& end) const
{
    Segment trajectory(start, end);
    for (auto& s : m_polySegs)
    {
        if (trajectory.intersects(s))
        {
            sf::Vector2f v = s.End - s.Start;
            sf::Vector2f n(v.y, -v.x);
            //invert normal if pointing in wrong direction
            float tAngle = Helpers::Vectors::getAngle(end - start);
            float nAngle = Helpers::Vectors::getAngle(n);
            if (nAngle - tAngle > 90.f) n = -n;

            return Helpers::Vectors::normalize(n);
        }
    }
    sf::Vector2f rv(end - start);
    return Helpers::Vectors::normalize(rv);
}

void MapObject::createSegments()
{
    if (m_polypoints.size() == 0)
    {
        LOG("Unable to object segments, object data missing.", Logger::Type::Error);
        LOG("Check image file paths referenced by tmx file.", Logger::Type::Error);
        return;
    }

    for (auto i = 0u; i < m_polypoints.size() - 1; i++)
    {
        m_polySegs.push_back(Segment(m_polypoints[i], m_polypoints[i + 1]));
    }
    if (m_shape != Polyline) //close shape
        m_polySegs.push_back(Segment(*(m_polypoints.end() - 1), *m_polypoints.begin()));

    LOG("Added " + std::to_string(m_polySegs.size()) + " segments to Map Object", Logger::Type::Info);
}

bool MapObject::convex() const
{
    if (m_shape == MapObjectShape::Polyline)
        return false;

    bool negative = false;
    bool positive = false;

    sf::Uint16 a, b, c, n = m_polypoints.size();
    for (a = 0u; a < n; ++a)
    {
        b = (a + 1) % n;
        c = (b + 1) % n;

        float cross = Helpers::Vectors::cross(m_polypoints[a], m_polypoints[b], m_polypoints[c]);

        if (cross < 0.f)
            negative = true;
        else if (cross > 0.f)
            positive = true;
        if (positive && negative) return false;
    }
    return true;
}

const std::vector<sf::Vector2f>& MapObject::polyPoints()const
{
    return m_polypoints;
}

void MapObject::reverseWinding()
{
    std::reverse(m_polypoints.begin(), m_polypoints.end());
}

void MapObject::setQuad(TileQuad* quad)
{
    m_tileQuad = quad;
    m_tileQuad->setVisible(visible());
}

//note we have these versions of the movement
//functions so drawables can be updated accordingly
void MapObject::setPosition(float x, float y)
{
    auto oldPos = getPosition();
    
    sf::Transformable::setPosition(x, y);
    if (m_tileQuad)
    {
        m_tileQuad->move({ x - oldPos.x, y - oldPos.y });
    }
}

void MapObject::setPosition(const sf::Vector2f& position)
{
    setPosition(position.x, position.y);
}

void MapObject::move(float x, float y)
{
    sf::Transformable::move(x, y);
    if (m_tileQuad)
    {
        m_tileQuad->move({ x, y });
    }
}

void MapObject::move(const sf::Vector2f& amount)
{
    move(amount.x, amount.y);
}

//private
sf::Vector2f MapObject::calcCentre() const
{
	if(m_shape == Polyline) return sf::Vector2f();

	if(m_shape == Rectangle || m_polypoints.size() < 3)
	{
		//we don't have a triangle so use bounding box
		return sf::Vector2f(getPosition().x + (m_size.x / 2.f), getPosition().y + (m_size.y / 2.f));
	}
	//calc centroid of poly shape
	sf::Vector2f centroid;
	float signedArea = 0.f;
	float x0 = 0.f; // Current vertex X
	float y0 = 0.f; // Current vertex Y
	float x1 = 0.f; // Next vertex X
	float y1 = 0.f; // Next vertex Y
	float a = 0.f;  // Partial signed area

	// For all vertices except last
	unsigned i;
	for(i = 0; i < m_polypoints.size() - 1; ++i)
	{
		x0 = m_polypoints[i].x;
		y0 = m_polypoints[i].y;
		x1 = m_polypoints[i + 1].x;
		y1 = m_polypoints[i + 1].y;
		a = x0 * y1 - x1 * y0;
		signedArea += a;
		centroid.x += (x0 + x1) * a;
		centroid.y += (y0 + y1) * a;
	}

	// Do last vertex
	x0 = m_polypoints[i].x;
	y0 = m_polypoints[i].y;
	x1 = m_polypoints[0].x;
	y1 = m_polypoints[0].y;
	a = x0 * y1 - x1 * y0;
	signedArea += a;
	centroid.x += (x0 + x1) * a;
	centroid.y += (y0 + y1) * a;

	signedArea *= 0.5;
	centroid.x /= (6 * signedArea);
	centroid.y /= (6 * signedArea);

	//convert to world space
    centroid = getTransform().transformPoint(centroid);
	return centroid;
}

void MapObject::calcTestValues()
{
	m_centrePoint = calcCentre();
	for(auto i = m_polypoints.cbegin(); i != m_polypoints.cend(); ++i)
	{
		float length = Helpers::Vectors::getLength(*i - m_centrePoint);
		if(m_furthestPoint < length)
		{
			m_furthestPoint = length;
			if(m_shape == Polyline) m_centrePoint = *i / 2.f;
		}
	}
	//polyline centre ought to be half way between the start point and the furthest vertex
	if(m_shape == Polyline) m_furthestPoint /= 2.f;
}

void MapObject::createAABB()
{
	if(!m_polypoints.empty())
	{
		m_AABB = sf::FloatRect(m_polypoints[0], m_polypoints[0]);
		for(auto point = m_polypoints.cbegin(); point != m_polypoints.cend(); ++point)
		{
			if(point->x < m_AABB.left) m_AABB.left = point->x;
			if(point->x > m_AABB.width) m_AABB.width = point->x;
			if(point->y < m_AABB.top) m_AABB.top = point->y;
			if(point->y > m_AABB.height) m_AABB.height = point->y;
		}

		//calc true width and height by distance between points
		m_AABB.width -= m_AABB.left;
		m_AABB.height -= m_AABB.top;

		//debug draw AABB
		//m_debugShape.append(sf::Vector2f(m_AABB.left, m_AABB.top));
		//m_debugShape.append(sf::Vector2f(m_AABB.left + m_AABB.width, m_AABB.top));
		//m_debugShape.append(sf::Vector2f(m_AABB.left + m_AABB.width, m_AABB.top + m_AABB.height));
		//m_debugShape.append(sf::Vector2f(m_AABB.left, m_AABB.top + m_AABB.height));
	}
}