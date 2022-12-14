/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Player2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
#include "Primitives/MeshBuilder.h"

// Include Game Manager
#include "GameManager.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::CPlayer2D(void)
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
	, cSoundController(NULL)
{
	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::~CPlayer2D(void)
{
	// We won't delete this since it was created elsewhere
	cSoundController = NULL;

	// We won't delete this since it was created elsewhere
	cInventoryManager = NULL;

	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	if (cBulletGenerator)
	{
		delete cBulletGenerator;
		cBulletGenerator = nullptr;
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CPlayer2D::Init(void)
{
	dir = DIRECTION::RIGHT;

	cBulletGenerator = new CBulletGenerator();

	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	cMouseController = CMouseController::GetInstance();

	cScene2D = CScene2D::GetInstance();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();

	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	// Load the player texture 
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Sprites/Player.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Sprites/Player.png" << endl;
		return false;
	}
	
	//CS: Create the animated sprite and setup the animation 
	animatedPlayer = CMeshBuilder::GenerateSpriteAnimation(8, 4, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedPlayer->AddAnimation("rightS", 0, 3);
	animatedPlayer->AddAnimation("leftS", 4, 7);
	animatedPlayer->AddAnimation("HrightS", 8, 11);
	animatedPlayer->AddAnimation("HleftS", 12, 15);
	
	animatedPlayer->AddAnimation("rightW", 16, 19);
	animatedPlayer->AddAnimation("leftW", 20, 23);
	animatedPlayer->AddAnimation("HrightW", 24, 27);
	animatedPlayer->AddAnimation("HleftW", 28, 31);
	//CS: Play the "idle" animation as default
	animatedPlayer->PlayAnimation("rightW", -1, 1.0f);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);

	// Get the handler to the CInventoryManager instance
	cInventoryManager = CInventoryManager::GetInstance();
	// Add a Lives icon as one of the inventory items
	cInventoryItem = cInventoryManager->Add("Lives", "Image/Scene2D_Lives.tga", 20, 20);
	cInventoryItem->vec2Size = glm::vec2(25, 25);
	cInventoryItem = cInventoryManager->Add("Bullets", "Image/Scene2D/Bullet.png", 100, 100);
	cInventoryItem->vec2Size = glm::vec2(25, 25);
	cInventoryItem = cInventoryManager->Add("Base HP", "Image/Scene2D/Base.tga", 100, 100);
	cInventoryItem->vec2Size = glm::vec2(25, 25);


	// Get the handler to the CSoundController
	cSoundController = CSoundController::GetInstance();
	FaceDirection = RIGHT;
	SetHitBox(false);

	// Materials
	n_wood = 25;
	n_stone = 30;
	n_iron = 15;
	n_bronze = 10;
	n_silver = 10;
	n_gold = 5;
	X = Y = 0;
	MaterialRange = false;

	SPE = 3;

	openCrafting = false;
	openInventory = false;
	WeaponEquiped = true;

	return true;
}

/**
 @brief Reset this instance
 */
bool CPlayer2D::Reset()
{
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	//Set it to fall upon entering new level
	//cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	//CS: Reset double jump
	iJumpCount = 0;

	//CS: Play the "idle" animation as default
	animatedPlayer->PlayAnimation("right", -1, 1.0f);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	return true;
}

/**
 @brief Update this instance
 */
void CPlayer2D::Update(const double dElapsedTime)
{
	static double time = 0.0;
	time += dElapsedTime;

	if (cInventoryManager->GetItem("Lives")->GetCount() <= 0)
	{
		vec2Index = cMap2D->GetPlayerSpawnIndex();
		vec2NumMicroSteps.x = vec2NumMicroSteps.y = 0.f;
		cInventoryManager->GetItem("Lives")->Add(100);
		cInventoryManager->GetItem("Plank")->Remove(cInventoryManager->GetItem("Plank")->GetCount());
		cInventoryManager->GetItem("Stone")->Remove(cInventoryManager->GetItem("Stone")->GetCount());
		cInventoryManager->GetItem("Iron")->Remove(cInventoryManager->GetItem("Iron")->GetCount());
		cInventoryManager->GetItem("Silver")->Remove(cInventoryManager->GetItem("Silver")->GetCount());
		cInventoryManager->GetItem("Bronze")->Remove(cInventoryManager->GetItem("Bronze")->GetCount());
		cInventoryManager->GetItem("Gold")->Remove(cInventoryManager->GetItem("Gold")->GetCount());
		cInventoryManager->GetItem("Turret")->Remove(cInventoryManager->GetItem("Turret")->GetCount());
		cInventoryManager->GetItem("WoodWall")->Remove(cInventoryManager->GetItem("WoodWall")->GetCount());
		cInventoryManager->GetItem("StoneWall")->Remove(cInventoryManager->GetItem("StoneWall")->GetCount());
		cInventoryManager->GetItem("IronWall")->Remove(cInventoryManager->GetItem("IronWall")->GetCount());
	}

	if (cInventoryManager->GetItem("Base HP")->GetCount() <= 0)
		CGameManager::GetInstance()->bPlayerLost = true;

	if (cScene2D->getWaveLevel() >= 11)
		CGameManager::GetInstance()->bPlayerWon = true;

	// Store the old position
	vec2OldIndex = vec2Index;
	vec2OldMicroSteps = vec2NumMicroSteps;

	// To determine the player's direction
	char* keydownAD = "";
	char* keydownWS = "";

	// Get keyboard updates
	if (cKeyboardController->IsKeyDown(GLFW_KEY_A))
	{
		keydownAD = "A";
		// Calculate the new position to the left
		if (vec2Index.x >= 0)
		{
			vec2NumMicroSteps.x -= SPE;
			if (vec2NumMicroSteps.x < 0)
			{
				vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(LEFT);

		// If the new position is not feasible, then revert to old position
		if (CheckPosition(LEFT) == false)
		{
			vec2Index = vec2OldIndex;
			vec2NumMicroSteps.x = 0;
		}

		FaceDirection = LEFT;

		//CS: Change Color
		//runtimeColour = glm::vec4(1.0, 0.0, 0.0, 1.0);
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_D))
	{
		keydownAD = "D";
		// Calculate the new position to the right
		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		{
			vec2NumMicroSteps.x += SPE;

			if (vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
			{
				vec2NumMicroSteps.x = 0;
				vec2Index.x++;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(RIGHT);

		// If the new position is not feasible, then revert to old position
		if (CheckPosition(RIGHT) == false)
		{
			vec2NumMicroSteps.x = 0;
		}

		FaceDirection = RIGHT;

		//CS: Change Color
		//runtimeColour = glm::vec4(1.0, 1.0, 0.0, 1.0);
	}
	if (cKeyboardController->IsKeyDown(GLFW_KEY_W))
	{
		keydownWS = "W";
		// Calculate the new position up
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			vec2NumMicroSteps.y += SPE;
			if (vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				vec2NumMicroSteps.y = 0;
				vec2Index.y++;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(UP);

		// If the new position is not feasible, then revert to old position
		if (CheckPosition(UP) == false)
		{
			vec2NumMicroSteps.y = 0;
		}

		//FaceDirection = UP;

		//CS: Play the "idle" animation
		//animatedPlayer->PlayAnimation("idle", -1, 1.0f);

		//CS: Change Color
		//runtimeColour = glm::vec4(0.0, 1.0, 1.0, 0.5);
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_S))
	{
		keydownWS = "S";
		vec2OldIndex = vec2Index;

		// Calculate the new position down
		if (vec2Index.y >= 0)
		{
			vec2NumMicroSteps.y -= SPE;
			if (vec2NumMicroSteps.y < 0)
			{
				vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(DOWN);

		// If the new position is not feasible, then revert to old position
		if (CheckPosition(DOWN) == false)
		{
			vec2Index = vec2OldIndex;
			vec2NumMicroSteps.y = 0;
		}

		//FaceDirection = DOWN;

		//CS: Play the "idle" animation
		//animatedPlayer->PlayAnimation("idle", -1, 1.0f);

		//CS: Change Color
		//runtimeColour = glm::vec4(1.0, 0.0, 1.0, 0.5);
	}
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_SPACE))
	{
		cSoundController->PlaySoundByID(4);
	}
	if (cKeyboardController->IsKeyDown(GLFW_KEY_1))
	{
		WeaponEquiped = true;
	}
	if (cKeyboardController->IsKeyDown(GLFW_KEY_2))
	{
		WeaponEquiped = false;
	}

	if ((keydownAD == "A") && (keydownWS == ""))
	{
		dir = DIRECTION::LEFT;
	}
	else if ((keydownAD == "D") && (keydownWS == ""))
	{
		dir = DIRECTION::RIGHT;
	}
	else if ((keydownAD == "") && (keydownWS == "W"))
	{
		dir = DIRECTION::UP;
	}
	else if ((keydownAD == "") && (keydownWS == "S"))
	{
		dir = DIRECTION::DOWN;
	}
	else if ((keydownAD == "A") && (keydownWS == "W"))
	{
		dir = DIRECTION::LEFT_UP;
		// Player is trying to move left_up but collided with walls on top
		if ((vec2OldMicroSteps.y == vec2NumMicroSteps.y) && (vec2OldMicroSteps.x != vec2NumMicroSteps.x))
			dir = DIRECTION::LEFT;
		// Player is trying to move left_up but collided with walls at the left
		else if ((vec2OldMicroSteps.x == vec2NumMicroSteps.x) && (vec2OldMicroSteps.y != vec2NumMicroSteps.y))
			dir = DIRECTION::UP;
	}
	else if ((keydownAD == "A") && (keydownWS == "S"))
	{
		dir = DIRECTION::LEFT_DOWN;
		// Player is trying to move left_down but collided with walls below
		if ((vec2OldMicroSteps.y == vec2NumMicroSteps.y) && (vec2OldMicroSteps.x != vec2NumMicroSteps.x))
			dir = DIRECTION::LEFT;
		// Player is trying to move left_down but collided with walls at the left
		else if ((vec2OldMicroSteps.x == vec2NumMicroSteps.x) && (vec2OldMicroSteps.y != vec2NumMicroSteps.y))
			dir = DIRECTION::DOWN;
	}
	else if ((keydownAD == "D") && (keydownWS == "W"))
	{
		dir = DIRECTION::RIGHT_UP;
		// Player is trying to move right_up but collided with walls on top
		if ((vec2OldMicroSteps.y == vec2NumMicroSteps.y) && (vec2OldMicroSteps.x != vec2NumMicroSteps.x))
			dir = DIRECTION::RIGHT;
		// Player is trying to move right_up but collided with walls on the right
		else if ((vec2OldMicroSteps.x == vec2NumMicroSteps.x) && (vec2OldMicroSteps.y != vec2NumMicroSteps.y))
			dir = DIRECTION::UP;
	}
	else if ((keydownAD == "D") && (keydownWS == "S"))
	{
		dir = DIRECTION::RIGHT_DOWN;
		// Player is trying to move right_down but collided with walls below
		if ((vec2OldMicroSteps.y == vec2NumMicroSteps.y) && (vec2OldMicroSteps.x != vec2NumMicroSteps.x))
			dir = DIRECTION::RIGHT;
		// Player is trying to move right_down but collided with walls on the right
		else if ((vec2OldMicroSteps.x == vec2NumMicroSteps.x) && (vec2OldMicroSteps.y != vec2NumMicroSteps.y))
			dir = DIRECTION::DOWN;
	}

	keydownAD = "";
	keydownWS = "";

	// Check GUI state
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_E))
	{
		openInventory = !openInventory;
		openCrafting = false;
	}

	if (cKeyboardController->IsKeyPressed(GLFW_KEY_F))
	{
		openCrafting = !openCrafting;
		openInventory = false;
	}

	// Generate bullet & limit its firing rate to 1 bullet every 0.2s
	static double currTime = 0.0;
	static const double LCLICK_WAIT_TIME = 0.2;
	if (!openCrafting && !openInventory)
	{
		if (time > (currTime + LCLICK_WAIT_TIME))
		{
			if (cMouseController->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT) && WeaponEquiped == false)
			{
				currTime = time;
				cBulletGenerator->GenerateBullet(this->vec2Index, (int)dir);
				cInventoryManager->GetItem("Bullets")->Remove(1);
			}
		}
	}

	for (unsigned i = 0; i < cBulletGenerator->GetBulletsVector().size(); ++i)
	{
		cBulletGenerator->GetBulletsVector()[i]->Update();
		cBulletGenerator->GetBulletsVector()[i]->SetEnemyVector(cScene2D->getEnemyVec());
	}

	if (FaceDirection == LEFT)
	{
		if (WeaponEquiped)
		{
			animatedPlayer->PlayAnimation("leftS", -1, 1.0f);
		}
		else
		{
			animatedPlayer->PlayAnimation("leftW", -1, 1.0f);
		}
	}
	if (FaceDirection == RIGHT)
	{
		if (WeaponEquiped)
		{
			animatedPlayer->PlayAnimation("rightS", -1, 1.0f);
		}
		else
		{
			animatedPlayer->PlayAnimation("rightW", -1, 1.0f);
		}
	}

	// Interact with the Map
	InteractWithMap();

	if (cKeyboardController->IsKeyPressed(GLFW_KEY_F8))
	{
		cInventoryManager->GetItem("Plank")->Add(cInventoryItem->GetMaxCount());
		cInventoryManager->GetItem("Stone")->Add(cInventoryItem->GetMaxCount());
		cInventoryManager->GetItem("Iron")->Add(cInventoryItem->GetMaxCount());
		cInventoryManager->GetItem("Silver")->Add(cInventoryItem->GetMaxCount());
		cInventoryManager->GetItem("Bronze")->Add(cInventoryItem->GetMaxCount());
		cInventoryManager->GetItem("Gold")->Add(cInventoryItem->GetMaxCount());
		cInventoryManager->GetItem("Turret")->Add(cInventoryItem->GetMaxCount());
	}

	// Randomly generates tiles
	RandomTileGenerator();

	// Check for Materials around the player
	CheckMaterialAround();

	//CS: Update the animated sprite
	animatedPlayer->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CPlayer2D::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);

	for (unsigned i = 0; i < cBulletGenerator->GetBulletsVector().size(); ++i)
		cBulletGenerator->GetBulletsVector()[i]->PreRender();
}

/**
 @brief Render this instance
 */
void CPlayer2D::Render(const glm::mat4& view, const glm::mat4& projection)
{
	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
													vec2UVCoordinate.y,
													0.0f));
	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection * view * transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

		//CS: Render the animated sprite
		glBindVertexArray(VAO);
		animatedPlayer->Render();
		for (unsigned i = 0; i < cBulletGenerator->GetBulletsVector().size(); ++i)
			cBulletGenerator->GetBulletsVector()[i]->Render(view, projection);
		glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CPlayer2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
	for (unsigned i = 0; i < cBulletGenerator->GetBulletsVector().size(); ++i)
		cBulletGenerator->GetBulletsVector()[i]->PostRender();
}

