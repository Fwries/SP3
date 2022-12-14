/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

class CScene2D;
class CTurret;

// Include shader
#include "RenderControl\shader.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Settings
#include "GameControl\Settings.h"

// Include Physics2D
#include "Physics2D.h"

// Include Player2D
#include "Player2D.h"

#include "Turret.h"

#include "Scene2D.h"

#include "InventoryManager.h"

// Include Keyboard controller
#include "Inputs\KeyboardController.h"
#include "Inputs\MouseController.h"

class CEnemy2D : public CEntity2D
{
public:
	// Constructor
	CEnemy2D(void);

	// Destructor
	virtual ~CEnemy2D(void);

	// Init
	bool Init(void);
	bool babySlimeInit(glm::vec2 bossPos);
	bool slimeBossInit(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(const glm::mat4& view, const glm::mat4& projection);

	// PostRender
	void PostRender(void);

	// Set the indices of the enemy2D
	void Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis);

	// Set the number of microsteps of the enemy2D
	void Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis);

	// Set the UV coordinates of the enemy2D
	void Setvec2UVCoordinates(const float fUVCoordinate_XAxis, const float fUVCoordinate_YAxis);

	// Get the indices of the enemy2D
	glm::vec2 Geti32vec2Index(void) const;

	// Get the number of microsteps of the enemy2D
	glm::vec2 Geti32vec2NumMicroSteps(void) const;

	// Set the UV coordinates of the enemy2D
	glm::vec2 Getvec2UVCoordinates(void) const;

	// Set the handle to cPlayer to this class instance
	void SetPlayer2D(CPlayer2D* cPlayer2D);

	//Getting nearest turret
	glm::vec2& findNearestTurret();
	glm::vec2& findNearestBasePart();

	// boolean flag to indicate if this enemy is active
	bool bIsActive;
	bool hitBox;

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	enum FSM
	{
		MOVING = 0,
		BLOCKED = 1,
		ATTACK = 2,
		DEAD = 3,
		FROZEN = 4,
		NUM_FSM
	};

	enum EnemyType
	{
		SKULL = 0,
		SKELE1 = 1,
		VAMPIRE = 2,
		GOBLIN = 3,
		SLIMEBOSS = 4,
		SLIMEBABY = 400,
		NUMENEMYTYPES
	};

	enum Status
	{
		NORMAL = 0,
		BURN = 1,
		FREEZE = 2,
		WIND = 3,
		GOLD = 4,
		FROST = 5,
		NUMOFSTATUSES
	};

	glm::vec2 i32vec2OldIndex;
	glm::vec2 nearestLive;
	CTurret* nearestTurret;
	int nearestTurretInt;

	glm::vec2 nearestBasePart;

	int spawnDeterminer;
	int randType;

	int statMultiplier;


	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// The vec2 which stores the indices of the enemy2D in the Map2D
	glm::vec2 i32vec2Index;
	glm::vec2 Startvec2Index;

	// The vec2 variable which stores The number of microsteps from the tile indices for the enemy2D. 
	// A tile's width or height is in multiples of these microsteps
	glm::vec2 i32vec2NumMicroSteps;

	// The vec2 variable which stores the UV coordinates to render the enemy2D
	glm::vec2 vec2UVCoordinate;

	// The vec2 which stores the indices of the destination for enemy2D in the Map2D
	glm::vec2 i32vec2Destination;
	// The vec2 which stores the direction for enemy2D movement in the Map2D
	glm::vec2 i32vec2Direction;

	// Settings
	CSettings* cSettings;

	// Physics
	CPhysics2D cPhysics2D;

	CInventoryManager* cInventoryManager;
	CInventoryItem* cInventoryItem;

	//Animated Sprite
	CSpriteAnimation* animatedEnemy;

	CMouseController* cMouseController;

	// Current color
	glm::vec4 runtimeColour;

	// Handle to the CPlayer2D
	CPlayer2D* cPlayer2D;

	// Current FSM
	FSM sCurrentFSM;

	Status status;

	EnemyType enemyType;

	// FSM counter - count how many frames it has been in this FSM
	int iFSMCounter;
	int statusCounter;

	float MoveCooldown;
	float MoveTime;

	float AttackCooldown;
	float AttackTime;

	bool faceLeft;

	bool targetableTurret;

	//variables for game
	int HP;
	float MAXHP;
	int ATK;
	float SPE;
	int TRGE;
	int ARGE;

	int X, Y;
	int elapsed;
	int spawnRate;

	// Max count in a state
	const int iMaxFSMCounter = 60;

	int meleeCounter;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	CScene2D* cScene2D;

	// Constraint the enemy2D's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	// Check if a position is possible to move into
	bool CheckPosition(DIRECTION eDirection);
	
	bool AdjustPosition(DIRECTION eDirection);

	// Check if the enemy2D is in mid-air
	bool IsMidAir(void);

	// Let enemy2D interact with the player
	bool InteractWithPlayer(void);

	// Update direction
	void UpdateDirection(void);

	// Flip horizontal direction. For patrol use only
	void FlipHorizontalDirection(void);
	void FlipVerticalDirection(void);

	// Update position
	void UpdatePosition(glm::vec2 destination);

	void updateWindBLow(int direction);
};

