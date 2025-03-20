#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, JUMP, CROUCH, COVER
};


void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	bJumping = false;
	spritesheet.loadFromFile("images/SoaringEagleSpritesheet.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.125, 0.125), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(7);
	
		sprite->setAnimationSpeed(STAND_LEFT, 8);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.f, 0.125f));
		//sprite->setMirror(true);
		
		sprite->setAnimationSpeed(STAND_RIGHT, 8);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.f, 0.125f));
		//sprite->setMirror(false);
		
		sprite->setAnimationSpeed(MOVE_LEFT, 8);
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.125f, 0.125f));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.250f, 0.125f));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.375f, 0.125f));
		
		sprite->setAnimationSpeed(MOVE_RIGHT, 8);
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.125, 0.125f));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.250, 0.125f));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.375, 0.125f));

		sprite->setAnimationSpeed(JUMP, 8);
		sprite->addKeyframe(JUMP, glm::vec2(0.f, 0.5f));

		sprite->setAnimationSpeed(CROUCH, 8);
		sprite->addKeyframe(CROUCH, glm::vec2(0.f, 0.5f));

		sprite->setAnimationSpeed(COVER, 8);
		sprite->addKeyframe(COVER, glm::vec2(0.f, 0.375f));
		
	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
	sprite->update(deltaTime);
	if(Game::instance().getKey(GLFW_KEY_LEFT))
	{
		if(sprite->animation() != MOVE_LEFT)
		{
			sprite->changeAnimation(MOVE_LEFT);
			sprite->setMirror(true);
		}
		posPlayer.x -= 2;

		if(map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x += 2;
			sprite->changeAnimation(STAND_LEFT);
			sprite->setMirror(true);
		}
	}
	else if(Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		if (sprite->animation() != MOVE_RIGHT)
		{
			sprite->changeAnimation(MOVE_RIGHT);
			sprite->setMirror(false);
		}
		posPlayer.x += 2;

		if(map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x -= 2;
			sprite->changeAnimation(STAND_RIGHT);
			sprite->setMirror(false);
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_UP))
	{
		if (sprite->animation() != COVER)
		{
			sprite->changeAnimation(COVER);
			sprite->setMirror(sprite->isMirrored());
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_DOWN))
	{
		if (sprite->animation() != CROUCH)
		{
			sprite->changeAnimation(CROUCH);
			sprite->setMirror(sprite->isMirrored());
		}
	}
	else
	{
		if (sprite->isMirrored())
		{
			sprite->changeAnimation(STAND_LEFT);
			sprite->setMirror(true);  // Mirror left
		}
		else if (!sprite->isMirrored())
		{
			sprite->changeAnimation(STAND_RIGHT);
			sprite->setMirror(false); // No mirror for right
		}
	}
	
	if(bJumping)
	{
		if (sprite->animation() != JUMP) // Evita cambiar de nuevo si ya está en JUMP
			sprite->changeAnimation(JUMP);

		jumpAngle += JUMP_ANGLE_STEP;
		if(jumpAngle == 180)
		{
			bJumping = false;
			posPlayer.y = startY;
		}
		else
		{
			posPlayer.y = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));
			if(jumpAngle > 90)
				bJumping = !map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y);
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		if(map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
		{
			/*if (sprite->animation() != STAND_LEFT && sprite->animation() != STAND_RIGHT)
			{
				// Verifica la dirección en la que está mirando el jugador y cambia la animación de pie
				if (sprite->isMirrored()) // Si está mirando hacia la izquierda
					sprite->changeAnimation(STAND_LEFT);
				else // Si está mirando hacia la derecha
					sprite->changeAnimation(STAND_RIGHT);
			}*/

			if(Game::instance().getKey(GLFW_KEY_Z))
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
			}
			
		}
	}
	
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::render()
{
	sprite->render();
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}




