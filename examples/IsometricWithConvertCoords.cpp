#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <MapLoader.h>
#include <sstream>

int main()
{
	sf::RenderWindow renderWindow(sf::VideoMode(800u, 600u), "TMX Loader");

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