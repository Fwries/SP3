/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#pragma once

class CScene2D;

// Include SingletonTemplate
#include "DesignPatterns/SingletonTemplate.h"

// Include CEntity2D
#include "Primitives/Entity2D.h"

// FPS Counter
#include "TimeControl\FPSCounter.h"

// Include CInventoryManager
#include "InventoryManager.h"

#include "Inputs\KeyboardController.h"

#include "Player2D.h"

#include "Scene2D.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include IMGUI
// Important: GLEW and GLFW must be included before IMGUI
#ifndef IMGUI_ACTIVE
#include "GUI\imgui.h"
#include "GUI\backends\imgui_impl_glfw.h"
#include "GUI\backends\imgui_impl_opengl3.h"
#define IMGUI_ACTIVE
#endif

#include "GameControl/Settings.h"

#include <string>
using namespace std;

struct ImageData
{
	std::string fileName;
	unsigned textureID;
};

class CGUI_Scene2D : public CSingletonTemplate<CGUI_Scene2D>, public CEntity2D
{
	friend CSingletonTemplate<CGUI_Scene2D>;
public:
	// Init
	bool Init(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	// Return itemEquipped
	int GetEquipped();
	
	void OpenUpgrade();

	bool UpgradeState();

	int Checkupgrade();

protected:
	// Constructor
	CGUI_Scene2D(void);

	// Destructor
	virtual ~CGUI_Scene2D(void);

	// FPS Control
	CFPSCounter* cFPSCounter;

	// Flags for IMGUI
	ImGuiWindowFlags window_flags;
	float m_fProgressBar;
	bool openInventory;
	bool openCrafting;
	bool openUpgrade;
	int Upgrade;
	int recipeNo;
	ImageData InventoryBG;
	ImageData EquipButton;
	ImageData EquippedButton;
	ImageData CraftingBG;
	ImageData LeftButton;
	ImageData RightButton;
	ImageData CraftButton;
	ImageData UpgradeBG;
	ImageData UpgradeButton;
	ImageData DestroyButton;
	vector<ImageData> TurretImg;
	unsigned iTextureID;
	unsigned LeftUpgrade;
	unsigned RightUpgrade;
	unsigned Material1;
	unsigned Material2;
	unsigned Material3;
	unsigned Material4;
	unsigned Material5;
	unsigned Material6;
	std::string LeftDesc;
	std::string RightDesc;
	std::string Cost1;
	std::string Cost2;
	std::string Cost3;
	std::string Cost4;
	std::string Cost5;
	std::string Cost6;

	// Left Tower Costs (1 = Tier 1 Physical | 2 = Tier 2 Physical | 3 = Tier 3 Physical)
	int LeftTowerCosts;
	// Right Tower Costs(1 = Tier 1 Elemental | 2 = Tier 2 Elemental| 3 = Tier 3 Elemental)
	int RightTowerCosts;
	void ChangeTurretCosts();
	void LeftCostRemove();
	void RightCostRemove();

	int prevWave;
	float announcementTimer;

	// Variables for Buildings
	int itemEquipped;

	vector<CTurret*> turretVector;

	CSettings* cSettings;

	CPlayer2D* cPlayer2D;

	CScene2D* cScene2D;

	// The handler containing the instance of CInventoryManager
	CInventoryManager* cInventoryManager;
	// The handler containing the instance of CInventoryItem
	CInventoryItem* cInventoryItem;

	CKeyboardController* cKeyboardController;
};
