/*********************************************************************
Matt Marchant 2015 - 2016
http://trederia.blogspot.com

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

//flexible logging class

#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <tmx/Export.hpp>

#include <SFML/System/NonCopyable.hpp>

#include <string>
#include <iostream>
#include <fstream>

//to enable logging define one of these in your preprocessor directives
#ifdef LOG_OUTPUT_ALL
#define LOG(m, t) tmx::Logger::log(m, t, tmx::Logger::Output::All)
#elif defined LOG_OUTPUT_CONSOLE
#define LOG(m, t) tmx::log(m, t, tmx::Logger::Output::Console)
#elif defined LOG_OUTPUT_FILE
#define LOG(m, t) tmx::Logger::log(m, t, tmx::Logger::Output::File)
#else
#define LOG(m, t)
#endif //LOG_LEVEL

#ifdef _MSC_VER
#include <Windows.h>
#endif //MSC_VER

namespace tmx
{
    class Logger final : private sf::NonCopyable
    {
    public:
        enum class Output
        {
            Console,
            File,
            All
        };

        enum Type
        {
            Info	= (1 << 0),
            Warning = (1 << 1),
            Error	= (1 << 2)
        };

        Logger();

        /*!
        \brief logs a message to a given destination.
        \param message Message to log
        \param type Whether this message gets tagged as information, a warning or an error
        \param output Can be the console via cout, a log file on disk, or both
        */
        void log(const std::string& message, Type type = Type::Info, Output output = Output::Console)
        {
			if(!(m_logFilter & type)) return;

            std::string outstring;
            switch (type)
            {
            case Type::Info:
            default:
                outstring = "INFO: " + message;
                break;
            case Type::Error:
                outstring = "ERROR: " + message;
                break;
            case Type::Warning:
                outstring = "WARNING: " + message;
                break;
            }

            if (output == Output::Console || output == Output::All)
            {
                (type == Type::Error) ?
                    std::cerr << outstring << std::endl
                :
                    std::cout << outstring <<  std::endl;
#ifdef _MSC_VER
                outstring += "\n";
                auto stemp = std::wstring(std::begin(outstring), std::end(outstring));
                OutputDebugString((LPCSTR)stemp.c_str());
#endif //_MSC_VER
            }
            if (output == Output::File || output == Output::All)
            {
                //output to a log file
                std::ofstream file("output.log", std::ios::app);
                if (file.good())
                { 
                    file << outstring << std::endl;
                    file.close();
                }
                else
                {
                    log(message, type, Output::Console);
                    log("Above message was intended for log file. Opening file probably failed.", Type::Warning, Output::Console);
                }
            }
        }

		/*!
        \brief Sets the level of logging via a bit mask.
		Made up from Logger::Info, Logger::Warning and Logger::Error
        */
		void setLogLevel(int level)
		{
			m_logFilter = level;
		}
	private:
		int m_logFilter;
    };

    /*!
    \brief Global access point for logging to default logger
    */
    TMX_EXPORT_API void log(const std::string& m, tmx::Logger::Type t, tmx::Logger::Output o);
    /*!
    \brief Global access point for setting log level of default logger
    */
    TMX_EXPORT_API void setLogLevel(int level);
}

#endif //LOGGER_HPP_