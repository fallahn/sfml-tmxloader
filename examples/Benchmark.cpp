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
#include <tmx/Log.h>

namespace
{
	sf::Vector2f getViewMovement(float dt)
	{
		sf::Vector2f movement;

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			movement.x = -1.f;
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			movement.x = 1.f;

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			movement.y = -1.f;
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			movement.y = 1.f;

		movement = Helpers::Vectors::normalize(movement) * 500.f * dt;
		return movement;
	}

	void handleWindowEvent(sf::RenderWindow& renderWindow)
	{
			sf::Event event;
			while(renderWindow.pollEvent(event))
			{
				if (event.type == sf::Event::Closed
					|| (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
				{
					renderWindow.close();
				}
			}
	}

	sf::Font loadFont()
	{
		//setup fonts
		sf::Font font;
		if (!font.loadFromFile("fonts/Ubuntu-M.ttf"))
		{
			std::cout << "font not loaded for fps count" << std::endl;
			//do nothing its just a test
		}
		return font;
	}

	sf::Text getFpsText(const sf::Font& font)
	{
		sf::Text fpsText;
		fpsText.setFont(font);
		fpsText.setColor(sf::Color::White);
		fpsText.setCharacterSize(25);
		return fpsText;
	}
}
int main()
{
    sf::RenderWindow renderWindow(sf::VideoMode(800u, 600u), "TMX Loader");
    sf::Font font = loadFont();
    sf::Text fpsText = getFpsText(font);

	//set the debugging output mode
	tmx::Logger::SetLogLevel(tmx::Logger::Info | tmx::Logger::Error);

    //create map loader and load map
    tmx::MapLoader ml("maps/");
    ml.Load("desert.tmx");

    sf::Clock deltaClock, frameClock;

    const float dt = 0.01f;

    float previousUpdateTime = deltaClock.getElapsedTime().asSeconds();
    float accumulator = 0.f;

    while(renderWindow.isOpen())
    {
        handleWindowEvent(renderWindow);

        //update
        float currentTime = deltaClock.getElapsedTime().asSeconds();
        float frameTime = currentTime - previousUpdateTime;
        previousUpdateTime = currentTime;
        accumulator += frameTime;

        sf::Vector2f movement;
        while ( accumulator >= dt )
        {
            movement = getViewMovement(dt);
            accumulator -= dt;
        }

        //allow moving of view
        sf::View view = renderWindow.getView();
        view.move(movement);
        renderWindow.setView(view);

        //show fps
        float fpsCount = (1.f / frameClock.restart().asSeconds());
        fpsText.setString( "FPS: " + (std::to_string(fpsCount)));
        fpsText.move(movement);

        //draw
        renderWindow.clear();
        renderWindow.draw(ml);
        renderWindow.draw(fpsText);
        renderWindow.display();
    }

    return 0;
}
