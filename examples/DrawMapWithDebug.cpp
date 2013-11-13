#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <MapLoader.h>

int main()
{
	sf::RenderWindow renderWindow(sf::VideoMode(800u, 600u), "TMX Loader");

	//create map loader and load map
	tmx::MapLoader ml("maps/");
	ml.Load("desert.tmx");

	//-----------------------------------//

	while(renderWindow.isOpen())
	{
		//poll input
		sf::Event event;
		while(renderWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				renderWindow.close();
        }

		//draw
		renderWindow.clear();
		renderWindow.draw(ml);
		ml.Draw(renderWindow, tmx::MapLayer::Debug);//draw with debug information shown
		renderWindow.display();
	}

	return 0;
}