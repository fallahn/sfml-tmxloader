/*********************************************************************
Matt Marchant 2013

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

#ifndef HELPERS_H_
#define HELPERS_H_

#include <math.h>
#include <SFML/Graphics/Rect.hpp>

//helper functions
namespace Helpers
{
	namespace Vectors
	{
		//returns length squared
		static const float GetLengthSquared(const sf::Vector2f& source)
		{
			return (source.x * source.x) + (source.y * source.y);
		}

		//Returns length of a given vector
		static const float GetLength(const sf::Vector2f& source)
		{
			return sqrt(GetLengthSquared(source));
		}

		//calculats dot product of 2 vectors
		static const float Dot(sf::Vector2f lv, sf::Vector2f rv)
		{
			return lv.x * rv.x + lv.y * rv.y;
		}

		//Returns a given vector with its length normalized to 1
		static const sf::Vector2f Normalize(sf::Vector2f source)
		{
			float length = std::sqrt(Dot(source, source));
			if (length != 0) source /= length;
				//return sf::Vector2f(source.x / length, source.y / length);
			//else
			return source;
		}

		//Returns angle in degrees of a given vector where 0 is horizontal
		static const float GetAngle(const sf::Vector2f& source)
		{
			return atan2(source.y , source.x) * 180.f / 3.14159265f;
		}
	};
};

#endif