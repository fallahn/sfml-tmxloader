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
// map object to box2D body example.
//////////////////////////////////////////////////////////////////////

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <tmx/MapLoader.h>
#include <tmx/tmx2box2d.h>

#include <tmx/DebugShape.h>

#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <Box2D/Collision/Shapes/b2CircleShape.h>

#include <memory>


int main()
{
	sf::RenderWindow renderWindow(sf::VideoMode(1280u, 640u), "TMX Loader - Box2D Body Creation Example");
	renderWindow.setVerticalSyncEnabled(true);

	//create map loader and load map
	tmx::MapLoader ml("maps/");
	ml.Load("b2d.tmx");

	//create a box2D world
	b2World world(tmx::SfToBoxVec(sf::Vector2f(0.f, 1000.f)));

	//parse map objects
	std::vector<std::unique_ptr<sf::Shape>> debugBoxes;
	std::vector<DebugShape> debugShapes;
	std::map<b2Body*, sf::CircleShape> dynamicShapes; //we can use raw pointers because box2D manages its own memory

	const std::vector<tmx::MapLayer>& layers = ml.GetLayers();
	for (const auto& l : layers)
	{
		if (l.name == "Static") //static bodies which make up the map geometry
		{
			for (const auto& o : l.objects)
			{
				//receive a pointer to the newly created body
				b2Body* b = tmx::BodyCreator::Add(o, world);

				//iterate over body info to create some visual debugging shapes to help visualise
				debugBoxes.push_back(std::unique_ptr<sf::RectangleShape>(new sf::RectangleShape(sf::Vector2f(6.f, 6.f))));
				sf::Vector2f pos = tmx::BoxToSfVec(b->GetPosition());
				debugBoxes.back()->setPosition(pos);
				debugBoxes.back()->setOrigin(3.f, 3.f);

				for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
				{
					b2Shape::Type shapeType = f->GetType();
					if (shapeType == b2Shape::e_polygon)
					{
						DebugShape ds;
						ds.setPosition(pos);
						b2PolygonShape* ps = (b2PolygonShape*)f->GetShape();

						int count = ps->GetVertexCount();
						for (int i = 0; i < count; i++)
							ds.AddVertex(sf::Vertex(tmx::BoxToSfVec(ps->GetVertex(i)), sf::Color::Green));

						ds.AddVertex(sf::Vertex(tmx::BoxToSfVec(ps->GetVertex(0)), sf::Color::Green));
						debugShapes.push_back(ds);
					}
					else if (shapeType == b2Shape::e_circle)
					{
						b2CircleShape* cs = static_cast<b2CircleShape*>(f->GetShape());
						float radius = tmx::BoxToSfFloat(cs->m_radius);
						std::unique_ptr<sf::CircleShape> c(new sf::CircleShape(radius));
						c->setPosition(pos);
						c->setOrigin(radius, radius);
						c->setOutlineColor(sf::Color::Green);
						c->setOutlineThickness(-1.f);
						c->setFillColor(sf::Color::Transparent);
						debugBoxes.push_back(std::move(c));
					}
				}
			}
		}
		else if (l.name == "Dynamic")
		{
			for (const auto& o : l.objects)
			{
				//this time keep a copy of the pointer so we can update the dynamic objects
				//with their information. Don't forget to create a dynamic body
				b2Body* b = tmx::BodyCreator::Add(o, world, b2BodyType::b2_dynamicBody);
				b->GetFixtureList()->SetRestitution(0.99f); //set some properties of the body
				//we assume for this example all dynamic objects are circular. Other shapes also work
				//but you need to impliment your own drawing for them.
				b2CircleShape* cs = static_cast<b2CircleShape*>(b->GetFixtureList()->GetShape());
				const float radius = tmx::BoxToSfFloat(cs->m_radius);
				sf::CircleShape c(radius);
				c.setOrigin(radius, radius);

				dynamicShapes.insert(std::pair<b2Body*, sf::CircleShape>(b, c));
			}
		}
	}


	//-----------------------------------//
	sf::Clock clock;
	while(renderWindow.isOpen())
	{
		//poll input
		sf::Event event;
		while(renderWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				renderWindow.close();
        }

		//update
		world.Step(clock.restart().asSeconds(), 6, 3);
		for (auto& ds : dynamicShapes)
		{
			//move the circle shape based on physics sim using conversion functions for units
			ds.second.setPosition(tmx::BoxToSfVec(ds.first->GetPosition()));
		}

		//draw
		renderWindow.clear();
		renderWindow.draw(ml);
		for (const auto& s : debugBoxes)
			renderWindow.draw(*s);
		for (const auto& s : debugShapes)
			renderWindow.draw(s);
		for (auto& ds : dynamicShapes)
			renderWindow.draw(ds.second);

		renderWindow.display();
	}

	return 0;
}