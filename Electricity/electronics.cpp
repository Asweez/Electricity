#include "electronics.h"

coord electronics::getNeighborCoord(int neighbor, int x, int y, int distance) {
	switch (neighbor) {
	case 0:
		return coord(x, y - distance);
	case 1:
		return coord(x + distance, y);
	case 2:
		return coord(x, y + distance);
	case 3:
		return coord(x - distance, y);
	}
	return coord(-1, -1);
}

int electronics::getNeighborCharge(const int neighbor, const int x1, const int y1) {
	coord neighborCoord = getNeighborCoord(neighbor, x1, y1);
	int x = neighborCoord.x;
	int y = neighborCoord.y;
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return 0;
	}
	int tileMeta = metadata[x][y];
	int tileCharge = charge[x][y];
	switch (pixels[x][y])
	{
	case 3:
		if (neighbor == (tileMeta + 2) % 4) {
			return 0;
		}
		else {
			return charge[x][y];
		}
	case 4:
		if (neighbor != tileMeta) {
			return 0;
		}
		else {
			return charge[x][y];
		}
	case 5:
		if (neighbor == (tileMeta + 2) % 4 || neighbor == (tileMeta + 3) % 4) {
			return 0;
		}
		else {
			if (neighbor == tileMeta) {
				return tileCharge & 1;
			}
			else if (neighbor == (tileMeta + 1) % 4) {
				return (tileCharge >> 1) & 1;
			}
		}
	case 6:
		if (neighbor == (tileMeta) % 4) {
			return tileCharge;
		}
		else {
			return 0;
		}
		break;
	case 7:
		if (neighbor == (tileMeta) % 4) {
			return (tileCharge & 1);
		}
		else {
			return 0;
		}
		break;
	case 8:
		if (neighbor == (tileMeta) % 4) {
			return tileCharge;
		}
		else {
			return 0;
		}
		break;
	case 9:
		if (neighbor == (tileMeta) % 4) {
			return tileCharge;
		}
		else {
			return 0;
		}
	default:
		return charge[x][y];
	}
}

