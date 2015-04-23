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

/*
Demonstrates adding and removing objects to an object layer dynamically.
Note that this is not the preferred method, and that this example only
exists by request.
*/

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <tmx/MapLoader.h>

#include <sstream>
#include <cassert>

void AddObject(tmx::MapLayer& layer, const sf::Vector2f& coords)
{
	const float width = 32.f;
	const float height = 64.f;
	
	tmx::MapObject mo;
	mo.SetPosition(coords);
	mo.AddPoint(sf::Vector2f());
	mo.AddPoint(sf::Vector2f(width, 0.f));
	mo.AddPoint(sf::Vector2f(width, height));
	mo.AddPoint(sf::Vector2f(0.f, height));
	mo.CreateDebugShape(sf::Color::Magenta); //must always call this after adding points
											//to properly create bounding properties.

	layer.objects.push_back(mo);
}


int main()
{
	sf::RenderWindow renderWindow(sf::VideoMode(800u, 600u), "TMX Loader");
	renderWindow.setVerticalSyncEnabled(true);

	//create map loader and load map
	tmx::MapLoader ml("maps/");
	ml.Load("addRemove.tmx");

	sf::Clock deltaClock, frameClock;

	assert(ml.GetLayers().size() > 1);
	auto& objectLayer = ml.GetLayers()[1];


	//-----------------------------------//

	while(renderWindow.isOpen())
	{
		//poll input
		sf::Event event;
		while(renderWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				renderWindow.close();

			if(event.type == sf::Event::KeyReleased)
			{
				switch(event.key.code)
				{
				case sf::Keyboard::Escape:
					renderWindow.close();
					break;
				case sf::Keyboard::D:
					if(objectLayer.objects.size())
					{
						//objectLayer.objects.pop_back();
						objectLayer.objects.erase(objectLayer.objects.end() - 1);
					}
					break;
				default: break;
				}
			}

			if(event.type == sf::Event::MouseButtonReleased)
			{
				switch(event.mouseButton.button)
				{
				case sf::Mouse::Left:
					{
					sf::Vector2f mousePos = renderWindow.mapPixelToCoords(sf::Mouse::getPosition(renderWindow));
					AddObject(objectLayer, mousePos);
					}
					break;
				default: break;
				}
			}
        }

		frameClock.restart();

		//draw
		renderWindow.clear();
		renderWindow.draw(ml);
		ml.Draw(renderWindow, tmx::MapLayer::Debug);
		renderWindow.display();

		const float time = 1.f / frameClock.getElapsedTime().asSeconds();
		std::stringstream stream;
		stream << "Click mouse to add or D to remove. Current fps: " << time << std::endl;
		renderWindow.setTitle(stream.str());
	}

	return 0;
}