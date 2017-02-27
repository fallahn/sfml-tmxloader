#include <cassert>
#include <fstream>
#include "SFML/Graphics.hpp"
#include "tmx/MapLoader.h"

///Determines if a filename is a regular file
///From http://www.richelbilderbeek.nl/CppIsRegularFile.htm
bool is_regular_file(const std::string& filename) noexcept
{
  std::fstream f;
  f.open(filename.c_str(),std::ios::in);
  return f.is_open();
}


int main()
{
  sf::RenderWindow window(sf::VideoMode(360, 280), "STP Example");
  const std::string filename{"orthogonal-outside.tmx"};
  assert(is_regular_file(filename));
  tmx::MapLoader map(filename);
  assert(!map.GetLayers().empty());

  // Start the game loop
  while (window.isOpen()) {
      // Process events
      sf::Event event;
      while (window.pollEvent(event)) {
          // Close window : exit
          if (event.type == sf::Event::Closed)
              window.close();
      }
      // Clear screen
      window.clear();
      // Draw the map
      window.draw(map);
      // Update the window
      window.display();
  }

  return EXIT_SUCCESS;
}
