

#include "TileMap.h"
#include "electronics.h"
#include "coord.h"
#include "ScreenEditing.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <algorithm>
#include <ctime>
#include <thread>
#include <unordered_set>
using namespace std;

int width, height;

//HERE I GO AGAIN ON MY OWN
//0 - nothing
//1 - wire
//2 - battery
//3 - NOT gate
//4 - switch
//5 - wire-cross
//6 - signal blocker
//7 - delay
//8 - AND gate
//9 - XOR gate
//10 - Code tile
//11 - WIFI
//12 - Clock

const coord screenSize(900, 900);

int **pixels;
int **charge;
int **metadata;
bool canPlaceAgain = true;
list <coord> updateQueue;
list<coord> updateQueueForNextFrame;
std::vector<coord> codeTiles;
unordered_map<int, list<coord>*> wifiTiles;
unordered_map<int, bool> wifiCharge;

void load(string filename) {
	ifstream in;
	in.open(filename);
	int inSizeX, inSizeY;
	if (!in) {
		inSizeX = 100;
		inSizeY = 100;
	}
	else {
		in >> inSizeX;
		in >> inSizeY;
	}
	int desiredSizeX, desiredSizeY;
	desiredSizeX = 600;
	desiredSizeY = 600;
	width = desiredSizeX;
	height = desiredSizeY;
	pixels = new int*[desiredSizeX];
	charge = new int*[desiredSizeX];
	metadata = new int*[desiredSizeX];

	for (int i = 0; i < desiredSizeX; i++) {
		pixels[i] = new int[desiredSizeY];
		charge[i] = new int[desiredSizeY];
		metadata[i] = new int[desiredSizeY];
	}

	if (in) {
		for (int i = 0; i < inSizeX; i++) {
			for (int j = 0; j < inSizeY; j++) {
				in >> pixels[i][j];
				in >> charge[i][j];
				in >> metadata[i][j];
				if (pixels[i][j] < 0) {
					pixels[i][j] = 0;
				}
                if(pixels[i][j] == 10){
                    codeTiles.push_back(coord(i, j));
                }
                if(pixels[i][j] == 11){
                    int channel = metadata[i][j] >> 2;
                    list<coord>* wifiList;
                    if(wifiTiles.count(channel) != 1){
                        wifiList = new list<coord>();
                        wifiTiles.insert({channel, wifiList});
                    }else{
                        wifiList = wifiTiles[channel];
                    }
                    wifiList->push_back(coord(i, j));
                }
				if (charge[i][j] < 0) {
					charge[i][j] = 0;
				}
				if (metadata[i][j] < 0) {
					metadata[i][j] = 0;
				}
			}
		}
		if (desiredSizeX > inSizeX) {
			for (int i = inSizeX; i < desiredSizeX; i++) {
				for (int j = inSizeY; j < desiredSizeY; j++) {
					pixels[i][j] = 0;
					charge[i][j] = 0;
					metadata[i][j] = 0;
				}
			}
		}
		in.close();
	}
	else {
		for (int i = 0; i < desiredSizeX; i++) {
			for (int j = 0; j < desiredSizeY; j++) {
				pixels[i][j] = 0;
				charge[i][j] = 0;
				metadata[i][j] = 0;
			}
		}
	}
}

void save(string filename) {
	ofstream out;
	out.open(filename);
	if (out) {
		out << width << ' ';
		out << height << endl;
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				out << pixels[x][y] << ' ';
				out << charge[x][y] << ' ';
				out << metadata[x][y] << ' '<<endl;
			}
			delete[] pixels[x];
			delete[] charge[x];
			delete[] metadata[x];
		}
		out.close();
	}
}