/**
 @brief Constraint the player's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CPlayer2D::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (vec2Index.x < 0)
		{
			vec2Index.x = 0;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y < 0)
		{
			vec2Index.y = 0;
			vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CPlayer2D::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CPlayer2D::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.x >= cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2NumMicroSteps.x = 0;
			return true;
		}

		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}

	}
	else if (eDirection == UP)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.y >= cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2NumMicroSteps.y = 0;
			return true;
		}

		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

// Check if the player is in mid-air
bool CPlayer2D::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
		return false;

	// Check if the tile below the player's current position is empty
	if ((vec2NumMicroSteps.x == 0) && 
		(cMap2D->GetMapInfo(vec2Index.y-1, vec2Index.x) == 0))
	{
		return true;
	}

	return false;
}

// Update Jump or Fall
void CPlayer2D::UpdateJumpFall(const double dElapsedTime)
{
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.SetTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current vec2Index.y
		int iIndex_YAxis_OLD = vec2Index.y;

		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->MICRO_STEP_YAXIS); //Displacement divide by distance for 1 microstep
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			vec2NumMicroSteps.y += iDisplacement_MicroSteps;
			if (vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				vec2NumMicroSteps.y -= cSettings->NUM_STEPS_PER_TILE_YAXIS;
				if (vec2NumMicroSteps.y < 0)
					vec2NumMicroSteps.y = 0;
				vec2Index.y++;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(UP);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop jump if so.
		int iIndex_YAxis_Proposed = vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i <= iIndex_YAxis_Proposed; i++)
		{
			// Change the player's index to the current i value
			vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(UP) == false)
			{
				// Align with the row
				vec2NumMicroSteps.y = 0;
				// Set the Physics to fall status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
				break;
			}
		}

		// If the player is still jumping and the initial velocity has reached zero or below zero, 
		// then it has reach the peak of its jump
		if ((cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP) && (cPhysics2D.GetDisplacement().y <= 0.0f))
		{
			// Set status to fall
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FALL)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.SetTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current vec2Index.y
		int iIndex_YAxis_OLD = vec2Index.y;

		// Translate the displacement from pixels to indices
		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->MICRO_STEP_YAXIS);

		if (vec2Index.y >= 0)
		{
			vec2NumMicroSteps.y -= fabs(iDisplacement_MicroSteps);
			if (vec2NumMicroSteps.y < 0)
			{
				vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(DOWN);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop fall if so.
		int iIndex_YAxis_Proposed = vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i >= iIndex_YAxis_Proposed; i--)
		{
			// Change the player's index to the current i value
			vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(DOWN) == false)
			{
				// Revert to the previous position
				if (i != iIndex_YAxis_OLD)
					vec2Index.y = i + 1;
				// Set the Physics to idle status
				/*cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);*/
				iJumpCount = 0;
				vec2NumMicroSteps.y = 0;
				break;
			}
		}
	}
}

