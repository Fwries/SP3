/**
 CMisc2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

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
class CEnemy2D;
class CScene2D;

// Include Settings
#include "GameControl\Settings.h"

// Include Physics2D
#include "Physics2D.h"

#include "Enemy2D.h"

// Include Player2D
#include "Player2D.h"
#include "Scene2D.h"

// Include InventoryManager
#include "InventoryManager.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

#include "BulletGenerator.h"

class CTurret : public CEntity2D
{
public:
	// Constructor
	CTurret(void);

	// Destructor
	virtual ~CTurret(void);

	// Init
	bool Init(int, int, bool);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

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

	void SetEnemyVector(vector<CEntity2D*>);

	int GetNearestEnemy();

	int GetTurretHP();

	void SetGetTurretHP(int);

	void findNearestEnemy();

	void UpgradeTurret(bool);

	int GetCurrTurret();

	int GetNextTurret(bool);

	CBulletGenerator* GetBulletGenerator();

	glm::vec2 getTurretPos();

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
		IDLE = 0,
		PATROL = 1,
		ATTACK = 2,
		NUM_FSM
	};

	enum TurretType
	{
		NONE = -1,

		WOOD_WALL = 0,
		STONE_WALL = 1000,
		IRON_WALL = 2000,
		
		TURRET = 0001,
		STONE_TURRET = 1011,
		ELEMENTAL_TURRET = 1012,

		REINFORCED_STONE_TURRET = 2011,
		MULTI_PEBBLE_TURRET = 2012,
		RANDOM_DMG_TURRET = 2013,

		FLAME_TURRET = 2021,
		FROST_TURRET = 2022,
		MYSTERIOUS_TURRET = 2023,

		SHARP_STONE_TURRET = 3011,
		IRON_TURRET = 3012,
		ORE_GENERATOR = 3013,

		STONE_BURST_TOWER = 3021,
		MULTISHOT_TURRET = 3022,
		TURRET2 = 3023,

		RANDOM_DMG_TURRETV2 = 3031,
		GLITCHED_TURRET = 3032,
		GETRANDOMTURRET = 3033,

		FLAME_SPEAR_TURRET = 3041,
		FLAMETHROWER_TURRET = 3042,
		FIREWALL_TURRET = 3043,

		ICE_SPEAR_TURRET = 3051,
		SNOWBALL_TURRET = 3052,
		ICE_FLOOR_TURRET = 3053,

		WIND_TURRET = 3061,
		THUNDER_TURRET = 3062,
		ISTERIOUS_TURRET = 3063,

		SHARPER_STONE_TURRET = 4011,
		SNIPER_TURRET = 4012,
		BLUNT_METAL_TURRET = 4013,

		REINFORCED_IRON_TURRET = 4021,
		SHINY_IRON_TURRET = 4022,
		TANK = 4023,

		GOLDEN_TURRET = 4031,
		ELEMENTAL_TURRET2 = 4032,
		MIDAS_TOUCH = 4033,

		IRON_BURST_TURRET = 4041,
		HOT_IRON_TURRET = 4042,
		REINFORCED_IRON_TURRET2 = 4043,

		MULTIMULTISHOT_TURRET = 4051,
		STARSHOT_TURRET = 4052,
		WRONGDIRECTION_TURRET = 4053,

		TURRET3 = 4061,
		SHOTGUN_TURRET = 4062,
		TURRETINFINITY = 4063,

		RANDOM_DMG_TURRETV3 = 4071,
		FLIP_A_COIN_TURRET = 4072,
		RANDOM_DIRECTION_TURRET = 4073,

		UPGRADED_GLITCHED_TURRET = 4081,
		GETRANDOMTURRET2 = 4082,
		ROBOT_PLAYER = 4083,

		ETERNAL_FLAME_SPEAR_TURRET = 4091,
		BLUE_FLAME_TURRET = 4092,
		DUO_FLAME_SPEAR_TURRET = 4093,


	};

	enum ELEMENT
	{
		NORMAL = 0,
		BURN = 1,
		FROZEN = 2,
	};

	glm::vec2 i32vec2OldIndex;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	CScene2D* cScene2D;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// A vector containing the instance of CEnemy2Ds
	vector<CEntity2D*> enemyVector;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// The vec2 which stores the indices of the enemy2D in the Map2D
	glm::vec2 i32vec2Index;

	// The vec2 variable which stores The number of microsteps from the tile indices for the enemy2D. 
	// A tile's width or height is in multiples of these microsteps
	glm::vec2 i32vec2NumMicroSteps;

	// The vec2 variable which stores the UV coordinates to render the enemy2D
	glm::vec2 vec2UVCoordinate;

	// The vec2 which stores the indices of the destination for enemy2D in the Map2D
	glm::vec2 i32vec2Destination;
	// The vec2 which stores the direction for enemy2D movement in the Map2D
	glm::vec2 i32vec2Direction;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	// InventoryManager
	CInventoryManager* cInventoryManager;
	// InventoryItem
	CInventoryItem* cInventoryItem;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	// Settings
	CSettings* cSettings;

	// Physics
	CPhysics2D cPhysics2D;

	// Current color
	glm::vec4 runtimeColour;

	CBulletGenerator* cBulletGenerator;

	// Current FSM
	FSM sCurrentFSM;

	TurretType turretType;
	TurretType upgradeLeft;
	TurretType upgradeRight;
	TurretType upgradeRare;

	// FSM counter - count how many frames it has been in this FSM
	int iFSMCounter;

	// Max count in a state
	const int iMaxFSMCounter = 60;

	glm::vec2 nearestLive;
	CEntity2D* nearestEnemy;
	int nearestEnemyInt;

	int TurretHP;
	int TurretDamage;
	int TurretElement;
	double TurretCooldown;
	double range;
	glm::vec4 Colour;

	double Time;
	double CurrTime;

	// Constraint the enemy2D's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	// Check if a position is possible to move into
	bool CheckPosition(DIRECTION eDirection);
	
	bool AdjustPosition(DIRECTION eDirection);

	// Let enemy2D interact with the player
	bool InteractWithPlayer(void);

	// Flip horizontal direction. For patrol use only
	void FlipHorizontalDirection(void);

	// Update position
	void UpdatePosition(void);

};

