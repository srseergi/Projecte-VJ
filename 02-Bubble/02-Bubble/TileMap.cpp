#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TileMap.h"


using namespace std;


TileMap *TileMap::createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	TileMap *map = new TileMap(levelFile, minCoords, program);
	
	return map;
}


TileMap::TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	loadLevel(levelFile);
	prepareArrays(minCoords, program);
}

TileMap::~TileMap()
{
	if(map != NULL)
		delete map;
}


void TileMap::render() const
{
	glEnable(GL_TEXTURE_2D);
	tilesheet.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6 * nTiles);
	glDisable(GL_TEXTURE_2D);
}

void TileMap::free()
{
	glDeleteBuffers(1, &vbo);
}

bool TileMap::loadLevel(const string& levelFile)
{
	ifstream fin(levelFile.c_str());
	if (!fin.is_open())
		return false;

	string line, tilesheetFile;
	stringstream sstream;

	// Comprobar encabezado
	getline(fin, line);
	if (line.compare(0, 7, "TILEMAP") != 0)
		return false;

	// Leer tama�o del mapa
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> mapSize.x >> mapSize.y;

	// Leer tileSize y blockSize
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> tileSize >> blockSize;

	// Leer nombre del fichero de tilesheet
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> tilesheetFile;
	tilesheet.loadFromFile(tilesheetFile, TEXTURE_PIXEL_FORMAT_RGBA);
	tilesheet.setWrapS(GL_CLAMP_TO_EDGE);
	tilesheet.setWrapT(GL_CLAMP_TO_EDGE);
	tilesheet.setMinFilter(GL_NEAREST);
	tilesheet.setMagFilter(GL_NEAREST);

	// Leer tama�o del tilesheet
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> tilesheetSize.x >> tilesheetSize.y;
	tileTexSize = glm::vec2(1.f / tilesheetSize.x, 1.f / tilesheetSize.y);

	// Reservar memoria para el mapa
	map = new int[mapSize.x * mapSize.y];

	// Leer el mapa l�nea por l�nea, esperando valores separados por comas.
	for (int j = 0; j < mapSize.y; j++)
	{
		getline(fin, line); // Leer una l�nea completa del mapa.
		stringstream lineStream(line);
		for (int i = 0; i < mapSize.x; i++)
		{
			int value;
			lineStream >> value;
			// Si el valor es -1, se asigna tile vac�o (0).
			if (value == -1)
				map[j * mapSize.x + i] = 0;
			else
				map[j * mapSize.x + i] = value;
			// Se ignora la coma entre n�meros, excepto tras el �ltimo valor de la l�nea.

			if (i < mapSize.x - 1)
			{
				if (lineStream.peek() == ',')
					lineStream.ignore();
			}
		}
	}

	fin.close();
	return true;
}


void TileMap::prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program)
{
	int tile;
	glm::vec2 posTile, texCoordTile[2], halfTexel;
	vector<float> vertices;
	nTiles = 0;
	halfTexel = glm::vec2(0.5f / tilesheet.width(), 0.5f / tilesheet.height());
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			tile = map[j * mapSize.x + i];
			if(tile != 0)
			{
				// Non-empty tile
				nTiles++;
				posTile = glm::vec2(minCoords.x + i * tileSize, minCoords.y + j * tileSize);
				texCoordTile[0] = glm::vec2(float((tile)%tilesheetSize.x) / tilesheetSize.x, float((tile)/tilesheetSize.x) / tilesheetSize.y);
				texCoordTile[1] = texCoordTile[0] + tileTexSize;
				//texCoordTile[0] += halfTexel;
				//texCoordTile[1] -= halfTexel;
				// First triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				// Second triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				vertices.push_back(posTile.x); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[1].y);
			}
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * nTiles * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	posLocation = program.bindVertexAttribute("position", 2, 4*sizeof(float), 0);
	texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4*sizeof(float), (void *)(2*sizeof(float)));
}

// Collision tests for axis aligned bounding boxes.
// Method collisionMoveDown also corrects Y coordinate if the box is
// already intersecting a tile below.

bool TileMap::collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;
	
	x = pos.x / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;

	std::vector<int> collidableTiles = { 13, 24, 35, 30, 41, 32, 43, 61 };

	for(int y=y0; y<=y1; y++)
	{
		int tile = map[y * mapSize.x + x];
		if(std::find(collidableTiles.begin(), collidableTiles.end(), tile) != collidableTiles.end())
			return true;
	}
	
	return false;
}

bool TileMap::collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;
	
	x = (pos.x + size.x - 1) / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;

	std::vector<int> collidableTiles = { 11, 22, 33, 29, 40, 65 };

	for(int y=y0; y<=y1; y++)
	{
		int tile = map[y * mapSize.x + x];
		if(std::find(collidableTiles.begin(), collidableTiles.end(), tile) != collidableTiles.end())
			return true;
	}
	
	return false;
}

bool TileMap::collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	int x0, x1, y;
	
	x0 = pos.x / tileSize;
	x1 = (pos.x + size.x - 1) / tileSize;
	y = (pos.y + size.y - 1) / tileSize;

	std::vector<int> collidableTiles = { 6, 7, 11, 12, 13, 47, 48, 49, 50, 51, 52, 53, 54, 31, 32, 29, 30, 20, 21, 25, 26, 27 };

	for(int x=x0; x<=x1; x++)
	{
		int tile = map[y * mapSize.x + x];
		if(std::find(collidableTiles.begin(), collidableTiles.end(), tile) != collidableTiles.end())
		{
			if(*posY - tileSize * y + size.y <= 6)
			{
				*posY = tileSize * y - size.y;
				return true;
			}
		}
	}
	
	return false;
}