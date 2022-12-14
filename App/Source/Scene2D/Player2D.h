/**
 CPlayer2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include Singleton template
#include "DesignPatterns\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;
class CScene2D;

// Include Keyboard controller
#include "Inputs\KeyboardController.h"
#include "Inputs\MouseController.h"

// Include Physics2D
#include "Physics2D.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

// Include InventoryManager
#include "InventoryManager.h"

#include "Scene2D.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

#include "BulletGenerator.h"

class CPlayer2D : public CSingletonTemplate<CPlayer2D>, public CEntity2D
{
	friend CSingletonTemplate<CPlayer2D>;
public:

	// Init
	bool Init(void);

	// Reset
	bool Reset(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(const glm::mat4& view, const glm::mat4& projection);

	// PostRender
	void PostRender(void);

	// Update the health and lives
	void UpdateHealthLives(void);

	bool GetMaterialRange(void);

	CBulletGenerator* GetBulletGenerator();

	void changeBaseHP(int amount);

	int getPlayerDirection(void);

	float SPE;


	CEntity2D* nearestEnemy;
	glm::vec2 nearestLive;
	int nearestEnemyInt;
	void findNearestEnemy();
	int getNearestEnemy(void);

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT,
		UP,
		DOWN,
		LEFT_UP,
		LEFT_DOWN,
		RIGHT_UP,
		RIGHT_DOWN,
		NUM_DIRECTIONS
	};

	DIRECTION dir;

	glm::vec2 vec2OldIndex;
	glm::vec2 vec2OldMicroSteps;

	CBulletGenerator* cBulletGenerator;

	CScene2D* cScene2D;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;
	
	CMouseController* cMouseController;

	// Physics
	CPhysics2D cPhysics2D;

	//CS: Animated Sprite
	CSpriteAnimation* animatedPlayer;

	// Current color
	glm::vec4 runtimeColour;

	// InventoryManager
	CInventoryManager* cInventoryManager;
	// InventoryItem
	CInventoryItem* cInventoryItem;


	// Count the number of jumps
	int iJumpCount;
	bool FaceDirection;
	bool Hitbox;
	bool MaterialRange;

	// Variables for Materials
	int n_wood, n_stone, n_silver, n_iron, n_gold, n_coal, n_bronze;
	int X, Y;

	// variables for GUI
	bool openCrafting;
	bool openInventory;

	bool WeaponEquiped;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	// Constructor
	CPlayer2D(void);

	// Destructor
	virtual ~CPlayer2D(void);

	// Constraint the player's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	// Check if a position is possible to move into
	bool CheckPosition(DIRECTION eDirection);

	// Check if the player is in mid-air
	bool IsMidAir(void);

	// Update Jump or Fall
	void UpdateJumpFall(const double dElapsedTime = 0.0166666666666667);

	// Let player interact with the map
	void InteractWithMap(void);

	void RandomTileGenerator(void);

	void CheckMaterialAround(void);

	bool GetWeapon();
};

