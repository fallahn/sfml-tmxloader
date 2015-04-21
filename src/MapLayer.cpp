/*********************************************************************
Matt Marchant 2013 - 2014
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

#include <tmx/MapLayer.h>

using namespace tmx;
///------TileQuad-----///
TileQuad::TileQuad(sf::Uint16 i0, sf::Uint16 i1, sf::Uint16 i2, sf::Uint16 i3)
	: m_parentSet	(nullptr)
{
	m_indices[0] = i0;
	m_indices[1] = i1;
	m_indices[2] = i2;
	m_indices[3] = i3;
}

void TileQuad::Move(const sf::Vector2f& distance)
{
	m_movement = distance;
	if(m_parentSet)
	{
		m_parentSet->m_dirtyQuads.push_back(this);
	}
}


///------LayerSet-----///

//public
LayerSet::LayerSet(const sf::Texture& texture)
	: m_texture	(texture),
	m_visible	(true)
{

}

TileQuad* LayerSet::AddTile(sf::Vertex vt0, sf::Vertex vt1, sf::Vertex vt2, sf::Vertex vt3)
{
	m_vertices.push_back(vt0);
	m_vertices.push_back(vt1);
	m_vertices.push_back(vt2);
	m_vertices.push_back(vt3);

	sf::Uint16 i = m_vertices.size() - 4u;
	m_quads.emplace_back(TileQuad::Ptr(new TileQuad(i, i + 1, i + 2, i + 3)));
	m_quads.back()->m_parentSet = this;

	m_UpdateAABB(vt0.position, vt2.position);

	return m_quads.back().get();
}

void LayerSet::Cull(const sf::FloatRect& bounds)
{
	m_visible = m_boundingBox.intersects(bounds);
}

//private
void LayerSet::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
	for(const auto& q : m_dirtyQuads)
	{
		for(const auto& p : q->m_indices)
		{
			m_vertices[p].position += q->m_movement;
		}
	}
	m_dirtyQuads.clear();

	if(!m_vertices.empty() && m_visible)
	{
		states.texture = &m_texture;
		rt.draw(&m_vertices[0], static_cast<unsigned int>(m_vertices.size()), sf::Quads, states);
	}
}

void LayerSet::m_UpdateAABB(sf::Vector2f position, sf::Vector2f size)
{
	if(m_boundingBox.width == 0.f)
	{
		//not been set yet so take on initial size
		m_boundingBox = sf::FloatRect(position, size);
		return;
	}

	if(position.x < m_boundingBox.left)
		m_boundingBox.left = position.x;

	if(position.y < m_boundingBox.top)
		m_boundingBox.top = position.y;

	if(size.x > m_boundingBox.left + m_boundingBox.width)
		m_boundingBox.width = size.x - m_boundingBox.left;

	if(size.y > m_boundingBox.top + m_boundingBox.height)
		m_boundingBox.height = size.y - m_boundingBox.top;
}

///------MapLayer-----///

//public
MapLayer::MapLayer(MapLayerType type)
	: opacity	(1.f),
	visible		(true),
	type		(type),
	m_shader	(nullptr)
{}

void MapLayer::SetShader(const sf::Shader& shader)
{
	m_shader = &shader;
}

void MapLayer::Cull(const sf::FloatRect& bounds)
{
	for(auto& ls : layerSets)
		ls.second->Cull(bounds);
}

//private
void MapLayer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
	if(!visible) return; //skip invisible layers

	states.shader = m_shader;
	for(const auto& ls : layerSets)
	{
		rt.draw(*ls.second, states);
	}

	if(type == ImageLayer)
	{
		//draw tiles used on objects
		for(const auto& tile : tiles)
		{
			rt.draw(tile.sprite, tile.renderStates);
		}
	}
}