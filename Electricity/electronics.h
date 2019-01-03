#pragma once
#ifndef ELECTRONICS_H
#define ELECTRONICS_H
#include "coord.h"
#include <stdlib.h>
#include <list>
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_map>
using namespace std;

class electronics {
public:
	electronics(int** pixelArray, int** chargeArray, int** metadataArray, int width1, int height1, list<coord>* queue, vector<coord>* codeTileVector, string codeFile, unordered_map<int, list<coord>*>* wifiTileMap, unordered_map<int, bool>* wifiChargeMap) {
		pixels = pixelArray;
		charge = chargeArray;
		metadata = metadataArray;
		width = width1;
		height = height1;
		nextFrameUpdateQueue = queue;
        codeFileName = codeFile;
        codeTiles = codeTileVector;
        wifiTiles = wifiTileMap;
        wifiCharges = wifiChargeMap;
	}
	int getNeighborCharge(const int neighbor, const int x1, const int y1);
	static coord getNeighborCoord(int neighbor, int x, int y, int distance = 1);
	bool updateTile(coord tile);
	void initTile(int x, int y);
    void tileDeleted(coord tile);
    void tileMetaChanged(coord tile, int prevMeta);
    string getExtraDesc(coord tile);

private:
	int** pixels;
	int** charge;
	int** metadata;
	list<coord>* nextFrameUpdateQueue;
    std::vector<coord> *codeTiles;
    unordered_map<int, list<coord>*>* wifiTiles;
    unordered_map<int, bool>* wifiCharges;
    int width, height;
    string codeFileName;
    int codeLine = 0;
};

#endif
