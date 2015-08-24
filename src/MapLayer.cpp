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

#include <tmx/MapLayer.h>

using namespace tmx;
///------TileQuad-----///
TileQuad::TileQuad(sf::Uint16 i0, sf::Uint16 i1, sf::Uint16 i2, sf::Uint16 i3)
	: m_parentSet	(nullptr),
	m_patchIndex	(-1)
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
LayerSet::LayerSet(const sf::Texture& texture, sf::Uint8 patchSize, const sf::Vector2u& mapSize, const sf::Vector2u tileSize)
	: m_texture	(texture),
	m_patchSize	(patchSize),
	m_mapSize	(mapSize),
	m_tileSize	(tileSize),
	m_patchCount(std::ceil(static_cast<float>(mapSize.x) / patchSize), std::ceil(static_cast<float>(mapSize.y) / patchSize)),
	m_visible	(true)
{
	m_patches.resize(m_patchCount.x * m_patchCount.y);
}

TileQuad* LayerSet::AddTile(sf::Vertex vt0, sf::Vertex vt1, sf::Vertex vt2, sf::Vertex vt3, sf::Uint16 x, sf::Uint16 y)
{
	sf::Int32 patchX = static_cast<sf::Int32>(std::ceil(x / m_patchSize));
	sf::Int32 patchY = static_cast<sf::Int32>(std::ceil(y / m_patchSize));
	sf::Int32 patchIndex = m_patchCount.x * patchY + patchX;

	m_patches[patchIndex].push_back(vt0);
	m_patches[patchIndex].push_back(vt1);
	m_patches[patchIndex].push_back(vt2);
	m_patches[patchIndex].push_back(vt3);

	sf::Uint16 i = m_patches[patchIndex].size() - 4u;
	m_quads.emplace_back(TileQuad::Ptr(new TileQuad(i, i + 1, i + 2, i + 3)));
	m_quads.back()->m_parentSet = this;
	m_quads.back()->m_patchIndex = patchIndex;

	UpdateAABB(vt0.position, vt2.position);

	return m_quads.back().get();
}

void LayerSet::Cull(const sf::FloatRect& bounds)
{
	m_visible = m_boundingBox.intersects(bounds);

	//update visible patch indices
	m_visiblePatchStart.x = static_cast<int>(std::floor((bounds.left / m_tileSize.x) / m_patchSize));
	m_visiblePatchStart.y = static_cast<int>(std::floor((bounds.top / m_tileSize.y) / m_patchSize));
	if(m_visiblePatchStart.x < 0) m_visiblePatchStart.x = 0;
	if(m_visiblePatchStart.y < 0) m_visiblePatchStart.y = 0;

	m_visiblePatchEnd.x = static_cast<int>(std::ceil((bounds.width / m_tileSize.x) / m_patchSize));
	m_visiblePatchEnd.y = static_cast<int>(std::ceil((bounds.height / m_tileSize.y) / m_patchSize));
	if(m_visiblePatchEnd.x > m_patchCount.x) m_visiblePatchEnd.x = m_patchCount.x;
	if(m_visiblePatchEnd.y > m_patchCount.y) m_visiblePatchEnd.y = m_patchCount.y;

	m_visiblePatchEnd += m_visiblePatchStart;
}

//private
void LayerSet::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
	//std::vector<sf::Int32> dirtyPatches; //TODO prevent patches being duplicated
	for(const auto& q : m_dirtyQuads)
	{
		for(const auto& p : q->m_indices)
		{
			m_patches[q->m_patchIndex][p].position += q->m_movement;
		}
		//mark AABB as dirty if patch size has changed - TODO this doesn't shrink AABB :/
		//if(!m_boundingBox.contains(m_patches[q->m_patchIndex][0].position)
		//	|| !m_boundingBox.contains(m_patches[q->m_patchIndex][0].position))
		//{
		//	//we only need to check first and 3rd - not all 4
		//	dirtyPatches.push_back(q->m_patchIndex);
		//}
	}
	m_dirtyQuads.clear();

	//for(auto p : dirtyPatches)
	//{
	//	//update AABB
	//	sf::Vector2f min, max;
	//	const auto& verts = m_patches[p];
	//	for(auto i = 0; i < verts.size(); i += 2) //only check extents
	//	{
	//		if(verts[i].position.x < min.x) min.x = verts[i].position.x;
	//		else if(verts[i].position.x > max.x) max.x = verts[i].position.x;
	//		if(verts[i].position.y < min.y) min.y = verts[i].position.y;
	//		else if(verts[i].position.y > max.y) max.y = verts[i].position.y;
	//	}

	//	//TODO this doesn't shrink the AABB!
	//	if(m_boundingBox.left > min.x) m_boundingBox.left = min.x;
	//	if(m_boundingBox.top > min.y) m_boundingBox.top = min.y;
	//	if(std::fabs(m_boundingBox.left) + m_boundingBox.width < max.x) m_boundingBox.width = std::fabs(m_boundingBox.left) + max.x;
	//	if(std::fabs(m_boundingBox.top) + m_boundingBox.height < max.y) m_boundingBox.width = std::fabs(m_boundingBox.top) + max.y;
	//}

	if(!m_visible) return;

	for(auto x = m_visiblePatchStart.x; x <= m_visiblePatchEnd.x; ++x)
	{
		for(auto y = m_visiblePatchStart.y; y <= m_visiblePatchEnd.y; ++y)
		{
			auto index = y * m_patchCount.x + x;
			if(index < m_patches.size() && !m_patches[index].empty())
			{
				states.texture = &m_texture;
				rt.draw(m_patches[index].data(), static_cast<unsigned>(m_patches[index].size()), sf::Quads, states);
			}
		}
	}
}

void LayerSet::UpdateAABB(sf::Vector2f position, sf::Vector2f size)
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

	if(size.x > m_boundingBox.width + std::fabs(m_boundingBox.left))
		m_boundingBox.width = size.x + std::fabs(m_boundingBox.left);

	if(size.y > m_boundingBox.height + std::fabs(m_boundingBox.top))
		m_boundingBox.height = size.y + std::fabs(m_boundingBox.top);
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