/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
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

class CEnemy2D;
class CTurret;
class CGUI_Scene2D;
class CPlayer2D;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include CPlayer2D
#include "Player2D.h"
// Include CEnemy2D
#include "Enemy2D.h"
// Include CMisc2D
#include "Misc2D.h"
// Include CTurret
#include "Turret.h"

// Include vector
#include <vector>

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Include Mouse controller
#include "Inputs\MouseController.h"

// GUI_Scene2D
#include "GUI_Scene2D.h"

// Game Manager
#include "GameManager.h"

class CScene2D : public CSingletonTemplate<CScene2D>
{
	friend CSingletonTemplate<CScene2D>;
public:
	// Init
	bool Init(void);

	// Update
	bool Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	bool GetPlayerWon();

	vector<CTurret*>& getTurretVec(void);

	vector<CEntity2D*>& getEnemyVec(void);

	void spawnExtraEnemy(int i);
	void setSlimeBPos(glm::vec2 pos);
	glm::vec2 getSlimePos();

	int GetTurretNo(void);
protected:
	// The handler containing the instance of the 2D Map
	CMap2D* cMap2D;
	// The handler containing the instance of CPlayer2Ds
	CPlayer2D* cPlayer2D;
	// The handler containing the instance of CPlayer2Ds
	// The handler containing the instance of CPlayer2Ds
	CTurret* cTurret;
	// A vector containing the instance of CEnemy2Ds
	vector<CEntity2D*> enemyVector;
	// A vector containing the instance of CMisc2Ds
	vector<CEntity2D*> miscVector;
	// A vector containing the instance of CTurret2Ds
	vector<CTurret*> turretVector;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;
	CMouseController* cMouseController;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// GUI_Scene2D
	CGUI_Scene2D* cGUI_Scene2D;

	// Game Manager
	CGameManager* cGameManager;

	// Inventory
	CInventoryItem* cInventoryItem;
	CInventoryManager* cInventoryManager;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	int Closest;
	bool PlayerWon;

	double elapsed;
	double timeElapsed;
	int spawnRate;
	int TurretNo;

	int waveLevel;
	bool spawnBoss;

	bool extraEnemyToSpawn;

	glm::vec2 slimeBossPos;

	// Constructor
	CScene2D(void);
	// Destructor
	virtual ~CScene2D(void);
};