/**
 @brief Let player interact with the map. You can add collectibles such as powerups and health here.
 */
void CPlayer2D::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 2:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
		cInventoryItem = cInventoryManager->GetItem("Coin");
		cInventoryItem->Add(1);
		break;
	case 10:
		// Increase the lives by 1
		cInventoryItem = cInventoryManager->GetItem("Lives");
		cInventoryItem->Add(1);
		// Erase the life from this position
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
		break;
	case 99:
		// Level has been completed
		CGameManager::GetInstance()->bLevelCompleted = true;
		break;
	default:
		break;
	}
}

void CPlayer2D::RandomTileGenerator(void)
{
	while (n_wood > 0)
	{
		X = 15 + (rand() % 35);
		Y = 15 + (rand() % 35);
		if (cMap2D->GetMapInfo(X, Y) == 0)
		{
			cMap2D->SetMapInfo(X, Y, 135);
			n_wood--;
		}
	}
	while (n_stone > 0)
	{
		X = 15 + (rand() % 35);
		Y = 15 + (rand() % 35);
		if (cMap2D->GetMapInfo(X, Y) == 0)
		{
			cMap2D->SetMapInfo(X, Y, 140);
			n_stone--;
		}
	}
	while (n_iron > 0)
	{
		X = 5 + (rand() % 55);
		Y = 5 + (rand() % 55);
		if (cMap2D->GetMapInfo(X, Y) == 0)
		{
			cMap2D->SetMapInfo(X, Y, 141);
			n_iron--;
		}
	}
	while (n_silver > 0)
	{
		X = 5 + (rand() % 55);
		Y = 5 + (rand() % 55);
		if (cMap2D->GetMapInfo(X, Y) == 0)
		{
			cMap2D->SetMapInfo(X, Y, 142);
			n_silver--;
		}
	}
	while (n_bronze > 0)
	{
		X = 5 + (rand() % 55);
		Y = 5 + (rand() % 55);
		if (cMap2D->GetMapInfo(X, Y) == 0)
		{
			cMap2D->SetMapInfo(X, Y, 143);
			n_bronze--;
		}
	}
	while (n_gold > 0)
	{
		X = 5 + (rand() % 55);
		Y = 5+ (rand() % 55);
		if (cMap2D->GetMapInfo(X, Y) == 0)
		{
			cMap2D->SetMapInfo(X, Y, 144);
			n_gold--;
		}
	}
}

