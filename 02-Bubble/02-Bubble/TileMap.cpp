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
    ifstream fin(levelFile);
    if (!fin.is_open())
        return false;

    string line, tilesheetFile;
    stringstream sstream;

    getline(fin, line);
    if (line.compare(0, 7, "TILEMAP") != 0)
        return false;

    getline(fin, line);
    sstream.str(line);
    sstream >> mapSize.x >> mapSize.y;
    sstream.clear();

    getline(fin, line);
    sstream.str(line);
    sstream >> tileSize >> blockSize;
    sstream.clear();

    getline(fin, line);
    sstream.str(line);
    sstream >> tilesheetFile;
    sstream.clear();

    tilesheet.loadFromFile(tilesheetFile, TEXTURE_PIXEL_FORMAT_RGBA);
    tilesheet.setWrapS(GL_CLAMP_TO_EDGE);
    tilesheet.setWrapT(GL_CLAMP_TO_EDGE);
    tilesheet.setMinFilter(GL_NEAREST);
    tilesheet.setMagFilter(GL_NEAREST);

    getline(fin, line);
    sstream.str(line);
    sstream >> tilesheetSize.x >> tilesheetSize.y;
    sstream.clear();

    tileTexSize = glm::vec2(1.f / tilesheetSize.x, 1.f / tilesheetSize.y);

    map = new int[mapSize.x * mapSize.y];

    for (int j = 0; j < mapSize.y; j++) {
        getline(fin, line);
        int i = 0, idx = 0;
        while (i < line.size() && idx < mapSize.x) {
            if (line[i] == '-') { // Detectar "-1" y asignar 0
                map[j * mapSize.x + idx] = 0;
                i += 2; // Saltar "-1"
            }
            else if (isdigit(line[i])) {
                int value = line[i] - '0';
                while (i + 1 < line.size() && isdigit(line[i + 1])) {
                    value = value * 10 + (line[i + 1] - '0');
                    i++;
                }
                map[j * mapSize.x + idx] = value;
                i++;
            }
            else {
                i++; // Ignorar caracteres no v�lidos
            }
            idx++;
        }
    }

    fin.close();
    return true;
}

void TileMap::prepareArrays(const glm::vec2& minCoords, ShaderProgram& program)
{
    int tile;
    glm::vec2 posTile, texCoordTile[2], halfTexel;
    vector<float> vertices;

    nTiles = 0;
    halfTexel = glm::vec2(0.5f / tilesheet.width(), 0.5f / tilesheet.height());

    for (int j = 0; j < mapSize.y; j++) {
        for (int i = 0; i < mapSize.x; i++) {
            tile = map[j * mapSize.x + i];
            if (tile > 0) { // Evitar procesar tiles vac�os (0 de "-1")
                nTiles++;
                posTile = glm::vec2(minCoords.x + i * tileSize, minCoords.y + j * tileSize);

                // C�lculo corregido para tilesheet de 11x7 tiles
                int tileRow = (tile - 1) / 11;  // Fila del tile en el tileset
                int tileCol = (tile - 1) % 11;  // Columna del tile en el tileset

                texCoordTile[0] = glm::vec2(float(tileCol) / tilesheetSize.x, float(tileRow) / tilesheetSize.y);
                texCoordTile[1] = texCoordTile[0] + tileTexSize;
                texCoordTile[1] -= halfTexel;

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
    posLocation = program.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
    texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void*)(2 * sizeof(float)));
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
	for(int y=y0; y<=y1; y++)
	{
		if(map[y*mapSize.x+x] != 0)
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
	for(int y=y0; y<=y1; y++)
	{
		if(map[y*mapSize.x+x] != 0)
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
	for(int x=x0; x<=x1; x++)
	{
		if(map[y*mapSize.x+x] != 0)
		{
			if(*posY - tileSize * y + size.y <= 4)
			{
				*posY = tileSize * y - size.y;
				return true;
			}
		}
	}
	
	return false;
}






























