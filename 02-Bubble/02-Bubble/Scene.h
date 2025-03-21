#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Troll.h"

// Scene contains all the entities of our game.
// It is responsible for updating and render them.

class Scene
{
public:
    Scene();
    ~Scene();

    void init();
    void cameraUpdate();
    void update(int deltaTime);
    void render();

private:
    void initShaders();

private:
    TileMap* map;
    TileMap* back;
    Player* player;
    ShaderProgram texProgram;
    float currentTime;
    glm::mat4 projection;

    Troll* troll1;
    Troll* troll2;
    Troll* troll3;  
    Troll* troll4;  
};

#endif // _SCENE_INCLUDE
