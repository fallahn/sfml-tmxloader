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
	ml.Load("desert.tmx");

	sf::Clock deltaClock;

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

		//allow moving of view
		sf::View view = renderWindow.getView();
		sf::Vector2f movement;

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			movement.x = -1.f;
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			movement.x = 1.f;
		
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			movement.y = -1.f;
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			movement.y = 1.f;
		
		float dt = deltaClock.restart().asSeconds();
		movement = Helpers::Vectors::Normalize(movement) * 300.f * dt;
		view.move(movement);
		renderWindow.setView(view);

		//draw
		renderWindow.clear();
		renderWindow.draw(ml);
		renderWindow.display();

		const float time = 1.f / dt;
		std::stringstream stream;
		stream << "Use the cursor keys to move the view. Current fps: " << time << std::endl;
		renderWindow.setTitle(stream.str());
	}

	return 0;
}