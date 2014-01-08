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
	: m_needsUpdate	(false)
{
	m_indices[0] = i0;
	m_indices[1] = i1;
	m_indices[2] = i2;
	m_indices[3] = i3;
}

void TileQuad::Move(const sf::Vector2f& distance)
{
	m_movement = distance;
	m_needsUpdate = true;
}


///------LayerSet-----///

//public
LayerSet::LayerSet(const sf::Texture& texture)
	: m_texture	(texture)
{

}

TileQuad::Ptr LayerSet::AddTile(sf::Vertex vt0, sf::Vertex vt1, sf::Vertex vt2, sf::Vertex vt3)
{
	m_vertices.push_back(vt0);
	m_vertices.push_back(vt1);
	m_vertices.push_back(vt2);
	m_vertices.push_back(vt3);

	sf::Uint16 i = m_vertices.size() - 4u;
	m_quads.push_back(std::make_shared<TileQuad>(i, i + 1, i + 2, i + 3));
	return m_quads.back();
}

//private
void LayerSet::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
	for(auto& q : m_quads)
	{
		if(q->m_needsUpdate)
		{
			for(auto& i : q->m_indices)
			{
				m_vertices[i].position += q->m_movement;
			}
			q->m_needsUpdate = false;
		}
	}
	
	if(!m_vertices.empty())
	{
		states.texture = &m_texture;
		rt.draw(&m_vertices[0], static_cast<unsigned int>(m_vertices.size()), sf::Quads, states);
	}
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

//private
void MapLayer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
	if(!visible) return; //skip invisible layers

	states.shader = m_shader;
	for(auto& ls : layerSets)
	{
		rt.draw(*ls.second, states);
	}

	if(type == ImageLayer)
	{
		//draw tiles used on objects
		for(auto& tile : tiles)
		{
			rt.draw(tile.sprite, tile.renderStates);
		}
	}
}