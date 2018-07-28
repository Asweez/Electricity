#include "ScreenEditing.h"
#include <iostream>

ScreenEditing::ScreenEditing(int width1, int height1, coord screenSize1) {
	width = width1;
	height = height1;
	centerTile = vectorf(width / 2, height / 2);
	zoom = (int)(width / 2);
	screenSize = screenSize1;
}

void ScreenEditing::tryZoom(float delta) {
	if (delta > 0) {
		//zoom in
		zoom = max(20, zoom - 7);
	}
	else {
		//zoom out;
		zoom = min(zoom + 7, width / 2);
		if (centerTile.x - zoom < 0) {
			centerTile.x = zoom;
		}
		else if (centerTile.x + zoom >= width) {
			centerTile.x = (width - 1) - zoom;
		}
		if (centerTile.y - zoom < 0) {
			centerTile.y = zoom;
		}
		else if (centerTile.y + zoom >= height) {
			centerTile.y = (height - 1) - zoom;
		}
	}
	cout << "Zoom: " << zoom << ", Center: (" << centerTile.x << ", " << centerTile.y << ")"<<endl;
}

bool ScreenEditing::tryMove(const int dir) {
	float moveAmount = 0.01f;
	switch (dir) {
	case 0:
		if (centerTile.y - zoom > 0) {
			centerTile.y-= moveAmount;
			return true;
		}
		break;
	case 1:
		if (centerTile.x + zoom < width - 1) {
			centerTile.x+= moveAmount;
			return true;
		}
		break;
	case 2:
		if (centerTile.y + zoom < height - 1) {
			centerTile.y+= moveAmount;
			return true;
		}
		break;
	case 3:
		if (centerTile.x - zoom > 0) {
			centerTile.x-= moveAmount;
			return true;
		}
		break;
	}
	return false;
}

float ScreenEditing::getTileSize() {
	return ((float)screenSize.x) / ((float)(2 * zoom));
}

coord ScreenEditing::getUpperLeftCorner() {
	return coord(centerTile.x - zoom, centerTile.y - zoom);
}

bool ScreenEditing::updateSelection(coord newStart, coord newSize) {
	if (newStart != selectionStart || newSize != selectionSize) {
		selectionStart = newStart;
		selectionSize = newSize;
		return true;
	}
	return false;
}

coord ScreenEditing::translateScreenCoordToGlobalCoord(coord screenCoord) {
	return coord(screenCoord.x + (centerTile.x - zoom), screenCoord.y + (centerTile.y - zoom));
}

void ScreenEditing::copy(int** pixels, int **metadata, int** charge, bool cut) {
	coord start = selectionStart;
	coord end = selectionStart + selectionSize;
	int width = end.x - start.x;
	int height = end.y - start.y;
	width++;
	height++;
	clipboardSize = coord(width, height);
	clipboard = new int*[width];
	clipboardCharge = new int*[width];
	clipboardMeta = new int*[width];

	for (int i = 0; i < width; i++) {
		clipboard[i] = new int[height];
		clipboardCharge[i] = new int[height];
		clipboardMeta[i] = new int[height];
	}
	for (int x = start.x+1; x <= end.x; x++) {
		for (int y = start.y+1; y <= end.y; y++) {
			clipboard[x - start.x][y - start.y] = pixels[x][y];
			clipboardCharge[x - start.x][y - start.y] = charge[x][y];
			clipboardMeta[x - start.x][y - start.y] = metadata[x][y];
			if (cut) {
				pixels[x][y] = 0;
				metadata[x][y] = 0;
				charge[x][y] = 0;
			}
		}
	}
}

void ScreenEditing::paste(coord upperLeftPos, int** pixels, int** metadata, int** charge) {
	if (clipboard == NULL) return;
	for (int x = upperLeftPos.x + 1; x < upperLeftPos.x + clipboardSize.x; x++) {
		for (int y = upperLeftPos.y + 1; y < upperLeftPos.y + clipboardSize.y; y++) {
			pixels[x][y] = clipboard[x - upperLeftPos.x][y - upperLeftPos.y];
			charge[x][y] = clipboardCharge[x - upperLeftPos.x][y - upperLeftPos.y];
			metadata[x][y] = clipboardMeta[x - upperLeftPos.x][y - upperLeftPos.y];
		}
	}
	selectionStart = upperLeftPos;
	selectionSize = clipboardSize + coord(-1, -1);
}

