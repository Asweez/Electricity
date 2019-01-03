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
    default:
        return charge[x][y];
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
    case 10:
        if(neighbor != (tileMeta) % 4) return 0;
        return tileCharge;
    case 11:
        if(neighbor != (tileMeta) % 4) return 0;
        return tileCharge;
    case 12:
        if(neighbor != (tileMeta + 2) % 4) return 0;
        return tileCharge % 2;
    case 13:
        if(pixels[x1][y1] == 13){
            return tileCharge;
        }
        return 0;
	}
}

bool electronics::updateTile(coord tile) {
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
    int index = tileMeta >> 2;
	switch (pixels[x][y]) {
	case 0:
		charge[x][y] = 0;
		break;
    case 13:
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
				if (pixels[nC.x][nC.y] == 1 || pixels[nC.x][nC.y] == 13) {
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
					charge[x][y] = (tileMeta >> 2) * 8;
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
        neighborCharge = getNeighborCharge((tileMeta) % 4, x, y);

        if ((getNeighborCharge((tileMeta+1) % 4, x, y) > 0 ||neighborCharge > 0) && !(neighborCharge > 0 &&  getNeighborCharge((tileMeta+1) % 4, x, y) > 0)) {
			charge[x][y] = 1;
		}
		else {
			charge[x][y] = 0;
		}
		break;
    case 10:{
        if(std::find(codeTiles->begin(), codeTiles->end(), tile) == codeTiles->end()){
            codeTiles->push_back(tile);
        }
        neighborCharge = getNeighborCharge(tileMeta % 4, x, y);
        int prevCodeLine = codeLine;
        if(neighborCharge == 0){
            codeLine &= INTMAX_MAX - (int)(std::pow(2, index));
        }else{
            codeLine |= (int)(std::pow(2, index));
        }
        if(prevCodeLine != codeLine){
            for(int i = 0; i < codeTiles->size(); i++){
                if((*codeTiles)[i] != tile){
                    nextFrameUpdateQueue->push_back((*codeTiles)[i]);
                }
            }
        }
        ifstream is;
        is.open(codeFileName);
        if(is){
            for(int i = 0; i <= codeLine; i++){
                string str;
                if(std::getline(is, str)){//Parse a new line each time
                    if(i == codeLine){//Until we reach the specified line of code
                        if(str.length() <= index){
                            charge[x][y] = 0;
                            break;
                        }
                        if(str.at(index) == '1'){//If the bit of that line of code (specified by the metadata) is 1 we output charge
                            charge[x][y] = 1;
                            break;
                        }
                        charge[x][y] = 0;
                        break;
                    }
                }else{
                    charge[x][y] = 0;
                    break;
                }
            }
        }
        is.close();
        break;
    }case 11:
        neighborCharge = getNeighborCharge(tileMeta % 4, x, y);
        //THIS SHOULD BE HANDLED IN tileMetaChanged but in case for some reason this tile is in an empty channel
        if(wifiTiles->find(index) == wifiTiles->end()){
            //If there are no tiles already on this channel, make a new list for the channel
            list<coord>* l = new list<coord>();
            l->push_back(tile);
            wifiTiles->insert({index, l});
        }
        //THIS SHOULD ALSO BE HANDLED IN tileMetaChanged
        if(wifiCharges->find(index) == wifiCharges->end()){
            //If we don't currently have a stored charge for this channel
            wifiCharges->insert({index, false});
        }
        //Charge 1 will mean that this tile is being powered from an outside source, charge 2 means it's being powered over the network
        if(neighborCharge > 0){
            charge[x][y] = 1;
            if(!(*wifiCharges)[index]){
                (*wifiCharges)[index] = true;
                //Update other tiles in this channel
                for(auto const& i : *(*wifiTiles)[index]){
                    nextFrameUpdateQueue->push_back(i);
                }
            }
        }else{
            //If we aren't being powered directly
            if(tileCharge == 1){
                //If we were previously being powered directly then we need to check the network to see if the network is still powered from somewhere else
                charge[x][y] = 0;
                (*wifiCharges)[index] = false;//Reset charge temporarily
                for(auto const& i : *(*wifiTiles)[index]){
                    if(charge[i.x][i.y] == 1){
                        //We found another direct power source so no updates are necessary
                        (*wifiCharges)[index] = true;
                        charge[x][y] = 2;
                        break;
                    }
                }
                if(!(*wifiCharges)[index]){
                    //If we didn't find any other tile being directly powered we need to tell the rest of the tiles
                    for(auto const& i : *(*wifiTiles)[index]){
                        nextFrameUpdateQueue->push_back(i);
                    }
                }
            }else{
                //If we were not a power provider, set the charge based on the network because this tile will not cause any network updates
                if(!(*wifiCharges)[index]){
                    charge[x][y] = 0;
                }else{
                    charge[x][y] = 2;
                }
            }
        }
        break;
    case 12:
        //Bit 1 of charge is the actual output, the rest are just the countdown
        if(tileCharge >> 1 == 0){
            int delay = (tileMeta >> 2) * 4;
            charge[x][y] = (delay << 1) + ((tileCharge % 2) ^ 1);
        }else{
            charge[x][y] = tileCharge - 2;
        }
        nextFrameUpdateQueue->push_back(tile);
        break;
	}
	if (pixels[x][y] == 7) {
		return (charge[x][y] & 1) != (tileCharge & 1);
    }
	else {
		if (tileCharge != charge[x][y] || forceUpdate) {
			return true;
		}
	}
	return false;
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
    case 10:
        codeTiles->push_back(coord(x, y));
        break;
    case 11:{
        int defaultChannel = 114;
        metadata[x][y] = defaultChannel << 2;
        if(wifiTiles->find(defaultChannel) == wifiTiles->end()){
            wifiTiles->insert({defaultChannel, new list<coord>()});
        }
        (*wifiTiles)[defaultChannel]->push_back(coord(x, y));
        break;
    }
	}
}

