#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include "Troll.h" 


#define SCREEN_X 0
#define SCREEN_Y 0

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 10


Scene::Scene()
{
	back = NULL;
	map = NULL;
	player = NULL;
	troll1 = NULL;
	troll2 = NULL;
}

Scene::~Scene()
{
	if (back != NULL)
		delete back;
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
	if (troll1 != NULL) 
		delete troll1;
	if (troll2 != NULL) 
		delete troll2;
}


void Scene::init()
{
	initShaders();
	back = TileMap::createTileMap("levels/VJTileMapv2._Fondo.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	map = TileMap::createTileMap("levels/VJTileMapv2._Mapa.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);

	troll1 = new Troll();
	troll1->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	troll1->setPosition(glm::vec2(17 * map->getTileSize(), 6 * map->getTileSize()));
	troll1->setTileMap(map);

	troll2 = new Troll();
	troll2->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	troll2->setPosition(glm::vec2(31 * map->getTileSize(), 2 * map->getTileSize()));
	troll2->setTileMap(map);

	troll3 = new Troll(); 
	troll3->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	troll3->setPosition(glm::vec2(44 * map->getTileSize(), 0 * map->getTileSize()));
	troll3->setTileMap(map);

	troll4 = new Troll();  
	troll4->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	troll4->setPosition(glm::vec2(52 * map->getTileSize(), 0 * map->getTileSize()));
	troll4->setTileMap(map);


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
	troll1->update(deltaTime, player->getPosition());
	troll2->update(deltaTime, player->getPosition());
	troll3->update(deltaTime, player->getPosition());
	troll4->update(deltaTime, player->getPosition());  
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
	troll1->render();
	troll2->render();
	troll3->render(); 
	troll4->render();
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