void CPlayer2D::CheckMaterialAround(void)
{
	// Check Right of Player
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1))
	{
	case 135:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Plank");
			cInventoryItem->Add(1);
			n_wood++;
		}
		break; 
	case 140:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Stone");
			cInventoryItem->Add(1);
			n_stone++;
		}
		break;
	case 141:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Iron");
			cInventoryItem->Add(1);
			n_iron++;
		}
		break;
	case 142:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Silver");
			cInventoryItem->Add(1);
			n_silver++;
		}
		break;
	case 143:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Bronze");
			cInventoryItem->Add(1);
			n_bronze++;
		}
		break;
	case 144:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Gold");
			cInventoryItem->Add(1);
			n_gold++;
		}
		break;
	default:
		MaterialRange = false;
		break;
	}
	// Check Left of Player
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 1))
	{
	case 135:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Plank");
			cInventoryItem->Add(1);
			n_wood++;
		}
		break;
	case 140:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Stone");
			cInventoryItem->Add(1);
			n_stone++;
		}
		break;
	case 141:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Iron");
			cInventoryItem->Add(1);
			n_iron++;
		}
		break;
	case 142:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Silver");
			cInventoryItem->Add(1);
			n_silver++;
		}
		break;
	case 143:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Bronze");
			cInventoryItem->Add(1);
			n_bronze++;
		}
		break;
	case 144:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 1, 0);
			cInventoryItem = cInventoryManager->GetItem("Gold");
			cInventoryItem->Add(1);
			n_gold++;
		}
		break;
	default:
		break;
	}
	// Check Top of Player
	switch (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x))
	{
	case 135:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Plank");
			cInventoryItem->Add(1);
			n_wood++;
		}
		break;
	case 140:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Stone");
			cInventoryItem->Add(1);
			n_stone++;
		}
		break;
	case 141:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Iron");
			cInventoryItem->Add(1);
			n_iron++;
		}
		break;
	case 142:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Silver");
			cInventoryItem->Add(1);
			n_silver++;
		}
		break;
	case 143:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Bronze");
			cInventoryItem->Add(1);
			n_bronze++;
		}
		break;
	case 144:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Gold");
			cInventoryItem->Add(1);
			n_gold++;
		}
		break;
	default:
		break;
	}
	// Check Bottom of Player
	switch (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x))
	{
	case 135:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y - 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Plank");
			cInventoryItem->Add(1);
			n_wood++;
		}
		break;
	case 140:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y - 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Stone");
			cInventoryItem->Add(1);
			n_stone++;
		}
		break;
	case 141:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y - 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Iron");
			cInventoryItem->Add(1);
			n_iron++;
		}
		break;
	case 142:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y - 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Silver");
			cInventoryItem->Add(1);
			n_silver++;
		}
		break;
	case 143:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y - 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Bronze");
			cInventoryItem->Add(1);
			n_bronze++;
		}
		break;
	case 144:
		MaterialRange = true;
		if (cKeyboardController->IsKeyPressed('X'))
		{
			cMap2D->SetMapInfo(vec2Index.y - 1, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Gold");
			cInventoryItem->Add(1);
			n_gold++;
		}
		break;
	default:
		break;
	}
}

