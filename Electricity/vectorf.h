#pragma once
#ifndef VECTOR_F_H
#define VECTOR_F_H

class vectorf {
public:
	vectorf(float, float);
	vectorf();
	vectorf operator+ (const vectorf&);
	vectorf operator- (const vectorf&);
	bool operator!=(const vectorf&);
	bool operator==(const vectorf&);
	static vectorf lerp(const vectorf& c1, const vectorf& c2, float lerp);
	static float sqrDist(const vectorf&, const vectorf&);
	float x, y;
};

#endif // !VECTOR_F_H
