/*********************************************************************
Matt Marchant 2013 - 2015
SFML Tiled Map Loader - https://github.com/bjorn/tiled/wiki/TMX-Map-Format
						http://trederia.blogspot.com/2013/05/tiled-map-loader-for-sfml.html

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



///source for QuadTreeNode class///
#include <tmx/QuadTreeNode.h>

using namespace tmx;

QuadTreeNode::QuadTreeNode(sf::Uint16 level, const sf::FloatRect& bounds)
	: MAX_OBJECTS	(5u),
	MAX_LEVELS		(5u),
	m_level			(level),
	m_bounds		(bounds)
{ 
	m_children.reserve(4); 
}

//public functions//
void QuadTreeRoot::Clear(const sf::FloatRect& newBounds)
{
	m_objects.clear();
	m_children.clear();
	m_bounds = newBounds;

	m_searchDepth = 0u;
	m_depth = 0;
}

std::vector<MapObject*> QuadTreeNode::Retrieve(const sf::FloatRect& bounds, sf::Uint16& searchDepth)
{	
	searchDepth = m_level;
	std::vector<MapObject*> foundObjects;
	sf::Int16 index = GetIndex(bounds);

	//recursively add objects of child node if bounds is fully contained
	if(!m_children.empty() && index != -1) 
	{
		foundObjects = m_children[index]->Retrieve(bounds, searchDepth);
	}
	else
	{
		//add all objects of child nodes which intersect test area
		for(auto& child : m_children)
		{
			if(bounds.intersects(child->m_bounds))
			{
				std::vector<MapObject*> childObjects = child->Retrieve(bounds, searchDepth);
				foundObjects.insert(foundObjects.end(), childObjects.begin(), childObjects.end());
			}
		}

	}
	//and append objects in this node
	foundObjects.insert(foundObjects.end(), m_objects.begin(), m_objects.end());

	return foundObjects;
}

void QuadTreeNode::Insert(const MapObject& object)
{	
	//check if an object falls completely outside a node
	if(!object.GetAABB().intersects(m_bounds)) return;
	
	//if node is already split add object to corresponding child node
	//if it fits
	if(!m_children.empty())
	{
		sf::Int16 index = GetIndex(object.GetAABB());
		if(index != -1)
		{
			m_children[index]->Insert(object);
			return;
		}
	}
	//else add object to this node
	m_objects.push_back(const_cast<MapObject*>(&object));


	//check number of objects in this node, and split if necessary
	//adding any objects that fit to the new child node
	if(m_objects.size() > MAX_OBJECTS && m_level < MAX_LEVELS)
	{
		//split if there are no child nodes
		if(m_children.empty()) Split();

		sf::Uint16 i = 0;
		while(i < m_objects.size())
		{
			sf::Int16 index = GetIndex(m_objects[i]->GetAABB());
			if(index != -1)
			{
				m_children[index]->Insert(*m_objects[i]);
				m_objects.erase(m_objects.begin() + i);
			}
			else
			{
				i++; //we only increment i when not erasing, because erasing moves
				//everything up one index inside the vector
			}
		}
	}
}

void QuadTreeNode::GetVertices(std::vector<sf::Vertex>& vertices)
{
    sf::Color colour = sf::Color::Green;
    vertices.emplace_back(sf::Vector2f(m_bounds.left, m_bounds.top), sf::Color::Transparent); //pad with breaks between quads
    vertices.emplace_back(sf::Vector2f(m_bounds.left, m_bounds.top), colour);
    vertices.emplace_back(sf::Vector2f(m_bounds.left + m_bounds.width, m_bounds.top), colour);
    vertices.emplace_back(sf::Vector2f(m_bounds.left + m_bounds.width, m_bounds.top + m_bounds.height), colour);
    vertices.emplace_back(sf::Vector2f(m_bounds.left, m_bounds.top + m_bounds.height), colour);
    vertices.emplace_back(sf::Vector2f(m_bounds.left, m_bounds.top), colour);
    vertices.emplace_back(sf::Vector2f(m_bounds.left, m_bounds.top), sf::Color::Transparent);

    for (const auto& c : m_children)
        c->GetVertices(vertices);
}


//private functions//
void QuadTreeRoot::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    std::vector<sf::Vertex> verts;
    for (const auto& c : m_children)
        c->GetVertices(verts);

    rt.draw(verts.data(), verts.size(), sf::PrimitiveType::LinesStrip);
}


void QuadTreeNode::Split(void)
{
	const float halfWidth = m_bounds.width / 2.f;
	const float halfHeight = m_bounds.height / 2.f;
	const float x = m_bounds.left;
	const float y = m_bounds.top;
 
	m_children.push_back(std::make_shared<QuadTreeNode>(m_level + 1, sf::FloatRect(x + halfWidth, y, halfWidth, halfHeight)));
	m_children.push_back(std::make_shared<QuadTreeNode>(m_level + 1, sf::FloatRect(x, y, halfWidth, halfHeight)));
	m_children.push_back(std::make_shared<QuadTreeNode>(m_level + 1, sf::FloatRect(x, y + halfHeight, halfWidth, halfHeight)));
	m_children.push_back(std::make_shared<QuadTreeNode>(m_level + 1, sf::FloatRect(x+ halfWidth, y + halfHeight, halfWidth, halfHeight)));
}

sf::Int16 QuadTreeNode::GetIndex(const sf::FloatRect& bounds)
{
	sf::Int16 index = -1;
	float verticalMidpoint = m_bounds.left + (m_bounds.width / 2.f);
	float horizontalMidpoint = m_bounds.top + (m_bounds.height / 2.f);
 
	//Object can completely fit within the top quadrants
	bool topQuadrant = (bounds.top < horizontalMidpoint && bounds.top + bounds.height < horizontalMidpoint);
	//Object can completely fit within the bottom quadrants
	bool bottomQuadrant = (bounds.top > horizontalMidpoint);
 
	//Object can completely fit within the left quadrants
	if(bounds.left < verticalMidpoint && bounds.left + bounds.width < verticalMidpoint)
	{
		if(topQuadrant)
		{
			index = 1;
		}
		else if(bottomQuadrant)
		{
			index = 2;
		}
	}
	//Object can completely fit within the right quadrants
	else if(bounds.left > verticalMidpoint)
	{
		if(topQuadrant)
		{
			index = 0;
		}
		else if(bottomQuadrant) 
		{
			index = 3;
		}
	}
	return index;
}