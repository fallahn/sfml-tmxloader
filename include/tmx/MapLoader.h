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

#ifndef MAP_LOADER_H_
#define MAP_LOADER_H_

#include <tmx/QuadTreeNode.h>
#include <tmx/MapLayer.h>

#include <pugixml/pugixml.hpp>

#include <array>
#include <cassert>
#include <bitset>

namespace tmx
{
	enum class MapOrientation
	{
		Orthogonal,
		Isometric,
		SteppedIsometric
	};

	class MapLoader final : public sf::Drawable, private sf::NonCopyable
	{
	public:
		//requires a path to the map directory relative to working directory
		//and the maximum number of tiles in a single patch along one edge
		//where 0 does no patch splitting.
		MapLoader(const std::string& mapDirectory, sf::Uint8 patchSize = 10u);
		//loads a given tmx file, returns false on failure
		bool Load(const std::string& mapFile);
		//loads a map from an xml string in memory
		bool LoadFromMemory(const std::string& xmlString);
		//adds give path to list of directories to search for assets, such as tile sets
		void AddSearchPath(const std::string& path);
		//updates the map's quad tree. Not necessary when not querying the quad tree
		//root area is the are covered by root node, for example the screen size
		void UpdateQuadTree(const sf::FloatRect& rootArea);
		//queries the quad tree and returns a vector of objects contained by nodes enclosing
		//or intersecting testArea
		std::vector<MapObject*> QueryQuadTree(const sf::FloatRect& testArea);
		//returns a vector of map layers
		std::vector<MapLayer>& GetLayers();
		const std::vector<MapLayer>& GetLayers() const;
		//draws visible tiles to given target, optionally draw outline of objects for debugging
		void Draw(sf::RenderTarget& rt, MapLayer::DrawType type, bool debug = false);
		//overload for drawing layer by index
		void Draw(sf::RenderTarget& rt, sf::Uint16 index, bool debug = false);
		//projects orthogonal world coords to isometric world coords if available, else return original value
		//eg: use to convert an isometric world coordinate to a position to be drawn in view space
		sf::Vector2f IsometricToOrthogonal(const sf::Vector2f& projectedCoords);
		//returns orthogonal world coords from projected coords
		//eg: use to find the orthogonal world coordinates currently under the mouse cursor
		sf::Vector2f OrthogonalToIsometric(const sf::Vector2f& worldCoords);
		//returns the size of an individual tile in pixels
		sf::Vector2u GetTileSize() const;
		//returns the map size in pixels
		sf::Vector2u GetMapSize() const;
		//returns empty string if property not found
		std::string GetPropertyString(const std::string& name);
		//sets the shader property of a layer's rendering states member
		void SetLayerShader(sf::Uint16 layerId, const sf::Shader& shader);
		//so we can test if QuadTree is available
        bool QuadTreeAvailable() const;

    private:
		//properties which correspond to tmx
		sf::Uint16 m_width, m_height; //tile count
		sf::Uint16 m_tileWidth, m_tileHeight; //width / height of tiles
		MapOrientation m_orientation;
		float m_tileRatio; //width / height ratio of isometric tiles
		std::map<std::string, std::string> m_properties;

		mutable sf::FloatRect m_bounds; //bounding area of tiles visible on screen
		mutable sf::Vector2f m_lastViewPos; //save recalc bounds if view not moved
		std::vector<std::string> m_searchPaths; //additional paths to search for tileset files

		mutable std::vector<MapLayer> m_layers; //layers of map, including image and object layers
		std::vector<std::unique_ptr<sf::Texture>> m_imageLayerTextures;
		std::vector<std::unique_ptr<sf::Texture>> m_tilesetTextures; //textures created from complete sets used when drawing vertex arrays
		const sf::Uint8 m_patchSize;
		struct TileInfo //holds texture coords and tileset id of a tile
		{
			std::array<sf::Vector2f, 4> Coords;
			sf::Vector2f Size;
			sf::Uint16 TileSetId;
			TileInfo();
			TileInfo(const sf::IntRect& rect, const sf::Vector2f& size, sf::Uint16 tilesetId);
		};
		std::vector<TileInfo> m_tileInfo; //stores information on all the tilesets for creating vertex arrays

		sf::VertexArray m_gridVertices; //used to draw map grid in debug
		bool m_mapLoaded, m_quadTreeAvailable;
		//root node for quad tree partition
		QuadTreeRoot m_rootNode;


		bool LoadFromXmlDoc(const pugi::xml_document& doc);
		//resets any loaded map properties
		void Unload();
		//sets the visible area of tiles to be drawn
		void SetDrawingBounds(const sf::View& view);

		//utility functions for parsing map data
		bool ParseMapNode(const pugi::xml_node& mapNode);
		bool ParseTileSets(const pugi::xml_node& mapNode);
		bool ProcessTiles(const pugi::xml_node& tilesetNode);
        bool ParseCollectionOfImages(const pugi::xml_node& tilesetNode);
		bool ParseLayer(const pugi::xml_node& layerNode);
        TileQuad* AddTileToLayer(MapLayer& layer, sf::Uint16 x, sf::Uint16 y, sf::Uint32 gid, const sf::Vector2f& offset = sf::Vector2f());
		bool ParseObjectgroup(const pugi::xml_node& groupNode);
		bool ParseImageLayer(const pugi::xml_node& imageLayerNode);
		void ParseLayerProperties(const pugi::xml_node& propertiesNode, MapLayer& destLayer);
		void SetIsometricCoords(MapLayer& layer);
		void DrawLayer(sf::RenderTarget& rt, MapLayer& layer, bool debug = false);
		std::string FileFromPath(const std::string& path);

		//sf::drawable
		void draw(sf::RenderTarget& rt, sf::RenderStates states) const;

		//utility method for parsing colour values from hex values
		sf::Color ColourFromHex(const char* hexStr) const;

		//method for decompressing zlib compressed strings
		bool Decompress(const char* source, std::vector<unsigned char>& dest, int inSize, int expectedSize);
		//creates a vertex array used to draw grid lines when using debug output
		void CreateDebugGrid(void);

		//caches loaded images to prevent loading the same tileset more than once
		sf::Image& LoadImage(const std::string& imageName);
		std::map<std::string, std::shared_ptr<sf::Image> >m_cachedImages;
		bool m_failedImage;

        //Reading the flipped bits
        std::vector<unsigned char> IntToBytes(sf::Uint32 paramInt);
        std::pair<sf::Uint32, std::bitset<3> > ResolveRotation(sf::Uint32 gid);

        //Image flip functions
        void FlipY(sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3);
        void FlipX(sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3);
        void FlipD(sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3);

        void DoFlips(std::bitset<3> bits,sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3);
    };


	//method for decoding base64 encoded strings
	static std::string base64_decode(std::string const& string);
}

#endif //MAP_LOADER_H_
