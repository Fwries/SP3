/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Scene2D.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"


/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CScene2D::CScene2D(void)
	: cMap2D(NULL)
	, cPlayer2D(NULL)
	, cKeyboardController(NULL)
	, cMouseController(NULL)
	, cGUI_Scene2D(NULL)
	, cGameManager(NULL)
	, cSoundController(NULL)
{
}

/**
 @brief Destructor
 */
CScene2D::~CScene2D(void)
{
	if (cSoundController)
	{
		// We won't delete this since it was created elsewhere
		cSoundController = NULL;
	}

	if (cGameManager)
	{
		cGameManager->Destroy();
		cGameManager = NULL;
	}

	if (cGUI_Scene2D)
	{
		cGUI_Scene2D->Destroy();
		cGUI_Scene2D = NULL;
	}

	if (cKeyboardController)
	{
		// We won't delete this since it was created elsewhere
		cKeyboardController = NULL;
	}

	// Destroy the enemies
	for (int i = 0; i < enemyVector.size(); i++)
	{
		delete enemyVector[i];
		enemyVector[i] = NULL;
	}
	enemyVector.clear();

	// Destroy the enemies
	for (int i = 0; i < miscVector.size(); i++)
	{
		delete miscVector[i];
		miscVector[i] = NULL;
	}
	miscVector.clear();

	if (cPlayer2D)
	{
		cPlayer2D->Destroy();
		cPlayer2D = NULL;
	}

	if (cMap2D)
	{
		cMap2D->Destroy();
		cMap2D = NULL;
	}

	// Clear out all the shaders
	//CShaderManager::GetInstance()->Destroy();
}

/**
@brief Init Initialise this instance
*/ 
bool CScene2D::Init(void)
{
	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	// Initialise Inventory
	cInventoryManager = cInventoryManager->GetInstance();

	elapsed = 0;
	timeElapsed = 0.025;
	spawnRate = 8;

	waveLevel = 1;
	spawnBoss = false;

	// Create and initialise the Map 2D
	cMap2D = CMap2D::GetInstance();
	// Set a shader to this class
	cMap2D->SetShader("Shader2D");
	// Initialise the instance
	if (cMap2D->Init(1, 64, 64) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}
	//// Load the map into an array
	//if (cMap2D->LoadMap("Maps/DM2213_Map_Level_01.csv") == false)
	//{
	//	// The loading of a map has failed. Return false
	//	return false;
	//}
	//// Load the map into an array
	//if (cMap2D->LoadMap("Maps/DM2213_Map_Level_02.csv", 1) == false)
	//{
	//	// The loading of a map has failed. Return false
	//	return false;
	//}
	//Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2213_Map_Level_03.csv", 0) == false)
	{
		// The loading of a map has failed. Return false
		return false;
	}

	// Activate diagonal movement
	cMap2D->SetDiagonalMovement(true);
	Closest = NULL;

	// Load Scene2DColour into ShaderManager
	CShaderManager::GetInstance()->Use("Shader2D_Colour");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);
	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();
	// Pass shader to cPlayer2D
	cPlayer2D->SetShader("Shader2D_Colour");
	// Initialise the instance
	if (cPlayer2D->Init() == false)
	{
		cout << "Failed to load CPlayer2D" << endl;
		return false;
	}

	// Create and initialise the CEnemy2D
	enemyVector.clear();

	/*CTeamMate2D* cTeamMate2D = new CTeamMate2D();
	if (cTeamMate2D->Init() == false)
	{
		cout << "Failed to load CTeamMate2D" << endl;
		return false;
	}*/

	miscVector.clear();
	while (true)
	{
		CMisc2D* cMisc2D = new CMisc2D();
		// Pass shader to cEnemy2D
		cMisc2D->SetShader("Shader2D_Colour");
		// Initialise the instance
		if (cMisc2D->Init() == true)
		{
			cMisc2D->SetPlayer2D(cPlayer2D);
			miscVector.push_back(cMisc2D);
		}
		else
		{
			// Break out of this loop if the enemy has all been loaded
			break;
		}
	}

	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();

	// Store the keyboard controller singleton instance here
	cMouseController = CMouseController::GetInstance();

	// Store the cGUI_Scene2D singleton instance here
	cGUI_Scene2D = CGUI_Scene2D::GetInstance();
	cGUI_Scene2D->Init();

	// Game Manager
	cGameManager = CGameManager::GetInstance();
	cGameManager->Init();

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\The Hero.ogg"), 1, true);

	cSoundController->LoadSound(FileSystem::getPath("Sounds\\sfx_coin_double1.wav"), 3, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\sfx_weapon_singleshot2.wav"), 4, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\sfx_wpn_punch4.wav"), 5, true);

	cSoundController->LoadSound(FileSystem::getPath("Sounds\\sfx_deathscream_alien2.wav"), 7, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\sfx_deathscream_robot2.wav"), 8, true);

	//sounds for damage
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\turretHit.ogg"), 9, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\enemyHit.ogg"), 10, true);

	return true;
}

