#pragma once
#ifndef COORD
#define COORD
#include <SFML\Graphics.hpp>
#include "vectorf.h"
class coord {
public:
	coord(int, int);
	coord(sf::Vector2i);
	coord(vectorf);
	coord();
	coord operator+ (const coord&);
	coord operator- (const coord&);
	bool operator!=(const coord&);
    bool operator== (const coord&) const;
	static coord lerp(const coord& c1, const coord& c2, float lerp);
	static float sqrDist(const coord&, const coord&);
	int x, y;
};

#endif