void queueNeighborUpdates(coord tile, int toUpdate) {
	int x = tile.x;
	int y = tile.y;
	for (int i = 0; i < 4; i++) {
		if (toUpdate % 2 == 1) {
			coord neighbor = electronics::getNeighborCoord(i, x, y);
			if (std::find(updateQueue.begin(), updateQueue.end(), neighbor) == updateQueue.end()) {
				updateQueue.push_back(neighbor);
			}
		}
		toUpdate = toUpdate >> 1;
	}
}

void queueUpdate(coord tile) {
	if (std::find(updateQueue.begin(), updateQueue.end(), tile) == updateQueue.end()) {
		updateQueue.push_back(tile);
	}
}

void placeTile(const coord tile, int type, electronics& electronics, coord* previousTilePlaced = NULL) {
	pixels[tile.x][tile.y] = type;
	if (previousTilePlaced != NULL) {
		*previousTilePlaced = coord(tile.x, tile.y);
	}
	metadata[tile.x][tile.y] = 0;
	charge[tile.x][tile.y] = 0;
	electronics.initTile(tile.x, tile.y);
	queueUpdate(coord(tile.x, tile.y));
	queueNeighborUpdates(coord(tile.x, tile.y), 15);
}

void updateAndDrawGraphics(ScreenEditing* scn, TileMap* tileMap, TileMap* metadataTilemap, sf::RenderWindow* window, sf::Text* text, bool* shouldDrawMeta, bool* shouldUpdateTileMap) {
	while (window->isOpen()) {
		tileMap->update(*scn, pixels, charge);
		window->draw(*tileMap);
		if (*shouldDrawMeta) {
			window->draw(*metadataTilemap);
		}
		window->draw(*text);
		window->display();
	}
}