/**
@brief Update Update this instance
*/
bool CScene2D::Update(const double dElapsedTime)
{
	// Call the cPlayer2D's update method before Map2D as we want to capture the inputs before map2D update
	cPlayer2D->Update(dElapsedTime);
	cSoundController->PlaySoundByID(1);

	elapsed += timeElapsed;
	//cout << intElapsed << endl;
	if (remainder(elapsed, spawnRate) >= 0 && remainder(elapsed, spawnRate) <= 0.025)
	{
		CEnemy2D* cEnemy2D = new CEnemy2D();
		// Pass shader to cEnemy2D
		cEnemy2D->SetShader("Shader2D_Colour");
		// Initialise the instance
		if (cEnemy2D->Init() == true)
		{
			cEnemy2D->SetPlayer2D(cPlayer2D);
			enemyVector.push_back(cEnemy2D);
		}
	}
	if (spawnBoss == true)
	{
		CEnemy2D* cEnemy2D = new CEnemy2D();
		// Pass shader to cEnemy2D
		cEnemy2D->SetShader("Shader2D_Colour");
		// Initialise the instance
		if (cEnemy2D->slimeBossInit() == true)
		{
			cEnemy2D->SetPlayer2D(cPlayer2D);
			enemyVector.push_back(cEnemy2D);
		}
		spawnBoss = false;
	}
	//cout << remainder(elapsed, 60) << endl;
	if (remainder(elapsed, 30) >= 0 && remainder(elapsed, 30) <= 0.025 && elapsed >= 6 && waveLevel <= 9)
	{
		if (spawnRate > 4)
		{
			spawnRate = spawnRate - 4;
		}
		else if (spawnRate > 2)
		{
			spawnRate = spawnRate - 2;
		}
		else if (spawnRate > 1)
		{
			spawnRate = spawnRate - 1;
		}
		waveLevel += 1;
		if (waveLevel % 2 != 0 && waveLevel != 1)
		{
			spawnBoss = true;
		}
	}
	else if (remainder(elapsed, 60) >= 0 && remainder(elapsed, 60) <= 0.025 && waveLevel == 10)
	{

	}

	for (int i = 0; i < enemyVector.size(); i++)
	{
		enemyVector[i]->Update(dElapsedTime);
	}
	
	for (int i = 0; i < miscVector.size(); i++)
	{
		miscVector[i]->Update(dElapsedTime);
	}

	for (int i = 0; i < turretVector.size(); i++)
	{
		turretVector[i]->SetEnemyVector(enemyVector);
		turretVector[i]->Update(dElapsedTime);
	}
	
	// Call the Map2D's update method
	cMap2D->Update(dElapsedTime);

	// Get keyboard updates
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_F6))
	{
		// Save the current game to a save file
		// Make sure the file is open
		try {
			if (cMap2D->SaveMap("Maps/DM2213_Map_Level_01_SAVEGAMEtest.csv") == false)
			{
				throw runtime_error("Unable to save the current game to a file");
			}
		}
		catch (runtime_error e)
		{
			cout << "Runtime error: " << e.what();
			return false;
		}
	}

	// Call the cGUI_Scene2D's update method
	cGUI_Scene2D->Update(dElapsedTime);

	// Check if the game should go to the next level
	if (cGameManager->bLevelCompleted == true)
	{
		if (cMap2D->GetCurrentLevel() == 1)
		{
			cGameManager->bPlayerWon = true;
			return false;
		}
		cMap2D->SetCurrentLevel(cMap2D->GetCurrentLevel()+1);
		cPlayer2D->Reset();

		// Create and initialise the CEnemy2D
		enemyVector.clear();
		//while (true)
		//{
		//	CEnemy2D* cEnemy2D = new CEnemy2D();
		//	// Pass shader to cEnemy2D
		//	cEnemy2D->SetShader("Shader2D_Colour");
		//	// Initialise the instance
		//	if (cEnemy2D->Init() == true)
		//	{
		//		cEnemy2D->SetPlayer2D(cPlayer2D);
		//		enemyVector.push_back(cEnemy2D);
		//	}
		//	else
		//	{
		//		// Break out of this loop if the enemy has all been loaded
		//		break;
		//	}
		//}

		miscVector.clear();
		while (true)
		{
			CMisc2D* cMisc2D = new CMisc2D();
			// Pass shader to cEnemy2D
			cMisc2D->SetShader("Shader2D_Colour");
			// Initialise the instance
			if (cMisc2D->Init() == true)
			{
				cMisc2D->SetPlayer2D(cPlayer2D);
				miscVector.push_back(cMisc2D);
			}
			else
			{
				// Break out of this loop if the enemy has all been loaded
				break;
			}
		}

		cGameManager->bLevelCompleted = false;
	}

	// Check if the game has been won by the player
	if (cGameManager->bPlayerWon == true)
	{
		// End the game and switch to Win screen
		PlayerWon = true;
		return false;
	}
	// Check if the game should be ended
	else if (cGameManager->bPlayerLost == true)
	{
		//cSoundController->PlaySoundByID(2);
		PlayerWon = false;
		return false;
	}

	if (cGUI_Scene2D->GetEquipped() != 0)
	{
		if (cMouseController->IsButtonReleased(GLFW_MOUSE_BUTTON_LEFT))
		{
			//cout << cMouseController->GetMousePositionX() << " " << cMouseController->GetMousePositionY() << endl;
		}

		if (cKeyboardController->IsKeyPressed(GLFW_KEY_G))
		{
			//cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, 150);
			int WallType = cGUI_Scene2D->GetEquipped() - 1;
			switch (cGUI_Scene2D->GetEquipped())
			{
			case 1:
				if (cInventoryManager->GetItem("Turret")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1) == 0
					&& cPlayer2D->getPlayerDirection() == 0)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1,WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("Turret")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("Turret")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1) == 0
						&& cPlayer2D->getPlayerDirection() == 1)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1,WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("Turret")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("Turret")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x) == 0
						&& cPlayer2D->getPlayerDirection() == 2)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x,WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("Turret")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("Turret")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x) == 0
						&& cPlayer2D->getPlayerDirection() == 3)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x,WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("Turret")->Remove(1);
					}
					break;
				}
				break;
			case 2:
				if (cInventoryManager->GetItem("WoodWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1) == 0
					&& cPlayer2D->getPlayerDirection() == 0)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("WoodWall")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("WoodWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1) == 0
					&& cPlayer2D->getPlayerDirection() == 1)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("WoodWall")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("WoodWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x) == 0
					&& cPlayer2D->getPlayerDirection() == 2)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("WoodWall")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("WoodWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x) == 0
					&& cPlayer2D->getPlayerDirection() == 3)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("WoodWall")->Remove(1);
					}
					break;
				}
				break;
			case 3:
				if (cInventoryManager->GetItem("StoneWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1) == 0
					&& cPlayer2D->getPlayerDirection() == 0)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("StoneWall")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("StoneWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1) == 0
					&& cPlayer2D->getPlayerDirection() == 1)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("StoneWall")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("StoneWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x) == 0
					&& cPlayer2D->getPlayerDirection() == 2)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("StoneWall")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("StoneWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x) == 0
					&& cPlayer2D->getPlayerDirection() == 3)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("StoneWall")->Remove(1);
					}
					break;
				}
				break;
			case 4:
				if (cInventoryManager->GetItem("IronWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1) == 0
					&& cPlayer2D->getPlayerDirection() == 0)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("IronWall")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("IronWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1) == 0
					&& cPlayer2D->getPlayerDirection() == 1)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("IronWall")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("IronWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x) == 0
					&& cPlayer2D->getPlayerDirection() == 2)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("IronWall")->Remove(1);
					}
					break;
				}
				else if (cInventoryManager->GetItem("IronWall")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x) == 0
					&& cPlayer2D->getPlayerDirection() == 3)
				{
					CTurret* cTurret = new CTurret();
					// Pass shader to cEnemy2D
					cTurret->SetShader("Shader2D_Colour");
					// Initialise the instance
					if (cTurret->Init(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x, WallType) == true)
					{
						cMap2D->SetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x, 150);
						cTurret->SetEnemyVector(enemyVector);
						turretVector.push_back(cTurret);
						cInventoryManager->GetItem("IronWall")->Remove(1);
					}
					break;
				}
				break;
			default:
				break;
			}
		}
	}
	if (cKeyboardController->IsKeyPressed('H'))
	{
		if (cInventoryManager->GetItem("Turret")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x - 1) == 150)
		{
			for (int i = 0; i < turretVector.size(); i++)
			{
				if (turretVector[i]->getTurretPos() == glm::vec2(cPlayer2D->vec2Index.x - 1,cPlayer2D->vec2Index.y))
				{
					cout << "b" << endl;
					cGUI_Scene2D->OpenUpgrade();
					TurretNo = i;
				}
			}
		}
		else if (cInventoryManager->GetItem("Turret")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y, cPlayer2D->vec2Index.x + 1) == 150)
		{
			for (int i = 0; i < turretVector.size(); i++)
			{
				if (turretVector[i]->getTurretPos() == glm::vec2(cPlayer2D->vec2Index.x + 1, cPlayer2D->vec2Index.y))
				{
					cout << "b" << endl;
					cGUI_Scene2D->OpenUpgrade();
					TurretNo = i;
				}
			}
		}
		else if (cInventoryManager->GetItem("Turret")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y - 1, cPlayer2D->vec2Index.x) == 150)
		{
			for (int i = 0; i < turretVector.size(); i++)
			{
				if (turretVector[i]->getTurretPos() == glm::vec2(cPlayer2D->vec2Index.x, cPlayer2D->vec2Index.y - 1))
				{
					cout << "b" << endl;
					cGUI_Scene2D->OpenUpgrade();
					TurretNo = i;
				}
			}
		}
		else if (cInventoryManager->GetItem("Turret")->GetCount() > 0 && cMap2D->GetMapInfo(cPlayer2D->vec2Index.y + 1, cPlayer2D->vec2Index.x) == 150)
		{
			for (int i = 0; i < turretVector.size(); i++)
			{
				if (turretVector[i]->getTurretPos() == glm::vec2(cPlayer2D->vec2Index.x, cPlayer2D->vec2Index.y + 1))
				{
					cout << "b" << endl;
					cGUI_Scene2D->OpenUpgrade();
					TurretNo = i;
				}
			}
		}
	}

	return true;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CScene2D::PreRender(void)
{
	// Reset the OpenGL rendering environment
	glLoadIdentity();

	// Clear the screen and buffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable 2D texture rendering
	glEnable(GL_TEXTURE_2D);
}

