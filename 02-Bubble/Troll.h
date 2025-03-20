#ifndef _TROLL_INCLUDE
#define _TROLL_INCLUDE

#include "Sprite.h"
#include "TileMap.h"

class Troll
{
public:
    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
    void update(int deltaTime, const glm::vec2& playerPos);
    void render();
    void setTileMap(TileMap* tileMap);
    void setPosition(const glm::vec2& pos);
    glm::vec2 getPosition() const { return glm::vec2(posTroll); }
    void activate();   // Activa el Troll (spawnear)
    void deactivate(); // Desactiva el Troll (despawnear)
    bool isActive() const { return active; } // Devuelve si está activo

private:
    bool bJumping;
    bool active;  // Indica si el Troll está spawneado o no
    glm::ivec2 tileMapDispl, posTroll, spawnPosition; // Guarda la posición inicial
    int jumpAngle, startY;
    Texture spritesheet;
    Sprite* sprite;
    TileMap* map;
};

#endif // _TROLL_INCLUDE