int main() {
  string saveName = "16bitcpu";
  string path = "/Users/aidensweezey/Documents/Electricity/";
	string file = path + saveName + ".txt";
	load(file);

	sf::RenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "Electricity");

    
	//Electronics
	electronics electronics(pixels, charge, metadata, width, height, &updateQueueForNextFrame, &codeTiles, path + saveName + "-code.txt", &wifiTiles, &wifiCharge);
	coord previousTilePlaced;

	//Metadata
	bool canChangeMeta = true;
	coord selectedTile;

	//Zoom Stuff
	ScreenEditing scn = ScreenEditing(width, height, screenSize);

	//Selection Stuff
	coord mouseStartedClicking;
	bool shouldStartNewSelection = true;
	bool canDoCommand = true;
	bool canDeleteArea = true;
	bool canTranslate = true;
	bool canRotate = true;

	//Display Stuff
	sf::Font font;
	TileMap tileMap;
	TileMap metadataTileMap;
	unsigned int* tileSizeStart = new unsigned int(((float)screenSize.x) / ((float)(width)));
	tileMap.update(scn, pixels, charge);
	delete tileSizeStart;
	bool shouldDrawMeta = false;
	bool shouldUpdateTileMap = false;
    
	if (!font.loadFromFile("/Users/aidensweezey/Documents/Electricity/Electricity/consola.ttf")) {
		cout << "Font not loaded";
	}
	sf::Text text;
	window.setActive(false);
	std::thread drawThread(updateAndDrawGraphics, &scn, &tileMap, &metadataTileMap, &window, &text, &shouldDrawMeta, &shouldUpdateTileMap);
    unordered_set<int> keypresses;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				save(file);
				window.close();
				exit(0);
			}
			else if (event.type == sf::Event::MouseWheelScrolled) {
				scn.tryZoom(event.mouseWheelScroll.delta);
				shouldUpdateTileMap = true;
            }else if(event.type == sf::Event::KeyPressed){
                keypresses.insert(event.key.code);
            }else if(event.type == sf::Event::KeyReleased){
                keypresses.erase(event.key.code);
                switch(event.key.code){
                    case sf::Keyboard::E:
                        canChangeMeta = true;
                        break;
                    case sf::Keyboard::Q:
                        canChangeMeta = true;
                        break;
                    case sf::Keyboard::C:
                        canDoCommand = true;
                        break;
                    case sf::Keyboard::X:
                        canDoCommand = true;
                        break;
                    case sf::Keyboard::V:
                        canDoCommand = true;
                        break;
                    case sf::Keyboard::R:
                        canRotate = true;
                        break;
                    case sf::Keyboard::J:
                        canRotate = true;
                        break;
                    case sf::Keyboard::H:
                        canRotate = true;
                        break;
                    case sf::Keyboard::Backspace:
                        canDeleteArea = true;
                        break;
                    case sf::Keyboard::Up:
                        canTranslate = true;
                        break;
                    case sf::Keyboard::Right:
                        canTranslate = true;
                        break;
                    case sf::Keyboard::Left:
                        canTranslate = true;
                        break;
                    case sf::Keyboard::Down:
                        canTranslate = true;
                        break;
                    case sf::Keyboard::LBracket:
                        canTranslate = true;
                        break;
                    case sf::Keyboard::RBracket:
                        canTranslate = true;
                        break;
                    default:
                        break;
                }
            }

		}
		text.setCharacterSize(18);
		text.setFont(font);
        bool shouldUpdateDesc = false;
		if (window.hasFocus()) {
			coord mousePos = sf::Mouse::getPosition(window);
			int tileX = mousePos.x / scn.getTileSize();
			int tileY = mousePos.y / scn.getTileSize();
			coord globalTileCoord = scn.translateScreenCoordToGlobalCoord(coord(tileX, tileY));
			tileX = globalTileCoord.x;
			tileY = globalTileCoord.y;
			if (tileX >= 0 && tileX < width && tileY >= 0 && tileY < height) {
				int i = 0;
                if (keypresses.find(sf::Keyboard::Num1) != keypresses.end()) {
					//Wire
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)){
                        i = 11;
                    }else{
                        i = 1;
                    }
				}
                if (keypresses.find(sf::Keyboard::Num2) != keypresses.end()) {
					//Battery
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)){
                        i = 12;
                    }else{
                        i = 2;
                    }
				}
                if (keypresses.find(sf::Keyboard::Num3) != keypresses.end()) {
					//Not
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)){
                        i = 13;
                    }else{
                        i = 3;
                    }
				}
                if (keypresses.find(sf::Keyboard::Num4) != keypresses.end()) {
					//Switch
					i = 4;
				}
                if (keypresses.find(sf::Keyboard::Num5) != keypresses.end()) {
					//Cross wire
					i = 5;
				}
                if (keypresses.find(sf::Keyboard::Num6) != keypresses.end()) {
					//Signal Blocker
					i = 6;
				}
                if (keypresses.find(sf::Keyboard::Num7) != keypresses.end()) {
					//Signal Blocker
					i = 7;
				}
                if (keypresses.find(sf::Keyboard::Num8) != keypresses.end()) {
					//Signal Blocker
					i = 8;
				}
                if (keypresses.find(sf::Keyboard::Num9) != keypresses.end()) {
					//Signal Blocker
					i = 9;
				}
                if (keypresses.find(sf::Keyboard::Num0) != keypresses.end()) {
                    //Code
                    i = 10;
                }

				if (i != 0 && pixels[tileX][tileY] != i) {
					coord newTile = coord(tileX, tileY);
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
						float sqrDist = coord::sqrDist(previousTilePlaced, newTile);
						for (float f = 0; f <= 1; f += ((float)1 / sqrDist)) {
							placeTile(coord::lerp(previousTilePlaced, newTile, f), i, electronics);
						}
						previousTilePlaced = newTile;
					}
					else {
						placeTile(newTile, i, electronics, &previousTilePlaced);
					}
					shouldUpdateTileMap = true;
				}

                if (keypresses.find(sf::Keyboard::Space) != keypresses.end() && pixels[tileX][tileY] != 0) {
                    electronics.tileDeleted(coord(tileX, tileY));
					placeTile(coord(tileX, tileY), 0, electronics);
					shouldUpdateTileMap = true;
				}
				if (canChangeMeta) {
                    int amountToChange = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 4 : (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) ? 16 : 1);
                    int prevMeta = metadata[tileX][tileY];
                    if (keypresses.find(sf::Keyboard::E) != keypresses.end()) {
						metadata[tileX][tileY] += amountToChange;
						canChangeMeta = false;
					}
                    if (keypresses.find(sf::Keyboard::Q) != keypresses.end()) {
						metadata[tileX][tileY] = std::max(0, prevMeta - amountToChange);
						canChangeMeta = false;
					}
					if (!canChangeMeta) {
                        electronics.tileMetaChanged(coord(tileX, tileY), prevMeta);
						updateQueue.push_back(coord(-tileX, -tileY));
					}
				}

				int moveDir = -1;
                if (keypresses.find(sf::Keyboard::A) != keypresses.end()) {
					moveDir = 3;
				}
                if (keypresses.find(sf::Keyboard::W) != keypresses.end()) {
					moveDir = 0;
				}
                if (keypresses.find(sf::Keyboard::S) != keypresses.end()) {
					moveDir = 2;
				}
                if (keypresses.find(sf::Keyboard::D) != keypresses.end()) {
					moveDir = 1;
				}
				if (moveDir != -1) {
					if (scn.tryMove(moveDir)) {
						shouldUpdateTileMap = true;
					}
				}
				else {
					scn.centerTile = vectorf((int)scn.centerTile.x, (int)scn.centerTile.y);
				}
				if (canTranslate) {
					moveDir = -1;
                    if (keypresses.find(sf::Keyboard::Left) != keypresses.end()) {
						moveDir = 3;
					}
                    if (keypresses.find(sf::Keyboard::Up) != keypresses.end()) {
						moveDir = 0;
					}
                    if (keypresses.find(sf::Keyboard::Down) != keypresses.end()) {
						moveDir = 2;
					}
                    if (keypresses.find(sf::Keyboard::Right) != keypresses.end()) {
						moveDir = 1;
					}
					if (moveDir != -1) {
						scn.moveSelection(moveDir, pixels, metadata, charge, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5 : 1);
						canTranslate = false;
						for (int x = scn.selectionStart.x; x < scn.selectionStart.x + scn.selectionSize.x; x++) {
							for (int y = scn.selectionStart.y; y < scn.selectionStart.y + scn.selectionSize.y; y++) {
								queueUpdate(coord(x, y));
							}
						}
						shouldUpdateTileMap = true;
					}
                    if (keypresses.find(sf::Keyboard::LBracket) != keypresses.end()) {
                        scn.tryZoom(-6);
                        canTranslate = false;
                    }else if (keypresses.find(sf::Keyboard::RBracket) != keypresses.end()) {
                        scn.tryZoom(6);
                        canTranslate = false;
                    }
				}
				if (canRotate && keypresses.find(sf::Keyboard::R) != keypresses.end()) {
					scn.rotate(pixels, metadata, charge);
					shouldUpdateTileMap = true;
					canRotate = false;
				}
				if (canRotate && keypresses.find(sf::Keyboard::H) != keypresses.end()) {
					scn.flipHorizontally(pixels, metadata, charge);
					shouldUpdateTileMap = true;
					canRotate = false;
				}
				if (canRotate && keypresses.find(sf::Keyboard::J) != keypresses.end()) {
					scn.flipVertically(pixels, metadata, charge);
					shouldUpdateTileMap = true;
					canRotate = false;
				}
				if (canDoCommand && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
					bool shouldQueueUpdates = false;
					if (keypresses.find(sf::Keyboard::C) != keypresses.end()) {
						scn.copy(pixels, metadata, charge, false);
						canDoCommand = false;
					}else if(keypresses.find(sf::Keyboard::X) != keypresses.end()) {
						scn.copy(pixels, metadata, charge, true);
						shouldQueueUpdates = true;
						canDoCommand = false;
					}else if (keypresses.find(sf::Keyboard::V) != keypresses.end()) {
						scn.paste(coord(tileX, tileY), pixels, metadata, charge);
						shouldQueueUpdates = true;
						canDoCommand = false;
					}
					if (shouldQueueUpdates) {
						shouldUpdateTileMap = true;
						for (int x = scn.selectionStart.x; x < scn.selectionStart.x + scn.selectionSize.x; x++) {
							for (int y = scn.selectionStart.y; y < scn.selectionStart.y + scn.selectionSize.y; y++) {
								queueUpdate(coord(x, y));
							}
						}
					}
				}

                if (canDeleteArea && keypresses.find(sf::Keyboard::Backspace) != keypresses.end()) {
					scn.deleteArea(pixels, metadata, charge);
					for (int x = scn.selectionStart.x; x < scn.selectionStart.x + scn.selectionSize.x; x++) {
						for (int y = scn.selectionStart.y; y < scn.selectionStart.y + scn.selectionSize.y; y++) {
							queueUpdate(coord(x, y));
						}
					}
					shouldUpdateTileMap = true;
					canDeleteArea = false;
				}

				if (pixels[tileX][tileY] >= 0) {
                    shouldUpdateDesc = true;
					selectedTile = coord(tileX, tileY);
					metadataTileMap.updateMetadata(sf::Vector2f(scn.getTileSize(), scn.getTileSize()), pixels[tileX][tileY], metadata[tileX][tileY], scn.zoom, width, height, sf::Vector2i(scn.getUpperLeftCorner().x, scn.getUpperLeftCorner().y), selectedTile);
					shouldDrawMeta = true;
				}
				else {
					shouldDrawMeta = false;
                    shouldUpdateDesc = selectedTile.x != -1;
					selectedTile = coord(-1, -1);
				}


				//Selection
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
					shouldDrawMeta = false;
					coord tempStart, tempSize;
					if (shouldStartNewSelection) {
						tempStart = coord(tileX, tileY);
						mouseStartedClicking = coord(tileX, tileY);
						shouldStartNewSelection = false;
					}
					else {
						tempStart = coord(mouseStartedClicking);
						if (tileX < mouseStartedClicking.x) {
							tempStart.x = tileX;
						}if(tileY < mouseStartedClicking.y) {
							tempStart.y = tileY;
						}
						tempSize = coord(abs(tileX - mouseStartedClicking.x), abs(tileY - mouseStartedClicking.y));
					}
					if (scn.updateSelection(tempStart, tempSize)) {
						shouldUpdateTileMap = true;
					}
				}
				else {
					shouldStartNewSelection = true;
				}
			}
		}
		int updatesThisLoop = 0;
		while (!updateQueue.empty() && updatesThisLoop < 100000) {
			coord tileToUpdate = updateQueue.front();
			updateQueue.pop_front();
			int toUpdate = electronics.updateTile(tileToUpdate);
			queueNeighborUpdates(coord(abs(tileToUpdate.x), abs(tileToUpdate.y)), toUpdate);
			if (toUpdate > 0) {
				shouldUpdateTileMap = true;
			}
			updatesThisLoop++;
		}
		while (!updateQueueForNextFrame.empty()) {
			queueUpdate(updateQueueForNextFrame.front());
			updateQueueForNextFrame.pop_front();
		}
        if(shouldUpdateDesc){
            std::ostringstream oss;
            oss<< updatesThisLoop << " Updates   SelectedTile: (" <<selectedTile.x<<","<<selectedTile.y<<")   Metadata: " << (selectedTile.x >= 0 ? metadata[selectedTile.x][selectedTile.y] : 0) << " Charge: " << (selectedTile.x >= 0 ? charge[selectedTile.x][selectedTile.y] : 0);
            oss << electronics.getExtraDesc(selectedTile);
            text.setString(oss.str());
        }
	}
	return 0;

}