void electronics::tileDeleted(coord tile){
    int pos;
    switch(pixels[tile.x][tile.y]){
        case 10:
            codeLine &= INTMAX_MAX - (int)(std::pow(2, metadata[tile.x][tile.y] >> 2));
            pos = -1;
            for(int i = 0; i < codeTiles->size(); i++){
                if((*codeTiles)[i] == tile){
                    pos = i;
                }else{
                    nextFrameUpdateQueue->push_back((*codeTiles)[i]);
                }
            }
            if(pos != -1){
                codeTiles->erase(codeTiles->begin() + pos);
            }
            break;
        case 11:
            list<coord>* prevTiles = (*wifiTiles)[metadata[tile.x][tile.y] >> 2];
            prevTiles->remove(tile);
            if(prevTiles->size() == 0){
                //If there are no more tiles in channel this we don't want to keep the empty list
                delete prevTiles;//Free up that list
                wifiTiles->erase(metadata[tile.x][tile.y] >> 2);//Erase pointer from wifiTiles
                wifiCharges->erase(metadata[tile.x][tile.y] >> 2);
            }else if(charge[tile.x][tile.y] == 1){
                //If there are still wifi tiles in this channel
                //Reset charge on old channel
                (*wifiCharges)[metadata[tile.x][tile.y] >> 2] = false;
                //Update all tiles on old channel
                for(auto const& i : *(*wifiTiles)[metadata[tile.x][tile.y] >> 2]){
                    nextFrameUpdateQueue->push_back(i);
                }
            }
            break;
    }
}

void electronics::tileMetaChanged(coord tile, int prevMeta){
    const int newMeta = metadata[tile.x][tile.y];
    switch(pixels[tile.x][tile.y]){
        case 11:{
            if(newMeta % 4 != prevMeta % 4) return;
            //Remove tile from old channel
            list<coord>* prevTiles = (*wifiTiles)[prevMeta >> 2];
            prevTiles->remove(tile);
            if(prevTiles->size() == 0){
                //If there are no more tiles in channel this we don't want to keep the empty list
                delete prevTiles;//Free up that list
                wifiTiles->erase(prevMeta >> 2);//Erase pointer from wifiTiles
                wifiCharges->erase(prevMeta >> 2);
            }else{
                //If there are still wifi tiles in this channel
                //Reset charge on old channel
                (*wifiCharges)[prevMeta >> 2] = false;
                //Update all tiles on old channel
                for(auto const& i : *(*wifiTiles)[prevMeta >> 2]){
                    nextFrameUpdateQueue->push_back(i);
                }
            }
            if(wifiTiles->find(newMeta >>2) == wifiTiles->end()){
                //If no list exists for the new channel, make a new one
                wifiTiles->insert({newMeta >> 2, new list<coord>()});
            }
            if(wifiCharges->find(newMeta >> 2) == wifiCharges->end()){
                //If no charge exists for the new channel, make a new mapping
                wifiCharges->insert({newMeta >> 2, false});
            }
            //Add this tile to the list of tiles in the channel
            (*wifiTiles)[newMeta >> 2]->push_back(tile);
            //We already have an update queued for this tile so we will eventually update charge and send updates to all the other tiles in this channel
            break;
        }
    }
}

string electronics::getExtraDesc(coord tile){
    string str = "";
    switch(pixels[tile.x][tile.y]){
        case 7:
            str = " Delay: ";
            str += std::to_string((metadata[tile.x][tile.y]>>2) * 4);
            break;
        case 10:
            str = " Bit: ";
            str += std::to_string(metadata[tile.x][tile.y] >> 2);
            break;
        case 11:
            str =  " Channel: ";
            str += std::to_string(metadata[tile.x][tile.y] >> 2);
            str += " ToC: ";
            str += std::to_string((*wifiTiles)[metadata[tile.x][tile.y] >> 2]->size());
            str += " CC: ";
            str += std::to_string((*wifiCharges)[metadata[tile.x][tile.y] >> 2]);
            break;
        case 12:
            str = " Delay: ";
            str += std::to_string((metadata[tile.x][tile.y]>>2) * 4);
            break;
    }
    return str;
}


