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

#include <tmx/MapLoader.hpp>
#include <tmx/Log.hpp>

#include <cassert>

using namespace tmx;

//ctor
MapLoader::MapLoader(const std::string& mapDirectory, sf::Uint8 patchSize)
	: m_width			(1u),
	m_height			(1u),
	m_tileWidth			(1u),
	m_tileHeight		(1u),
	m_tileRatio			(1.f),
	m_patchSize			(patchSize),
	m_mapLoaded			(false),
	m_quadTreeAvailable	(false),
	m_failedImage		(false)
{
	//reserve some space to help reduce reallocations
	m_layers.reserve(10);
	addSearchPath(mapDirectory);

	assert(patchSize > 0);
}

bool MapLoader::load(const std::string& map)
{
	std::string mapPath = m_searchPaths[0] + fileFromPath(map);
	unload(); //clear any old data first

	//parse map xml, return on error
	pugi::xml_document mapDoc;
	pugi::xml_parse_result result = mapDoc.load_file(mapPath.c_str());
	if(!result)
	{
		LOG("Failed to open " + map, Logger::Type::Error);
		LOG("Reason: " + std::string(result.description()), Logger::Type::Error);
		return m_mapLoaded = false;
	}

	return loadFromXmlDoc(mapDoc);
}

bool MapLoader::loadFromMemory(const std::string& xmlString)
{
	unload();

	pugi::xml_document mapDoc;
	pugi::xml_parse_result result = mapDoc.load_string(xmlString.c_str());
	if(!result)
	{
		LOG("Failed to open map from string", Logger::Type::Error);
		LOG("Reason: " + std::string(result.description()), Logger::Type::Error);
		return m_mapLoaded = false;
	}

	return loadFromXmlDoc(mapDoc);
}

void MapLoader::addSearchPath(const std::string& path)
{
	m_searchPaths.push_back(path);

	std::string& s = m_searchPaths.back();
	std::replace(s.begin(), s.end(), '\\', '/');

	if(s.size() > 1 && *s.rbegin() != '/')
		s += '/';
	else if (s == ".")
		s = "./";
	else if(s == "/" || s == "\\") s = "";
}

void MapLoader::updateQuadTree(const sf::FloatRect& rootArea)
{
	m_rootNode.clear(rootArea);
	for(const auto& layer : m_layers)
	{
		for(const auto& object : layer.objects)
		{
			m_rootNode.insert(object);
		}
	}
	m_quadTreeAvailable = true;
}

std::vector<MapObject*> MapLoader::queryQuadTree(const sf::FloatRect& testArea)
{
	//quad tree must be updated at least once with UpdateQuadTree before we can call this
	assert(m_quadTreeAvailable);
	return m_rootNode.retrieve(testArea);
}

std::vector<MapLayer>& MapLoader::getLayers()
{
	return m_layers;
}

const std::vector<MapLayer>& MapLoader::getLayers() const
{
	return m_layers;
}

void MapLoader::drawLayer(sf::RenderTarget& rt, MapLayer::DrawType type, bool debug)
{
	setDrawingBounds(rt.getView());
	switch(type)
	{
	default:
	case MapLayer::All:
		for(const auto& l : m_layers)
			rt.draw(l);
		break;
	case MapLayer::Back:
		{
		//remember front of vector actually draws furthest back
		MapLayer& layer = m_layers.front();
		drawLayer(rt, layer, debug);
		}
		break;
	case MapLayer::Front:
		{
		MapLayer& layer = m_layers.back();
		drawLayer(rt, layer, debug);
		}
		break;
	case MapLayer::Debug:
		for(auto layer : m_layers)
		{
			if(layer.type == ObjectGroup)
			{
				for(const auto& object : layer.objects)
					if (m_bounds.intersects(object.getAABB()))
						object.drawDebugShape(rt);
			}
		}
		rt.draw(m_gridVertices);
		rt.draw(m_rootNode);
		break;
	}
}

void MapLoader::drawLayer(sf::RenderTarget& rt, sf::Uint16 index, bool debug)
{
	setDrawingBounds(rt.getView());
	drawLayer(rt, m_layers[index], debug);
}

sf::Vector2f MapLoader::isometricToOrthogonal(const sf::Vector2f& projectedCoords)
{
	//skip converting if we don't actually have an isometric map loaded
	if(m_orientation != MapOrientation::Isometric) return projectedCoords;

	return sf::Vector2f(projectedCoords.x - projectedCoords.y, (projectedCoords.x / m_tileRatio) + (projectedCoords.y / m_tileRatio));
}

sf::Vector2f MapLoader::orthogonalToIsometric(const sf::Vector2f& worldCoords)
{
	if(m_orientation != MapOrientation::Isometric) return worldCoords;

	return sf::Vector2f(((worldCoords.x / m_tileRatio) + worldCoords.y),
							(worldCoords.y - (worldCoords.x / m_tileRatio)));
}

sf::Vector2u MapLoader::getTileSize() const
{
	return sf::Vector2u(m_tileWidth, m_tileHeight);
}

sf::Vector2u MapLoader::getMapSize() const
{
	return sf::Vector2u(m_width * m_tileWidth, m_height * m_tileHeight);
}

std::string MapLoader::getPropertyString(const std::string& name)
{
	assert(m_properties.find(name) != m_properties.end());
	return m_properties[name];
}

void MapLoader::setLayerShader(sf::Uint16 layerId, const sf::Shader& shader)
{
	m_layers[layerId].setShader(shader);
}

bool MapLoader::quadTreeAvailable() const
{
	return m_quadTreeAvailable;
}



MapLoader::TileInfo::TileInfo()
	: TileSetId (0u)
{

}

MapLoader::TileInfo::TileInfo(const sf::IntRect& rect, const sf::Vector2f& size, sf::Uint16 tilesetId)
	: Size		(size),
	TileSetId	(tilesetId)
{
	Coords[0] = sf::Vector2f(static_cast<float>(rect.left), static_cast<float>(rect.top));
	Coords[1] = sf::Vector2f(static_cast<float>(rect.left + rect.width), static_cast<float>(rect.top));
	Coords[2] = sf::Vector2f(static_cast<float>(rect.left + rect.width), static_cast<float>(rect.top + rect.height));
	Coords[3] = sf::Vector2f(static_cast<float>(rect.left), static_cast<float>(rect.top + rect.height));
}