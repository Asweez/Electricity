#include "vectorf.h"

vectorf::vectorf(float x1, float y1) {
	x = x1;
	y = y1;
}

vectorf::vectorf() {
	x = 0;
	y = 0;
}

vectorf vectorf::operator+ (const vectorf& c1) {
	vectorf newvectorf(c1.x + this->x, c1.y + this->y);
	return newvectorf;
}

vectorf vectorf::operator- (const vectorf& c1) {
	vectorf newvectorf(this->x - c1.x, this->y - c1.y);
	return newvectorf;
}

bool vectorf::operator!= (const vectorf& c1) {
	return c1.x != this->x || c1.y != this->y;
}

bool vectorf::operator== (const vectorf& c1) {
	return c1.x == this->x && c1.y == this->y;
}

vectorf vectorf::lerp(const vectorf& c1, const vectorf& c2, float lerp) {
	return vectorf(c1.x + lerp * (c2.x - c1.x), c1.y + lerp * (c2.y - c1.y));
}

float vectorf::sqrDist(const vectorf& c1, const vectorf& c2) {
	int dx = c1.x - c2.x;
	int dy = c1.y - c2.y;
	return dx * dx + dy * dy;
}