int electronics::updateTile(coord tile) {
	int x = tile.x;
	int y = tile.y;
	bool forceUpdate = false;
	if (x < 0 && y < 0) {
		forceUpdate = true;
		x = abs(x);
		y = abs(y);
	}
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return false;
	}
	int tileCharge = charge[x][y];
	int lowestNeighboringCharge = 0;
	int tileMeta = metadata[x][y];
	int neighborCharge;
	switch (pixels[x][y]) {
	case 0:
		charge[x][y] = 0;
		break;
	case 1:
		for (int i = 0; i < 4; i++) {
			int neighborCharge = getNeighborCharge(i, x, y);
			if (neighborCharge > 0) {
				if (lowestNeighboringCharge == 0) {
					lowestNeighboringCharge = neighborCharge;
				}
				else if (neighborCharge < lowestNeighboringCharge) {
					lowestNeighboringCharge = neighborCharge;
				}
			}
			else if(tileCharge > 0){
				coord nC = getNeighborCoord(i, x, y);
				if (pixels[nC.x][nC.y] == 1) {
					forceUpdate = true;
				}
			}
		}
		if (lowestNeighboringCharge == 0) {
			charge[x][y] = 0;
			break;
		}
		if (tileCharge > 0 && tileCharge < lowestNeighboringCharge) {
			charge[x][y] = 0;
			break;
		}
		if (tileCharge == 0 || tileCharge >= lowestNeighboringCharge) {
			charge[x][y] = lowestNeighboringCharge + 1;
		}
		break;
	case 3:
		neighborCharge = getNeighborCharge(tileMeta, x, y);
		if (neighborCharge == 0) {
			charge[x][y] = 1;
		}
		else {
			charge[x][y] = 0;
		}
		break;
	case 4:
		//meta is input
		//meta + 1 is switch
		neighborCharge = getNeighborCharge((tileMeta + 1) % 4, x, y);
		if (neighborCharge) {
			charge[x][y] = getNeighborCharge(tileMeta, x, y) > 0 ? 1 : 0;
		}
		break;
	case 5:
		charge[x][y] = 0;
		if (getNeighborCharge(tileMeta, x, y) > 0) {
			charge[x][y]++;
		}
		if (getNeighborCharge((tileMeta + 1) % 4, x, y) > 0) {
			charge[x][y] += 2;
		}
		break;
	case 6:
		neighborCharge = getNeighborCharge((tileMeta) % 4, x, y);
		if (neighborCharge > 0 && getNeighborCharge((tileMeta + 1) % 4, x, y) == 0) {
			charge[x][y] = 1;
		}
		else {
			charge[x][y] = 0;
		}
		break;
	case 7:
		//So the first two bits of the meta is going to be the rotation, and the bits after that are the delay
		//The bits after first bit of charge are the delay counter, once the counter reaches zero it switches output charge to opposite
		//First bit of charge is the output charge
		if ((tileCharge >> 1) > 0) {
			if ((tileCharge >> 1) == 1) {
				//switch the charge
				charge[x][y] ^= 1;
				charge[x][y] -= 2;
			}
			else {
				charge[x][y] -= 2;
				nextFrameUpdateQueue->push_back(tile);
			}
		}
		neighborCharge = getNeighborCharge((tileMeta) % 4, x, y);
		if ((neighborCharge > 0 && (tileCharge & 1) == 0) || (neighborCharge == 0 && (tileCharge & 1) == 1)) {
			if (tileCharge >> 1 == 0) {
				//If charges are different and we are not delaying yet, set delay to max and make sure charge is opposite of neighbor still
				if ((tileMeta >> 2) > 0) {
					charge[x][y] = (tileMeta >> 2) * 2;
					charge[x][y] += neighborCharge > 0 ? 0 : 1;
					nextFrameUpdateQueue->push_back(tile);
				}
				else {
					charge[x][y] = neighborCharge > 0 ? 1 : 0;
				}
			}
		}
		else if((tileCharge >> 1) > 0) {
			//If charges are the same, set delay to zero and charge to equal to neighbor
			charge[x][y] = neighborCharge > 0 ? 1 : 0;
		}
		break;
	case 8:
		if (getNeighborCharge((tileMeta + 1) % 4, x, y) > 0 && getNeighborCharge((tileMeta) % 4, x, y) > 0) {
			charge[x][y] = 1;
		}
		else {
			charge[x][y] = 0;
		}
		break;
	case 9:
		if ((getNeighborCharge((tileMeta + 1) % 4, x, y) & 1) ^ (getNeighborCharge((tileMeta) % 4, x, y) & 1) == 1) {
			charge[x][y] = 1;
		}
		else {
			charge[x][y] = 0;
		}
		break;
	}
	if (forceUpdate) return 15;
	if (tileCharge == charge[x][y] && tileMeta == metadata[x][y]) return 0;
	return getNeighborsToUpdate(tile, tileCharge, tileMeta, charge[x][y], metadata[x][y]);
}

/*
	Returns a number that contains all updates to do (2 bits per direction)
*/
int electronics::getNeighborsToUpdate(coord tile, int prevCharge, int prevMeta, int newCharge, int newMeta) {
	int toUpdate = 0;
	switch (pixels[tile.x][tile.y]) {
	default:
		return 15;
	case 1:
		for (int i = 0; i < 4; i++) {
			coord n = getNeighborCoord(i, tile.x, tile.y);
			if (pixels[n.x][n.y] == 1) {
				if (charge[n.x][n.y] > newCharge || charge[n.x][n.y] == 0) {
					addNeighborToUpdate(i, &toUpdate);
				}
			}
			else if (pixels[n.x][n.y] != 0) {
				addNeighborToUpdate(i, &toUpdate);
			}
		}
		return toUpdate;
	case 3:
		return 15 - std::pow(2, newMeta % 4);
	case 5:
		if (prevMeta == newMeta) {
			toUpdate = 0;
			if (prevCharge % 2 != newCharge % 2) {
				addNeighborToUpdate((newMeta + 2) % 4, &toUpdate);
			}
			if ((prevCharge >> 1) != (newCharge >> 1)) {
				addNeighborToUpdate((newMeta + 3) % 4, &toUpdate);
			}
			return toUpdate;
		}
		else {
			return 15;
		}
	}
}

void electronics::addNeighborToUpdate(int neighbor, int *currentToUpdate) {
	if (neighbor == 0) {
		*currentToUpdate |= 1;
	}
	else if (neighbor == 1) {
		*currentToUpdate |= 2;
	}
	else if (neighbor == 2) {
		*currentToUpdate |= 4;

	}
	else if (neighbor == 3) {
		*currentToUpdate |= 8;
	}
}

void electronics::initTile(int x, int y) {
	switch (pixels[x][y])
	{
	case 2:
		charge[x][y] = 1;
		break;
	case 5:
		metadata[x][y] = 3;
		break;
	case 7:
		metadata[x][y] = 2000;
		break;
	}
}