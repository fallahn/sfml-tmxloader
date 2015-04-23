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


const char waterShader[] =
"#version 120\n"
"uniform sampler2D baseTexture;"
"uniform float time = 1.0;" //time in seconds

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

"vec3 mod289(vec3 x) {"
	"return x - floor(x * (1.0 / 289.0)) * 289.0;"
"}"

"vec4 mod289(vec4 x) {"
	"return x - floor(x * (1.0 / 289.0)) * 289.0;"
"}"

"vec4 permute(vec4 x) {"
	"return mod289(((x*34.0) + 1.0)*x);"
"}"

"vec4 taylorInvSqrt(vec4 r)"
"{"
	"return 1.79284291400159 - 0.85373472095314 * r;"
"}"

"float snoise(vec3 v)"
"{"
	"const vec2  C = vec2(1.0 / 6.0, 1.0 / 3.0);"
	"const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);"

	// First corner
	"vec3 i = floor(v + dot(v, C.yyy));"
	"vec3 x0 = v - i + dot(i, C.xxx);"

	// Other corners
	"vec3 g = step(x0.yzx, x0.xyz);"
	"vec3 l = 1.0 - g;"
	"vec3 i1 = min(g.xyz, l.zxy);"
	"vec3 i2 = max(g.xyz, l.zxy);"

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	"vec3 x1 = x0 - i1 + C.xxx;"
	"vec3 x2 = x0 - i2 + C.yyy;" // 2.0*C.x = 1/3 = C.y
	"vec3 x3 = x0 - D.yyy;"      // -1.0+3.0*C.x = -0.5 = -D.y

	// Permutations
	"i = mod289(i);"
	"vec4 p = permute(permute(permute("
		"i.z + vec4(0.0, i1.z, i2.z, 1.0))"
		"+ i.y + vec4(0.0, i1.y, i2.y, 1.0))"
		"+ i.x + vec4(0.0, i1.x, i2.x, 1.0));"

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	"float n_ = 0.142857142857;" // 1.0/7.0
	"vec3  ns = n_ * D.wyz - D.xzx;"

	"vec4 j = p - 49.0 * floor(p * ns.z * ns.z);"  //  mod(p,7*7)

	"vec4 x_ = floor(j * ns.z);"
	"vec4 y_ = floor(j - 7.0 * x_);"    // mod(j,N)

	"vec4 x = x_ *ns.x + ns.yyyy;"
	"vec4 y = y_ *ns.x + ns.yyyy;"
	"vec4 h = 1.0 - abs(x) - abs(y);"

	"vec4 b0 = vec4(x.xy, y.xy);"
	"vec4 b1 = vec4(x.zw, y.zw);"

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	"vec4 s0 = floor(b0)*2.0 + 1.0;"
	"vec4 s1 = floor(b1)*2.0 + 1.0;"
	"vec4 sh = -step(h, vec4(0.0));"

	"vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;"
	"vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;"

	"vec3 p0 = vec3(a0.xy, h.x);"
	"vec3 p1 = vec3(a0.zw, h.y);"
	"vec3 p2 = vec3(a1.xy, h.z);"
	"vec3 p3 = vec3(a1.zw, h.w);"

	//Normalise gradients
	"vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));"
	"p0 *= norm.x;"
	"p1 *= norm.y;"
	"p2 *= norm.z;"
	"p3 *= norm.w;"

	// Mix final noise value
	"vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);"
	"m = m * m;"
	"return 42.0 * dot(m*m, vec4(dot(p0, x0), dot(p1, x1),"
		"dot(p2, x2), dot(p3, x3)));"
"}"


"void main(void)"
"{"
	//sin distort for ripples
	"const float sinWidth = 0.08;" //smaller is wider
	"const float sinHeight = 0.001;" //larger is taller
	"const float sinTime = 9.5;" //larger is faster (if time is input in seconds then this value hertz)

	"vec2 coord = gl_TexCoord[0].xy;"

	"float offsetX = sin(gl_FragCoord.y * sinWidth + time * sinTime) * 0.5 + 0.5;"
	"coord.x += offsetX * sinHeight / 2.0;"

	"float offsetY = sin(gl_FragCoord.x * sinWidth + time * sinTime) * 0.5 + 0.5;"
	"coord.y += offsetY * sinHeight;"

	"float highlight = snoise(vec3(gl_FragCoord.xy * 0.04, time)) * 0.1;"

	//add colour for highlight
	"vec3 colour = texture2D(baseTexture, coord).rgb;"
	"colour = clamp(colour + highlight, 0.0, 1.0);"
	"gl_FragColor = vec4(colour, 1.0);"
"}";

int main()
{
	sf::RenderWindow renderWindow(sf::VideoMode(800u, 600u), "TMX Loader");
	renderWindow.setVerticalSyncEnabled(true);

	//create map loader and load map
	tmx::MapLoader ml("maps\\");
	ml.Load("shader_example.tmx");

	bool showDebug = false;
	sf::Clock frameClock, shaderClock;

	//load shader and set layer to use it
	sf::Shader waterEffect;
	waterEffect.loadFromMemory(waterShader, sf::Shader::Fragment);

	ml.SetLayerShader(0u, waterEffect);

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
				case sf::Keyboard::D:
					showDebug = !showDebug;
					break;
				default: break;
				}
			}
        }

		//update shader
		waterEffect.setParameter("time", shaderClock.getElapsedTime().asSeconds());

		//draw
		frameClock.restart();
		renderWindow.clear();
		renderWindow.draw(ml);
		if(showDebug) ml.Draw(renderWindow, tmx::MapLayer::Debug);//draw with debug information shown
		renderWindow.display();

		renderWindow.setTitle("Press D to Toggle debug shapes. " + std::to_string(1.f / frameClock.getElapsedTime().asSeconds()));
	}

	return 0;
}