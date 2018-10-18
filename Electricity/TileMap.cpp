#include "TileMap.h"
#include "electronics.h"
#include <iostream>

void TileMap::update(sf::Vector2f tileSize, int** tiles, int** charge, int zoom, sf::Vector2i upperLeftCornerPos, const int width, const int height, coord selectionStart, coord selectionSize)
{
	// resize the vertex array to fit the level size
	m_vertices.setPrimitiveType(sf::Quads);
	m_vertices.resize(zoom * zoom * 16);

	// populate the vertex array, with one quad per tile
	for (unsigned int i = upperLeftCornerPos.x; i < upperLeftCornerPos.x + (2 * zoom); ++i)
		for (unsigned int j = upperLeftCornerPos.y; j < upperLeftCornerPos.y + (2 * zoom); ++j)
		{
			//The numbers in terms of the frame being drawn
			int iFrame = i - upperLeftCornerPos.x;
			int jFrame = j - upperLeftCornerPos.y;
			// get the current tile number

			// get a pointer to the current tile's quad
			sf::Vertex* quad = &m_vertices[(iFrame + (jFrame * zoom * 2)) * 4];

			// define its 4 corners
			quad[0].position = sf::Vector2f(iFrame * tileSize.x, jFrame * tileSize.y);
			quad[1].position = sf::Vector2f((iFrame + 1) * tileSize.x, jFrame * tileSize.y);
			quad[2].position = sf::Vector2f((iFrame + 1) * tileSize.x, (jFrame + 1) * tileSize.y);
			quad[3].position = sf::Vector2f(iFrame * tileSize.x, (jFrame + 1) * tileSize.y);

			sf::Color tileColor;
			if (i >= width - 1 || i <= 0 || j <= 0 || j >= height - 1) {
				tileColor = sf::Color::Blue;
			}
			else {
				int tileNumber = tiles[i][j];
				tileColor = getColor(tileNumber, charge[i][j]);
			}

			if (i > selectionStart.x && i <= selectionStart.x + selectionSize.x && j > selectionStart.y && j <= selectionStart.y + selectionSize.y) {
				tileColor += sf::Color(111, 111, 111);
			}

			quad[0].color = tileColor;
			quad[1].color = tileColor;
			quad[2].color = tileColor;
			quad[3].color = tileColor;
		}

}

void TileMap::update(ScreenEditing s, int** tiles, int** charge) {
	update(sf::Vector2f(s.getTileSize(), s.getTileSize()), tiles, charge, s.zoom, sf::Vector2i(s.getUpperLeftCorner().x, s.getUpperLeftCorner().y), s.width, s.height, s.selectionStart, s.selectionSize);
}

void TileMap::updateMetadata(sf::Vector2f tileSize, const int tile, const int meta, int zoom, const int width, const int height, coord upperLeftCorner, coord selectedTile) {

	m_vertices.setPrimitiveType(sf::Quads);
	m_vertices.resize(16);
	
	for (int i = 0; i < 4; i++) {
		sf::Vertex* quad = &m_vertices[i * 4];
		coord tileInFrameCoords = electronics::getNeighborCoord(i, selectedTile.x, selectedTile.y);
		tileInFrameCoords.x -= upperLeftCorner.x;
		tileInFrameCoords.y -= upperLeftCorner.y;
		quad[0].position = sf::Vector2f(tileInFrameCoords.x * tileSize.x, tileInFrameCoords.y * tileSize.y);
		quad[1].position = sf::Vector2f((tileInFrameCoords.x + 1) * tileSize.x, tileInFrameCoords.y * tileSize.y);
		quad[2].position = sf::Vector2f((tileInFrameCoords.x + 1) * tileSize.x, (tileInFrameCoords.y + 1) * tileSize.y);
		quad[3].position = sf::Vector2f(tileInFrameCoords.x * tileSize.x, (tileInFrameCoords.y + 1) * tileSize.y);

		sf::Color c = sf::Color(0, 0, 0, 0);
		switch (tile) {
		case 3:
			c = i == meta ? sf::Color(255, 100, 0) : sf::Color::Blue;
			break;
		case 4:
			if (i == meta) {
				c = sf::Color(255, 100, 0);
			}
			else if (i == (meta + 1) % 4) {
				c = sf::Color::Green;
			}
			else if(i == (meta + 2) % 4){
				c = sf::Color::Blue;
			}
			break;
		case 5:
			c = (i == meta % 4 || i == (meta + 1) % 4) ? sf::Color(255, 100, 0) : sf::Color::Blue;
			break;
		case 6:
			if (i == meta) {
				c = sf::Color(255, 100, 0);
			}
			else if (i == (meta + 1) % 4) {
				c = sf::Color::Green;
			}
			else if (i == (meta + 2) % 4) {
				c = sf::Color::Blue;
			}
			break;
		case 7:
			if (i == meta % 4) {
				c = sf::Color(255, 100, 0);
			}
			else if (i == (meta + 2) % 4) {
				c = sf::Color::Blue;
			}
			break;
		case 8:
			if (i == meta || i == (meta + 1) % 4) {
				c = sf::Color(255, 100, 0);
			}
			else if (i == (meta + 2) % 4) {
				c = sf::Color::Blue;
			}
			break;
		case 9:
			if (i == meta || i == (meta + 1) % 4) {
				c = sf::Color(255, 100, 0);
			}
			else if (i == (meta + 2) % 4) {
				c = sf::Color::Blue;
			}
            break;
        case 10:
            if(i == meta % 4){
                c = sf::Color(255, 100, 0);
            }else if(i == (meta + 2) % 4){
                c = sf::Color::Blue;
            }
            break;
		}

		quad[0].color = c;
		quad[1].color = c;
		quad[2].color = c;
		quad[3].color = c;
	}
}


void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// apply the transform
	states.transform *= getTransform();

	// draw the vertex array
	target.draw(m_vertices, states);
}
	

sf::Color TileMap::getColor(const int i, const int charge) {
	switch (i) {
	case 1:
		return charge == 0 ? sf::Color(200, 200, 200) : sf::Color::Yellow;
	case 2:
		return charge > 0 ? sf::Color::Green : sf::Color(0, 100, 0);
	case 3:
		return sf::Color::Red;
	case 4:
		return charge > 0 ? sf::Color(255, 100, 0) : sf::Color(0, 80, 80);
	case 5:
		return sf::Color(150, 0, 150);
	case 6:
		return charge > 0 ? sf::Color(150, 50, 255) : sf::Color(70, 0, 225);
	case 7:
		return charge > 0 ? sf::Color(255, 180, 0) : sf::Color(255, 100, 0);
	case 8:
		return charge > 0 ? sf::Color(0, 180, 0) : sf::Color(0, 100, 0);
	case 9:
		return charge > 0 ? sf::Color(180, 0, 0) : sf::Color(100, 0, 0);
    case 10:
        return charge > 0 ? sf::Color::Green : sf::Color(0, 100, 0);
	}
	return sf::Color::Black;
}
