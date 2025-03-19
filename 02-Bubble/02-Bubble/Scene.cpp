#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define SCREEN_X 0
#define SCREEN_Y 0

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 10


Scene::Scene()
{
	back = NULL;
	map = NULL;
	player = NULL;
}

Scene::~Scene()
{
	if (back != NULL)
		delete back;
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
}


void Scene::init()
{
	initShaders();
	back = TileMap::createTileMap("levels/Fondo.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	map = TileMap::createTileMap("levels/Mapa.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;
}

void Scene::cameraUpdate() {
	// Obtener la posición del jugador
	glm::vec2 playerPos = player->getPosition();

	float camX = playerPos.x - SCREEN_WIDTH / 2.0f;
	float camY = playerPos.y - SCREEN_HEIGHT / 2.0f;

	// Obtener el tamaño del mapa en píxeles
	glm::ivec2 mapSize = map->getMapSize();
	float maxCamX = glm::max(0.0f, float(mapSize.x) - SCREEN_WIDTH);  
	float maxCamY = glm::max(0.0f, float(mapSize.y) - SCREEN_HEIGHT); 

	// Limitar la cámara dentro de los bordes del mapa
	camX = glm::clamp(camX, 0.0f, maxCamX);
	camY = glm::clamp(camY, 0.0f, maxCamY);

	// Actualizar la proyección con la nueva posición de la cámara
	projection = glm::ortho(camX, camX + SCREEN_WIDTH, camY + SCREEN_HEIGHT, camY);
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
	player->update(deltaTime);
	cameraUpdate();
}

void Scene::render()
{
	glm::mat4 modelview;

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	back->render();
	map->render();
	player->render();
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}