int getRotatedMeta(int tile, int meta) {
	int delay, rotation;
	switch (tile) {
	case 0:
		return meta;
	case 1:
		return meta;
	case 2:
		return meta;
	case 7:
		delay = meta >> 2;
		rotation = (meta & 1) + (2 * (meta & 2));
		rotation = (rotation + 1) % 4;
		return rotation + (delay << 2);
	default:
		return (meta + 1) % 4;
	}
}

int flipMeta(int tile, int meta, bool vertical) {
	int delay, rotation;
	//if vertical is false, then we are flipping horizontally
	switch (tile) {
	case 0:
		return meta;
	case 1:
		return meta;
	case 2:
		return meta;
	case 3:
		if (vertical) {
			return meta == 0 ? 2 : (meta == 2 ? 0 : meta);
		}
		else {
			return meta == 1 ? 3 : (meta == 3 ? 1 : meta);
		}
	case 5:
		if (vertical) {
			if (meta == 0) {
				return 1;
			}
			else if (meta == 1) {
				return 0;
			}
			else if (meta == 2) {
				return 3;
			}
			else {
				return 2;
			}
		}
		else {
			if (meta == 0) {
				return 3;
			}
			else if (meta == 1) {
				return 2;
			}
			else if (meta == 2) {
				return 1;
			}
			else {
				return 0;
			}
		}
	case 7:
		delay = meta >> 2;
		rotation = (meta & 1) + (2 * (meta & 2));
		rotation = (rotation + 2) % 4;
		return rotation + (delay << 2);
	default:
		return meta;
	}
}

void ScreenEditing::rotate(int** pixels, int** metadata, int** charge) {
	this->copy(pixels, metadata, charge, true);
	int** newPixels = new int*[clipboardSize.y];
	int** newMeta = new int*[clipboardSize.y];
	int** newCharge = new int*[clipboardSize.y];
	for (int i = 0; i < clipboardSize.y; i++) {
		newPixels[i] = new int[clipboardSize.x];
		newMeta[i] = new int[clipboardSize.x];
		newCharge [i] = new int[clipboardSize.x];
	}
	for (int x = 1; x < clipboardSize.x; x++) {
		for (int y = 1; y < clipboardSize.y; y++) {
			newPixels[clipboardSize.y - y][x] = clipboard[x][y];
			newMeta[clipboardSize.y - y][x] = getRotatedMeta(clipboard[x][y], clipboardMeta[x][y]);
			newCharge[clipboardSize.y - y][x] = clipboardCharge[x][y];
		}
	}
	clipboardSize = coord(clipboardSize.y, clipboardSize.x);
	delete clipboard;
	delete clipboardCharge;
	delete clipboardMeta;
	clipboard = newPixels;
	clipboardMeta = newMeta;
	clipboardCharge = newCharge;
	
	this->paste(selectionStart , pixels, metadata, charge);
}

void ScreenEditing::flipHorizontally(int** pixels, int** metadata, int** charge) {
	this->copy(pixels, metadata, charge, true);
	if (clipboardSize.x == 0) {
		return;
	}
	int** newPixels = new int*[clipboardSize.x];
	int** newMeta = new int*[clipboardSize.x];
	int** newCharge = new int*[clipboardSize.x];
	for (int i = 0; i < clipboardSize.x; i++) {
		newPixels[i] = new int[clipboardSize.y];
		newMeta[i] = new int[clipboardSize.y];
		newCharge[i] = new int[clipboardSize.y];
	}
	int widthMinusOne = clipboardSize.x;
	for (int x = 1; x < clipboardSize.x; x++) {
		for (int y = 0; y < clipboardSize.y; y++) {
			newPixels[widthMinusOne - x][y] = clipboard[x][y];
			newMeta[widthMinusOne - x][y] = flipMeta(clipboard[x][y], clipboardMeta[x][y], false);
			newCharge[widthMinusOne - x][y] = clipboardCharge[x][y];
		}
	}
	delete clipboard;
	delete clipboardCharge;
	delete clipboardMeta;
	clipboard = newPixels;
	clipboardMeta = newMeta;
	clipboardCharge = newCharge;

	this->paste(selectionStart, pixels, metadata, charge);
}

