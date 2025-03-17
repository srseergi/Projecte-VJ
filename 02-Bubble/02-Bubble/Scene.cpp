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
	// Obtener la posici�n del jugador
	glm::vec2 playerPos = player->getPosition();

	// Tama�o de cada tile en p�xeles
	float tileSize = map->getTileSize();

	// Tama�o del �rea visible en p�xeles (16x16 tiles)
	float viewWidth = 16.0f * tileSize;
	float viewHeight = 16.0f * tileSize;

	// Posici�n horizontal: sigue al jugador normalmente
	float camX = playerPos.x - viewWidth / 2.0f;

	// Limitar la c�mara dentro de los bordes del mapa en el eje X
	float maxCamX = map->getMapSize().x * tileSize - viewWidth;
	camX = glm::clamp(camX, 0.0f, maxCamX);

	// **GESTI�N DEL MOVIMIENTO VERTICAL DE LA C�MARA**
	static float currentCameraY = 0.0f; // Altura actual de la c�mara
	float targetCameraY = currentCameraY; // Altura objetivo

	// Umbral para detectar una ca�da que debe mover la c�mara
	float thresholdDown = viewHeight * 0.4f; // 40% desde arriba

	// Si el personaje cae en una nueva zona, movemos la c�mara suavemente
	if (playerPos.y > currentCameraY + viewHeight - thresholdDown) {
		targetCameraY = playerPos.y - viewHeight * 0.6f; // Mantiene vista arriba
	}

	// Interpolaci�n para hacer el movimiento suave
	currentCameraY = glm::mix(currentCameraY, targetCameraY, 0.1f); // 0.1f controla la suavidad

	// Limitar la c�mara dentro de los bordes del mapa en el eje Y
	float maxCamY = map->getMapSize().y * tileSize - viewHeight;
	currentCameraY = glm::clamp(currentCameraY, 0.0f, maxCamY);

	// Actualizar la proyecci�n con la nueva posici�n de la c�mara y el zoom
	projection = glm::ortho(
		camX, camX + viewWidth, // Izquierda, Derecha
		currentCameraY + viewHeight, currentCameraY // Arriba, Abajo
	);
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



