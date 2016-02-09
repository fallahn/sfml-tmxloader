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

#ifndef MAP_LOADER_HPP_
#define MAP_LOADER_HPP_

#include <tmx/QuadTreeNode.hpp>
#include <tmx/MapLayer.hpp>

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

    /*!
    \brief A drawable class which parses Tiled tmx format map files
    */
	class TMX_EXPORT_API MapLoader final : public sf::Drawable, private sf::NonCopyable
	{
	public:
		/*!
        \brief Constructor
        Requires a path to the map directory relative to working directory
		and the maximum number of tiles in a single patch along one edge
		where 0 does no patch splitting.
        */
		MapLoader(const std::string& mapDirectory, sf::Uint8 patchSize = 10u);
		/*!
        \brief Loads a given tmx file, returns false on failure
        */
		bool load(const std::string& mapFile);
		/*!
        \brief Loads a map from an xml string in memory
        */
		bool loadFromMemory(const std::string& xmlString);
		/*!
        \brief Adds give path to list of directories to search for assets, such as tile sets
        */
		void addSearchPath(const std::string& path);
		/*!
        \brief Updates the map's quad tree.
        Not necessary when not querying the quad tree. Root area is the area covered by root node,
        for example the screen size
        */
		void updateQuadTree(const sf::FloatRect& rootArea);
		/*!
        \brief Queries the quad tree and returns a vector of objects contained by nodes enclosing
		or intersecting testArea
        */
		std::vector<MapObject*> queryQuadTree(const sf::FloatRect& testArea);
		/*!
        \brief Returns a vector of map layers
        */
		std::vector<MapLayer>& getLayers();
        /*!
        \brief Returns a vector of map layers
        */
		const std::vector<MapLayer>& getLayers() const;
		/*!
        \brief Draws visible tiles to given target, optionally draw outline of objects for debugging
        */
		void drawLayer(sf::RenderTarget& rt, MapLayer::DrawType type, bool debug = false);
        /*!
        \brief Draws a layer by index
        */
		void drawLayer(sf::RenderTarget& rt, sf::Uint16 index, bool debug = false);
		/*!
        \brief Projects orthogonal world coords to isometric world coords if available, else returns original value.
		eg: use to convert an isometric world coordinate to a position to be drawn in view space
        */
		sf::Vector2f isometricToOrthogonal(const sf::Vector2f& projectedCoords);
		/*!
        \brief Returns orthogonal world coords from projected coords.
		eg: use to find the orthogonal world coordinates currently under the mouse cursor
        */
		sf::Vector2f orthogonalToIsometric(const sf::Vector2f& worldCoords);
		/*!
        \brief Returns the size of an individual tile in pixels
        */
		sf::Vector2u getTileSize() const;
		/*!
        \brief Returns the map size in pixels
        */
		sf::Vector2u getMapSize() const;
		/*!
        \brief Returns empty string if property not found
        */
		std::string getPropertyString(const std::string& name);
		/*!
        \brief Sets the shader property of a layer's rendering states member
        */
		void setLayerShader(sf::Uint16 layerId, const sf::Shader& shader);
        /*!
        \brief Returns true if the Quad Tree is available
        */
        bool quadTreeAvailable() const;

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
		struct TileInfo final //holds texture coords and tileset id of a tile
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


		bool loadFromXmlDoc(const pugi::xml_document& doc);
		//resets any loaded map properties
		void unload();
		//sets the visible area of tiles to be drawn
		void setDrawingBounds(const sf::View& view);

		//utility functions for parsing map data
		bool parseMapNode(const pugi::xml_node& mapNode);
		bool parseTileSets(const pugi::xml_node& mapNode);
		bool processTiles(const pugi::xml_node& tilesetNode);
        bool parseCollectionOfImages(const pugi::xml_node& tilesetNode);
		bool parseLayer(const pugi::xml_node& layerNode);
        TileQuad* addTileToLayer(MapLayer& layer, sf::Uint16 x, sf::Uint16 y, sf::Uint32 gid, const sf::Vector2f& offset = sf::Vector2f());
		bool parseObjectgroup(const pugi::xml_node& groupNode);
		bool parseImageLayer(const pugi::xml_node& imageLayerNode);
		void parseLayerProperties(const pugi::xml_node& propertiesNode, MapLayer& destLayer);
		void setIsometricCoords(MapLayer& layer);
		void drawLayer(sf::RenderTarget& rt, MapLayer& layer, bool debug = false);
		std::string fileFromPath(const std::string& path);

		//sf::drawable
		void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;

		//utility method for parsing colour values from hex values
		sf::Color colourFromHex(const char* hexStr) const;

		//method for decompressing zlib compressed strings
		bool decompress(const char* source, std::vector<unsigned char>& dest, int inSize, int expectedSize);
		//creates a vertex array used to draw grid lines when using debug output
		void createDebugGrid(void);

		//caches loaded images to prevent loading the same tileset more than once
		sf::Image& loadImage(const std::string& imageName);
		std::map<std::string, std::shared_ptr<sf::Image> >m_cachedImages;
		bool m_failedImage;

        //Reading the flipped bits
        std::vector<unsigned char> intToBytes(sf::Uint32 paramInt);
        std::pair<sf::Uint32, std::bitset<3> > resolveRotation(sf::Uint32 gid);

        //Image flip functions
        void flipY(sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3);
        void flipX(sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3);
        void flipD(sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3);

        void doFlips(std::bitset<3> bits,sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3);
    };


	//method for decoding base64 encoded strings
	static std::string base64_decode(std::string const& string);
}

#endif //MAP_LOADER_HPP_
