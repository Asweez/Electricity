#include "coord.h"

coord::coord(int x1, int y1) {
	x = x1;
	y = y1;
}

coord::coord(vectorf v) {
	x = (int)v.x;
	y = (int)v.y;
}

coord::coord(sf::Vector2i vector) {
	x = vector.x;
	y = vector.y;
}

coord::coord() {
	x = 0;
	y = 0;
}

coord coord::operator+ (const coord& c1) {
	coord newCoord(c1.x + this->x, c1.y + this->y);
	return newCoord;
}

coord coord::operator- (const coord& c1) {
	coord newCoord(this->x - c1.x, this->y - c1.y);
	return newCoord;
}

bool coord::operator!= (const coord& c1) {
	return c1.x != this->x || c1.y != this->y;
}

bool coord::operator== (const coord& c1) const{
	return c1.x == this->x && c1.y == this->y;
}

coord coord::lerp(const coord& c1, const coord& c2, float lerp) {
	return coord(c1.x + lerp * (c2.x - c1.x), c1.y + lerp * (c2.y - c1.y)); 
}

float coord::sqrDist(const coord& c1, const coord& c2) {
	int dx = c1.x - c2.x;
	int dy = c1.y - c2.y;
	return dx * dx + dy * dy;
}
