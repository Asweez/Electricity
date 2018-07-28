#ifndef TILEMAP_H_
#define TILEMAP_H_
#include <SFML\Graphics.hpp>
#include "ScreenEditing.h"
class TileMap : public sf::Drawable, public sf::Transformable {
public:
	sf::VertexArray m_vertices;
	void update(sf::Vector2f tileSize, int** tiles, int** charge, int zoom, sf::Vector2i upperLeftCornerPos, const int width, const int height, coord, coord);
	void update(ScreenEditing s, int** tiles, int** charge);
	void updateMetadata(sf::Vector2f tileSize, const int tile, const int meta, int zoom, const int width, const int height, coord upperLeftCorner, coord selectedTile);
private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	sf::Color getColor(int i, int charge);
};
#endif


