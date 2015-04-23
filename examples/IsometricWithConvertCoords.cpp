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

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <tmx/MapLoader.h>
#include <sstream>

int main()
{
	sf::RenderWindow renderWindow(sf::VideoMode(800u, 600u), "TMX Loader");
	renderWindow.setVerticalSyncEnabled(true);

	//create map loader and load map
	tmx::MapLoader ml("maps/");
	ml.Load("isometric_grass_and_water.tmx");

	//adjust the view to centre on map
	sf::View view = renderWindow.getView();
	view.setCenter(0.f, 300.f);
	renderWindow.setView(view);

	//to toggle debug output
	bool debug = false;

	//-----------------------------------//

	while(renderWindow.isOpen())
	{
		//poll input
		sf::Event event;
		while(renderWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				renderWindow.close();
			if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D)
				debug = !debug;
        }

		//draw map
		renderWindow.clear();
		renderWindow.draw(ml);
		if(debug)ml.Draw(renderWindow, tmx::MapLayer::Debug);
		renderWindow.display();

		//print mouse coords to orthographic (screen) coords and world (isometric) coords
		sf::Vector2f mousePosScreen = renderWindow.mapPixelToCoords(sf::Mouse::getPosition(renderWindow));
		sf::Vector2f mousePosWorld = ml.OrthogonalToIsometric(mousePosScreen);

		std::stringstream stream;
		stream << "Mouse Position: "<< mousePosScreen.x << ", " << mousePosScreen.y << " World Position: " << mousePosWorld.x << ", " << mousePosWorld.y;

		renderWindow.setTitle(stream.str());
	}

	return 0;
}