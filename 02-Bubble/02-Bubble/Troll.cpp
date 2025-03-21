#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Troll.h"
#include "Game.h"

#define JUMP_ANGLE_STEP 4   // Salto más rápido
#define JUMP_HEIGHT 60      // Salto más bajo
#define FALL_STEP 4         // Caída más rápida
#define MOVE_SPEED 1.0f     // Movimiento lateral más suave
#define DETECTION_RADIUS 1  80  // Radio en el que sigue al jugador
#define SPAWN_RADIUS 145      // Radio en el que el Troll aparece
#define DESPAWN_RADIUS 250    // Radio en el que el Troll desaparece

enum TrollAnims
{
    IDLE, JUMP
};

void Troll::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
    bJumping = false;
    active = false; // El Troll empieza inactivo hasta que el jugador se acerque
    spritesheet.loadFromFile("images/SoaringEagleSpritesheet.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.125, 0.125), &spritesheet, &shaderProgram);
    sprite->setNumberAnimations(2);

    sprite->setAnimationSpeed(IDLE, 8);
    sprite->addKeyframe(IDLE, glm::vec2(0.f, 0.125f));

    sprite->setAnimationSpeed(JUMP, 8);
    sprite->addKeyframe(JUMP, glm::vec2(0.f, 0.5f));

    sprite->changeAnimation(IDLE);
    tileMapDispl = tileMapPos;

    posTroll = tileMapPos;  // Posición actual
    spawnPosition = posTroll;  // Guardamos la posición de spawn
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posTroll.x), float(tileMapDispl.y + posTroll.y)));
}

void Troll::update(int deltaTime, const glm::vec2& playerPos)
{
    float distance = glm::distance(playerPos, glm::vec2(spawnPosition));

    // **Spawn y Despawn del Troll**
    if (!active && distance < SPAWN_RADIUS)
    {
        activate();
    }
    else if (active && distance > DESPAWN_RADIUS)
    {
        deactivate();
        return;
    }

    if (!active) return; // Si el Troll está inactivo, no se actualiza

    sprite->update(deltaTime);

    if (bJumping)
    {
        if (sprite->animation() != JUMP)
            sprite->changeAnimation(JUMP);

        jumpAngle += JUMP_ANGLE_STEP;
        if (jumpAngle >= 180)
        {
            bJumping = false;
            posTroll.y = startY;
            sprite->changeAnimation(IDLE);
        }
        else
        {
            posTroll.y = int(startY - JUMP_HEIGHT * sin(3.14159f * jumpAngle / 180.f));
            if (jumpAngle > 90)
                bJumping = !map->collisionMoveDown(posTroll, glm::ivec2(32, 32), &posTroll.y);
        }
    }
    else
    {
        posTroll.y += FALL_STEP;
        if (map->collisionMoveDown(posTroll, glm::ivec2(32, 32), &posTroll.y))
        {
            if (distance < DETECTION_RADIUS)
            {
                bJumping = true;
                jumpAngle = 0;
                startY = posTroll.y;
            }
        }
    }

    if (jumpAngle < 90)
    {
        if (playerPos.x < posTroll.x)
        {
            if (!map->collisionMoveLeft(posTroll, glm::ivec2(32, 32)))
            {
                posTroll.x -= MOVE_SPEED;
            }
        }
        else if (playerPos.x > posTroll.x)
        {
            if (!map->collisionMoveRight(posTroll, glm::ivec2(32, 32)))
            {
                posTroll.x += MOVE_SPEED;
            }
        }
    }

    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posTroll.x), float(tileMapDispl.y + posTroll.y)));
}

void Troll::render()
{
    if (active) // Solo se renderiza si está activo
        sprite->render();
}

void Troll::setTileMap(TileMap* tileMap)
{
    map = tileMap;
}

void Troll::setPosition(const glm::vec2& pos)
{
    posTroll = pos;
    spawnPosition = pos; // Guardamos la posición inicial para respawnear
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posTroll.x), float(tileMapDispl.y + posTroll.y)));
}

// **Función para activar (spawnear) el Troll**
void Troll::activate()
{
    active = true;
    posTroll = spawnPosition; // Reiniciar en la posición original
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posTroll.x), float(tileMapDispl.y + posTroll.y)));
}

// **Función para desactivar (despawnear) el Troll**
void Troll::deactivate()
{
    active = false;
}
