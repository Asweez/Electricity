#pragma once
#ifndef SCREEN_EDITING_H
#define SCREEN_EDITING_H
#include "coord.h"
#include "vectorf.h"
using namespace std;

class ScreenEditing {
public:
	ScreenEditing(int, int, coord);
	void tryZoom(float delta);
	bool tryMove(const int dir);
	float getTileSize();
	coord getUpperLeftCorner();
	coord translateScreenCoordToGlobalCoord(coord screenCoord);
	bool updateSelection(coord newStart, coord newSize);
	void copy(int** pixels, int **metadata, int** charge, bool cut);
	void paste(coord upperLeftPos, int** pixels, int** metadata, int** charge);
	void deleteArea(int** pixels, int** metadata, int** charge);
	void moveSelection(int direction, int** pixels, int** metadata, int** charge, int distance);
	void rotate(int** pixels, int** metadata, int** charge);
	void flipHorizontally(int**, int**, int**);
	void flipVertically(int**, int**, int**);
	int zoom;
	vectorf centerTile;
	int width, height;
	coord selectionStart;
	coord selectionSize;
private:
	int** clipboard;
	int** clipboardCharge;
	int** clipboardMeta;
	coord clipboardSize;
	coord screenSize;
};

#endif
