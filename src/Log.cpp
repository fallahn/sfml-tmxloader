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

#include <tmx/Log.hpp>

namespace
{
    tmx::Logger logger;
}

tmx::Logger::Logger() : m_logFilter(Type::Error | Type::Info | Type::Warning){}

TMX_EXPORT_API void tmx::log(const std::string& m, tmx::Logger::Type t, tmx::Logger::Output o)
{
    logger.log(m, t, o);
}

TMX_EXPORT_API void tmx::setLogLevel(int level)
{
    logger.setLogLevel(level);
}