void ScreenEditing::flipVertically(int** pixels, int** metadata, int** charge) {
	this->copy(pixels, metadata, charge, true);
	if (clipboardSize.x == 0) {
		return;
	}
	int** newPixels = new int*[clipboardSize.x];
	int** newMeta = new int*[clipboardSize.x];
	int** newCharge = new int*[clipboardSize.x];
	for (int i = 0; i < clipboardSize.x; i++) {
		newPixels[i] = new int[clipboardSize.y];
		newMeta[i] = new int[clipboardSize.y];
		newCharge[i] = new int[clipboardSize.y];
	}
	int heightMinusOne = clipboardSize.y;
	for (int x = 0; x < clipboardSize.x; x++) {
		for (int y = 1; y < clipboardSize.y; y++) {
			newPixels[x][heightMinusOne - y] = clipboard[x][y];
			newMeta[x][heightMinusOne - y] = flipMeta(clipboard[x][y], clipboardMeta[x][y], true);
			newCharge[x][heightMinusOne - y] = clipboardCharge[x][y];
		}
	}
	delete clipboard;
	delete clipboardCharge;
	delete clipboardMeta;
	clipboard = newPixels;
	clipboardMeta = newMeta;
	clipboardCharge = newCharge;

	this->paste(selectionStart, pixels, metadata, charge);
}

void ScreenEditing::deleteArea(int** pixels, int** metadata, int** charge) {
	coord start = selectionStart;
	coord end = selectionStart + selectionSize;
	for (int x = start.x + 1; x <= end.x; x++) {
		for (int y = start.y + 1; y <= end.y; y++) {
			pixels[x][y] = 0;
			charge[x][y] = 0;
			metadata[x][y] = 0;
		}
	}
}

void ScreenEditing::moveSelection(int direction, int** pixels, int** metadata, int** charge, int distance) {
	coord start = selectionStart;
	coord end = selectionStart + selectionSize;
	int width = end.x - start.x;
	width++;
	int height = end.y - start.y;
	height++;
	
	coord tempSize = coord(width, height);
	int** temp;
	int** tempCharge;
	int** tempMeta;
	temp = new int*[width];
	tempCharge = new int*[width];
	tempMeta = new int*[width];

	for (int i = 0; i < width; i++) {
		temp[i] = new int[height];
		tempCharge[i] = new int[height];
		tempMeta[i] = new int[height];
	}
	for (int x = start.x+1; x <= end.x; x++) {
		for (int y = start.y+1; y <= end.y; y++) {
			temp[x - start.x][y - start.y] = pixels[x][y];
			tempCharge[x - start.x][y - start.y] = charge[x][y];
			tempMeta[x - start.x][y - start.y] = metadata[x][y];
			pixels[x][y] = 0;
			charge[x][y] = 0;
			metadata[x][y] = 0;
		}
	}
	int dx = direction == 1 ? 1 : (direction == 3 ? -1 : 0);
	dx *= distance;
	int dy = direction == 2 ? 1 : (direction == 0 ? -1 : 0);
	dy *= distance;
	for (int x = start.x+1; x <= end.x; x++) {
		for (int y = start.y+1; y <= end.y; y++) {
			pixels[x + dx][y + dy] = temp[x - start.x][y - start.y];
			charge[x + dx][y + dy] = tempCharge[x - start.x][y - start.y];
			metadata[x + dx][y + dy] = tempMeta[x - start.x][y - start.y];
		}
	}
	for (int i = 0; i < width; i++) {
		delete temp[i];
		delete tempCharge[i];
		delete tempMeta[i];
	}
	delete temp;
	delete tempCharge;
	delete tempMeta;

	selectionStart.x += dx;
	selectionStart.y += dy;
}