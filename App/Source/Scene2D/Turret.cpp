/**
 CTurret
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Turret.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"

// Include GLEW
#include <GL/glew.h>

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include math.h
#include <math.h>

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CTurret::CTurret(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, sCurrentFSM(FSM::IDLE)
	, iFSMCounter(0)
	, quadMesh(NULL)
{
	transform = glm::mat4(1.0f);	// make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	i32vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	i32vec2Destination = glm::i32vec2(0, 0);	// Initialise the iDestination
	i32vec2Direction = glm::i32vec2(0, 0);		// Initialise the iDirection
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CTurret::~CTurret(void)
{
	// Delete the quadMesh
	if (quadMesh)
	{
		delete quadMesh;
		quadMesh = NULL;
	}

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool CTurret::Init(int uiRow, int uiCol, bool IsWall)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	cScene2D = CScene2D::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/Turret.png", true);
	if (iTextureID == 0)
	{
		cout << "Image/Tiles/tile038.png" << endl;
		return false;
	}

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Set the Physics to fall status by default
	cPhysics2D.Init();

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	cBulletGenerator = new CBulletGenerator();

	// Rand seeding
	srand(time(NULL));

	Time = 0.0;
	CurrTime = 0.0;

	if (IsWall)
	{
		turretType = WOOD_WALL;
		TurretHP = 5;
		TurretDamage = 0;
		TurretCooldown = 0.0;
		range = 0.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		upgradeLeft = STONE_WALL;
		upgradeRight = NONE;
	}
	else
	{
		turretType = TURRET;
		TurretHP = 6;
		TurretDamage = 4;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.588f, 0.294f, 0.f, 1.f);
		upgradeLeft = STONE_TURRET;
		upgradeRight = ELEMENTAL_TURRET;
	}

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	return true;
}

/**
 @brief Update this instance
 */