/**
 @brief Render Render this instance
 */
void CScene2D::Render(void)
{
	// Call the Map2D's PreRender()
	cMap2D->PreRender();
	// Call the Map2D's Render()
	cMap2D->Render();
	// Call the Map2D's PostRender()
	cMap2D->PostRender();

	for (int i = 0; i < enemyVector.size(); i++)
	{
		// Call the CEnemy2D's PreRender()
		enemyVector[i]->PreRender();
		// Call the CEnemy2D's Render()
		enemyVector[i]->Render();
		// Call the CEnemy2D's PostRender()
		enemyVector[i]->PostRender();
	}

	for (int i = 0; i < miscVector.size(); i++)
	{
		// Call the CMisc2D's PreRender()
		miscVector[i]->PreRender();
		// Call the CMisc2D's Render()
		miscVector[i]->Render();
		// Call the CMisc2D's PostRender()
		miscVector[i]->PostRender();
	}

	for (int i = 0; i < turretVector.size(); i++)
	{
		// Call the CMisc2D's PreRender()
		turretVector[i]->PreRender();
		// Call the CMisc2D's Render()
		turretVector[i]->Render();
		// Call the CMisc2D's PostRender()
		turretVector[i]->PostRender();
	}

	// Call the CPlayer2D's PreRender()
	cPlayer2D->PreRender();
	// Call the CPlayer2D's Render()
	cPlayer2D->Render();
	// Call the CPlayer2D's PostRender()
	cPlayer2D->PostRender();

	// Call the cGUI_Scene2D's PreRender()
	cGUI_Scene2D->PreRender();
	// Call the cGUI_Scene2D's Render()
	cGUI_Scene2D->Render();
	// Call the cGUI_Scene2D's PostRender()
	cGUI_Scene2D->PostRender();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CScene2D::PostRender(void)
{
}

bool CScene2D::GetPlayerWon()
{
	return PlayerWon;
}

vector<CTurret*>& CScene2D::getTurretVec(void)
{
	return turretVector;
}
vector<CEntity2D*>& CScene2D::getEnemyVec(void)
{
	return enemyVector;
}


void CScene2D::spawnExtraEnemy(int i)
{
	for (unsigned j = 0; j < i; ++j)
	{
		CEnemy2D* cEnemy2D = new CEnemy2D();
		// Pass shader to cEnemy2D
		cEnemy2D->SetShader("Shader2D_Colour");
		// Initialise the instance
		if (j == 0)
		{
			if (cEnemy2D->babySlimeInit(glm::vec2(slimeBossPos.x, slimeBossPos.y + 1)) == true)
			{
				cEnemy2D->SetPlayer2D(cPlayer2D);
				enemyVector.push_back(cEnemy2D);
			}
		}
		else if (j == 1)
		{
			if (cEnemy2D->babySlimeInit(glm::vec2(slimeBossPos.x + 1, slimeBossPos.y)) == true)
			{
				cEnemy2D->SetPlayer2D(cPlayer2D);
				enemyVector.push_back(cEnemy2D);
			}
		}
		else if (j == 2)
		{
			if (cEnemy2D->babySlimeInit(glm::vec2(slimeBossPos.x, slimeBossPos.y - 1)) == true)
			{
				cEnemy2D->SetPlayer2D(cPlayer2D);
				enemyVector.push_back(cEnemy2D);
			}
		}
		else
		{
			if (cEnemy2D->babySlimeInit(glm::vec2(slimeBossPos.x - 1, slimeBossPos.y)) == true)
			{
				cEnemy2D->SetPlayer2D(cPlayer2D);
				enemyVector.push_back(cEnemy2D);
			}
		}
	}
}

void CScene2D::setSlimeBPos(glm::vec2 pos)
{
	slimeBossPos = pos;
}
glm::vec2 CScene2D::getSlimePos()
{
	return slimeBossPos;
}

int CScene2D::GetTurretNo(void)
{
	return TurretNo;
}

int CScene2D::getWaveLevel(void)
{
	return waveLevel;
}
