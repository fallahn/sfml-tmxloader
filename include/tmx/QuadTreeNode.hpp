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

#ifndef QUADTREE_NODE_HPP_
#define QUADTREE_NODE_HPP_

#include <tmx/MapObject.hpp>

#include <memory>

namespace tmx
{
    /*!
    \brief Creates a node used to build quad trees for spatial partitioning of MapObjects.
    Example usage: create a root node the size of the viewable area, and insert each
    available map object. Then test the root node by calling retrieve passing for example
    the AABB of a sprite. The resulting vector will contain pointers to any objects contained
    in quads which are them selves contained, or intersected, by the sprites AABB. These can
    then be collision tested.   
    */

	class TMX_EXPORT_API QuadTreeNode
	{
        friend class QuadTreeRoot;
	public:
		QuadTreeNode(sf::Uint16 level = 0, const sf::FloatRect& bounds = sf::FloatRect(0.f, 0.f, 1.f, 1.f));
		virtual ~QuadTreeNode() = default;

		/*!
        \brief Fills a vector with references to all objects which
		appear in quads which are contained or intersect bounds.
        */
		std::vector<MapObject*> retrieve(const sf::FloatRect& bounds, sf::Uint16& currentDepth);
		/*!
        \brief Inserts a reference to the object into the node's object list
        */
		void insert(const MapObject& object);
	protected:

		sf::Uint16 m_level;
		sf::FloatRect m_bounds;
		std::vector<MapObject*> m_objects; //objects contained in current node
		std::vector< std::shared_ptr<QuadTreeNode> > m_children; //vector of child nodes

		/*!
        \brief Returns the index of the child node into which the givens bounds fits.
		Returns -1 if doesn't completely fit a child. Numbered anti-clockwise
		from top right node.
        */
		sf::Int16 getIndex(const sf::FloatRect& bounds);

		/*!
        \brief Divides node by creating 4 children
        */
		void split(void);

        void getVertices(std::vector<sf::Vertex>&);

	};

	/*!
    \brief Specialisation of QuadTreeNode for counting tree depth
    */
    class TMX_EXPORT_API QuadTreeRoot final : public QuadTreeNode, public sf::Drawable
	{
	public:
		QuadTreeRoot(sf::Uint16 level = 0, const sf::FloatRect& bounds = sf::FloatRect(0.f, 0.f, 1.f, 1.f))
			: QuadTreeNode(level, bounds), m_depth(0u), m_searchDepth(0u){};

		/*!
        \brief Clears node and all children
        */
		void clear(const sf::FloatRect& newBounds);
		/*!
        \brief Retrieves all objects in quads which contains or intersect test area
        */
		std::vector<MapObject*> retrieve(const sf::FloatRect& bounds)
		{
			return QuadTreeNode::retrieve(bounds, m_searchDepth);
		}

	private:
		//total depth of tree, and depth reached when querying
		sf::Uint16 m_depth, m_searchDepth;

        void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
	};
};


#endif //QUADTREE_NODE_HPP_