void CTurret::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return;


	//Monster damage handler
	if (TurretHP <= 0)
	{
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
		bIsActive = false;
		return;
	}

	if (turretType != WOOD_WALL && turretType != STONE_WALL && turretType != IRON_WALL)
	{
		Time += dElapsedTime;

		findNearestEnemy();

		// Generate bullet & limit its firing rate to 1 bullet every 0.2s
		if (Time > (CurrTime + TurretCooldown))
		{
			if (glm::length(vec2Index - nearestLive) <= range)
			{
				CurrTime = Time;
				switch (turretType)
				{
				case TURRET:
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
				}
			}
		}

		for (unsigned i = 0; i < cBulletGenerator->GetBulletsVector().size(); ++i)
		{
			cBulletGenerator->GetBulletsVector()[i]->Update();
		}
	}

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CTurret::PreRender(void)
{
	if (!bIsActive)
		return;

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

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
void CTurret::Render(void)
{
	if (!bIsActive)
		return;

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
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	// Render the tile
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	quadMesh->Render();
	//animatedMisc->Render();
	for (unsigned i = 0; i < cBulletGenerator->GetBulletsVector().size(); ++i)
		cBulletGenerator->GetBulletsVector()[i]->Render();
	glBindVertexArray(0);

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CTurret::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
	for (unsigned i = 0; i < cBulletGenerator->GetBulletsVector().size(); ++i)
		cBulletGenerator->GetBulletsVector()[i]->PostRender();
}

/**
@brief Set the indices of the enemy2D
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void CTurret::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void CTurret::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

void CTurret::SetEnemyVector(vector<CEntity2D*> NEWenemyVector)
{
	enemyVector = NEWenemyVector;
}

int CTurret::GetNearestEnemy()
{
	return nearestEnemyInt;
}

int CTurret::GetTurretHP()
{
	return TurretHP;
}

void CTurret::SetGetTurretHP(int newTurretHP)
{
	TurretHP = newTurretHP;
}

CBulletGenerator* CTurret::GetBulletGenerator()
{
	return cBulletGenerator;
}

/**
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CTurret::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (vec2Index.x < 0)
		{
			vec2Index.x = 0;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y < 0)
		{
			vec2Index.y = 0;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CTurret::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CTurret::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
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
			i32vec2NumMicroSteps.x = 0;
			return true;
		}

		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
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
			i32vec2NumMicroSteps.y = 0;
			return true;
		}

		// If the new position is fully within a column, then check this column only
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
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
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
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
		cout << "CTurret::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

bool CTurret::AdjustPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is between 2 rows, then check both rows as well
		if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) < 100)
			{
				i32vec2NumMicroSteps.y = 0;
				return true;
			}
			else if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) < 100)
			{
				vec2Index.y++;
				i32vec2NumMicroSteps.y = 0;
				return true;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is between 2 rows, then check both rows as well
		if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) < 100)
			{
				i32vec2NumMicroSteps.y = 0;
				return true;
			}
			else if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) < 100)
			{
				vec2Index.y++;
				i32vec2NumMicroSteps.y = 0;
				return true;
			}
		}

	}
	else if (eDirection == UP)
	{
	// If the new position is between 2 columns, then check both columns as well
	if (i32vec2NumMicroSteps.x != 0)
	{
		// If the 2 grids are not accessible, then return false
		if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) < 100)
		{
			i32vec2NumMicroSteps.x = 0;
			return true;
		}
		else if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) < 100)
		{
			vec2Index.x++;
			i32vec2NumMicroSteps.x = 0;
			return true;
		}
	}
	}
	else if (eDirection == DOWN)
	{
	// If the new position is between 2 columns, then check both columns as well
	if (i32vec2NumMicroSteps.x != 0)
	{
		// If the 2 grids are not accessible, then return false
		if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) < 100)
		{
			i32vec2NumMicroSteps.x = 0;
			return true;
		}
		if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) < 100)
		{
			vec2Index.x++;
			i32vec2NumMicroSteps.x = 0;
			return true;
		}
	}
	}
	else
	{
	cout << "CTurret::CheckPosition: Unknown direction." << endl;
	}

	return false;
}

bool CTurret::InteractWithPlayer(void)
{
	return false;
}

/**
 @brief Flip horizontal direction. For patrol use only
 */
void CTurret::FlipHorizontalDirection(void)
{
	i32vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void CTurret::UpdatePosition(void)
{

}

void CTurret::UpgradeTurret(bool IsLeft)
{
	if (IsLeft)
	{
		turretType = upgradeLeft;
	}
	else
	{
		turretType = upgradeRight;
	}

	TurretType upgradeRare;

	switch (turretType) // Setting up the stats for the different types
	{
	case STONE_WALL:
		upgradeLeft = IRON_WALL;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case STONE_TURRET:
		upgradeLeft = REINFORCED_STONE_TURRET;
		upgradeRight = MULTI_PEBBLE_TURRET;
		upgradeRare = RANDOM_DMG_TURRET;
		TurretDamage = 5;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case ELEMENTAL_TURRET:
		upgradeLeft = FLAME_TURRET;
		upgradeRight = FROST_TURRET;
		upgradeRare = MYSTERIOUS_TURRET;
		TurretDamage = 4;
		TurretHP = 6;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.627f, 0.125f, 0.941f, 1.f);
		break;

	// Tier 2
	case REINFORCED_STONE_TURRET:
		upgradeLeft = SHARP_STONE_TURRET;
		upgradeRight = IRON_TURRET;
		upgradeRare = ORE_GENERATOR;
		TurretDamage = 6;
		TurretHP = 12;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case MULTI_PEBBLE_TURRET:
		upgradeLeft = STONE_BURST_TOWER;
		upgradeRight = MULTISHOT_TURRET;
		upgradeRare = TURRET2;
		TurretDamage = 2;
		TurretHP = 4;
		TurretElement = NORMAL;
		TurretCooldown = 0.75;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case RANDOM_DMG_TURRET:
		upgradeLeft = RANDOMDMGTURRETV2;
		upgradeRight = GLITCHED_TURRET;
		upgradeRare = GETRANDOMTURRET;
		TurretDamage = 0;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		break;

	case FLAME_TURRET:
		upgradeLeft = FLAME_SPEAR_TURRET;
		upgradeRight = FLAMETHROWER_TURRET;
		upgradeRare = FIREWALL_TURRET;
		TurretDamage = 8;
		TurretHP = 8;
		TurretElement = BURN;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 0.0f, 0.0f, 1.f);
		break;
	case FROST_TURRET:
		upgradeLeft = ICE_SPEAR_TURRET;
		upgradeRight = SNOWBALL_TURRET;
		upgradeRare = ICE_FLOOR_TURRET;
		TurretDamage = 5;
		TurretHP = 8;
		TurretElement = FROZEN;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.678f, 0.847f, 0.902f, 1.f);
		break;
	case MYSTERIOUS_TURRET:
		upgradeLeft = WIND_TURRET;
		upgradeRight = THUNDER_TURRET;
		upgradeRare = ISTERIOUS_TURRET;
		TurretDamage = 6;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4((rand() % 10) / 10, (rand() % 10) / 10, (rand() % 10) / 10, 1.f);
		break;

	// Tier 3
	case SHARP_STONE_TURRET:
		upgradeLeft = SHARPER_STONE_TURRET;
		upgradeRight = SNIPER_TURRET;
		upgradeRare = BLUNT_METAL_TURRET;
		break;
	case IRON_TURRET:
		upgradeLeft = REINFORCED_IRON_TURRET;
		upgradeRight = SHINY_IRON_TURRET;
		upgradeRare = TANK;
		break;
	case ORE_GENERATOR:
		upgradeLeft = GOLDEN_TURRET;
		upgradeRight = ELEMENTAL_TURRET2;
		upgradeRare = MIDAS_TOUCH;
		break;

	case STONE_BURST_TOWER:
		upgradeLeft = IRON_BURST_TURRET;
		upgradeRight = HOT_IRON_TURRET;
		upgradeRare = REINFORCED_IRON_TURET;
		break;
	case MULTISHOT_TURRET:
		upgradeLeft = MULTIMULTISHOT_TURET;
		upgradeRight = STARSHOT_TURRET;
		upgradeRare = WRONGDIRECTION_TURRET;
		break;
	case TURRET2:
		upgradeLeft = TURRET3;
		upgradeRight = SHOTGUN_TURRET;
		upgradeRare = TURRETINFINITY;
		break;

	case RANDOMDMGTURRETV2:
		upgradeLeft = RANDOM_DMG_TURRETV3;
		upgradeRight = FLIP_A_COIN_TURRET;
		upgradeRare = RANDOM_DIRECTION_TURRET;
		break;
	case GLITCHED_TURRET:
		upgradeLeft = UPGRADED_GLITCHED_TURRET;
		upgradeRight = GETRANDOMTURRET2;
		upgradeRare = ROBOT_PLAYER;
		break;
	case GETRANDOMTURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case FLAME_SPEAR_TURRET:
		upgradeLeft = ETERNAL_FLAME_SPEAR_TURRET;
		upgradeRight = BLUE_FLAME_TURRET;
		upgradeRare = DUO_FLAME_SPEAR_TURRET;
		break;
	case FLAMETHROWER_TURRET: // All Below have not done the turret type yet
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case FIREWALL_TURRET:
		upgradeLeft = WIND_TURRET;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case ICE_SPEAR_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case SNOWBALL_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case ICE_FLOOR_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case WIND_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case THUNDER_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case ISTERIOUS_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	// Tier 4
	case SHARPER_STONE_TURRET:
	case SNIPER_TURRET:
	case BLUNT_METAL_TURRET:

	case REINFORCED_IRON_TURRET:
	case SHINY_IRON_TURRET:
	case TANK:

	case GOLDEN_TURRET:
	case ELEMENTAL_TURRET2:
	case MIDAS_TOUCH:

	case IRON_BURST_TURRET:
	case HOT_IRON_TURRET:
	case REINFORCED_IRON_TURET:

	case MULTIMULTISHOT_TURET:
	case STARSHOT_TURRET:
	case WRONGDIRECTION_TURRET:

	case TURRET3:
	case SHOTGUN_TURRET:
	case TURRETINFINITY:

	case RANDOM_DMG_TURRETV3:
	case FLIP_A_COIN_TURRET:
	case RANDOM_DIRECTION_TURRET:

	case UPGRADED_GLITCHED_TURRET:
	case GETRANDOMTURRET2:
	case ROBOT_PLAYER:

	case ETERNAL_FLAME_SPEAR_TURRET:
	case BLUE_FLAME_TURRET:
	case DUO_FLAME_SPEAR_TURRET:

	case IRON_WALL:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	}

	if (rand() % 2 == 2) // 1 out of 3
	{
		if (rand() % 1 == 1) // 1 out of 2
		{
			upgradeLeft = upgradeRare;
		}
		else
		{
			upgradeRight = upgradeRare;
		}
	}
}

int CTurret::GetTurret(bool IsLeft)
{
	if (IsLeft)
	{
		return upgradeLeft;
	}
	else
	{
		return upgradeRight;
	}
}

int CTurret::GetCurrTurret()
{
	return turretType;
}

void CTurret::findNearestEnemy()
{
	glm::vec2 NEWLive;
	for (int i = 0; i < enemyVector.size(); i++)
	{
		glm::vec2 currIndex = glm::vec2(enemyVector[i]->vec2Index.x, (int)cSettings->NUM_TILES_YAXIS - enemyVector[i]->vec2Index.y - 1);
		if (glm::length(currIndex - vec2Index) < glm::length(NEWLive - vec2Index))
		{
			nearestLive = currIndex;
			NEWLive = nearestLive;
			nearestEnemyInt = i;
			nearestEnemy = enemyVector[i];
		}
	}
}

glm::vec2 CTurret::getTurretPos()
{
	return vec2Index;
}