bool CPlayer2D::GetWeapon()
{
	return WeaponEquiped;
}

bool CPlayer2D::GetMaterialRange(void)
{
	return MaterialRange;
}

/**
 @brief Update the health and lives.
 */
void CPlayer2D::UpdateHealthLives(void)
{
	// But we reduce the lives by 1.
	cInventoryItem = cInventoryManager->GetItem("Lives");
	cInventoryItem->Remove(1);
	// Check if there is no lives left...
}

CBulletGenerator* CPlayer2D::GetBulletGenerator()
{
	return cBulletGenerator;
}

void CPlayer2D::changeBaseHP(int amount)
{
	cInventoryItem = cInventoryManager->GetItem("Base HP");
	cInventoryItem->Remove(amount);
}

int CPlayer2D::getPlayerDirection(void)
{
	return dir;
}

void CPlayer2D::findNearestEnemy()
{
	glm::vec2 NEWLive;
	nearestEnemy = nullptr;
	for (int i = 0; i < cScene2D->getEnemyVec().size(); i++)
	{
		glm::vec2 currIndex = glm::vec2(cScene2D->getEnemyVec()[i]->vec2Index.x, cScene2D->getEnemyVec()[i]->vec2Index.y);
		if (glm::length(currIndex - vec2Index) < glm::length(NEWLive - vec2Index))
		{
			nearestLive = currIndex;
			NEWLive = nearestLive;
			nearestEnemyInt = i;
			nearestEnemy = cScene2D->getEnemyVec()[i];
		}
	}
}

int CPlayer2D::getNearestEnemy(void)
{
	return nearestEnemyInt;
}
