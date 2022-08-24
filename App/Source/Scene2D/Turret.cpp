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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/WoodWall.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/WoodWall.png" << endl;
		}
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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/Turret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/Turret.png" << endl;
			return false;
		}
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
		for (int i = cScene2D->getTurretVec().size() - 1; i >= 0; --i)
		{
			if (cScene2D->getTurretVec()[i]->getTurretPos() == vec2Index && cScene2D->getTurretVec().size() > 0)
			{
				cScene2D->getTurretVec().erase(cScene2D->getTurretVec().begin() + i);
			}
		}
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
			if (glm::length(vec2Index - nearestLive) <= range && nearestEnemy != nullptr)
			{
				CurrTime = Time;
				switch (turretType)
				{
				// Normal Turret
				case TURRET:
				case STONE_TURRET:
				case ELEMENTAL_TURRET:
				case REINFORCED_STONE_TURRET:
				case MULTI_PEBBLE_TURRET:
				case SHARP_STONE_TURRET:
				case IRON_TURRET:
				case WIND_TURRET:
				case MYSTERIOUS_TURRET:
				case SHARPER_STONE_TURRET:
				case SNIPER_TURRET:
				case BLUNT_METAL_TURRET:
				case MIDAS_TOUCH:
				case REINFORCED_IRON_TURRET:
				case REINFORCED_IRON_TURRET2:
				case SHINY_IRON_TURRET:
				case GOLDEN_TURRET:
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
					break;
				
				// Elemental Turret
				case FLAME_TURRET:
				case FROST_TURRET:
				case FLAME_SPEAR_TURRET:
				case FLAMETHROWER_TURRET:
				case FIREWALL_TURRET:
				case ICE_SPEAR_TURRET:
				case SNOWBALL_TURRET:
				case ICE_FLOOR_TURRET:
				case HOT_IRON_TURRET:
				case ETERNAL_FLAME_SPEAR_TURRET:
					if (rand() % 101 <= ElementChance)
					{
						cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
					}
					else
					{
						cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, NORMAL, Colour);
					}
					break;
				case BLUE_FLAME_TURRET:
				case DUO_FLAME_SPEAR_TURRET:
					if (rand() % 101 <= ElementChance)
					{
						cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
					}
					else
					{
						cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, NORMAL, Colour);
					}
					break;

				// Random DMG Turret
				case RANDOM_DMG_TURRET:
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, rand() % 10 + 4, TurretElement, Colour);
					break;
				case RANDOM_DMG_TURRETV2:
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, rand() % 12 + 6, TurretElement, Colour);
					break;
				case RANDOM_DMG_TURRETV3:
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, rand() % 15 + 9, TurretElement, Colour);
					break;

				// GetRandomTurret
				case GETRANDOMTURRET:
				case GETRANDOMTURRET2:
				{
					int Path = rand() % 4;
					if (Path == 0 || Path == 1)
					{
						Path = 1;
					}
					else if (Path == 2 || Path == 3)
					{
						Path = 2;
					}
					else
					{
						Path = 3;
					}
					int Tier = rand() % 5 + 1;
					int BranchNo = 0;
					switch (Tier)
					{
					case 1:
						BranchNo = 1;
						if (Path == 3)
						{
							Path = rand() % 3 + 1;
						}
						break;
					case 2:
						BranchNo = rand() % 3 + 1;
						break;
					case 3:
						BranchNo = rand() % 7 + 1;
						break;
					case 4:
						BranchNo = rand() % 19 + 1;
						break;
					}
					upgradeLeft = int(Path + BranchNo * 10 + Tier * 1000);
					UpgradeTurret(true);
					break;
				}

				// TuretTuretTuret
				case TURRET3:
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
				case TURRET2:
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
					break;

				// Burst
				case STONE_BURST_TOWER:
				case IRON_BURST_TURRET:
					break;

				// Rainbow
				case ISTERIOUS_TURRET:
				case TURRETINFINITY:
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
					Colour = glm::vec4(static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, 1);
					break;

					// Glitched
				case GLITCHED_TURRET:
					TurretCooldown = static_cast<float>((rand() % 250)) / 100.f;
					Colour = glm::vec4(static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, 1);
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
					break;
				case UPGRADED_GLITCHED_TURRET:
					TurretCooldown = static_cast<float>((rand() % 150)) / 100.f;
					Colour = glm::vec4(static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, 1);
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, TurretDamage, TurretElement, Colour);
					break;

				// Multishot
				case MULTISHOT_TURRET:
					cBulletGenerator->GenerateBullet(this->vec2Index, 0, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 1, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 2, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 3, TurretDamage, TurretElement, Colour);
					break;
				case MULTIMULTISHOT_TURRET:
					cBulletGenerator->GenerateBullet(this->vec2Index, 0, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 1, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 2, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 3, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 4, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 5, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 6, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 7, TurretDamage, TurretElement, Colour);
					break;
				case STARSHOT_TURRET:
					cBulletGenerator->GenerateBullet(this->vec2Index, 0, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 1, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 2, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 5, TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, 7, TurretDamage, TurretElement, Colour);
					break;

				// Misc
				case ORE_GENERATOR:
					break;
				case THUNDER_TURRET:
					cBulletGenerator->GenerateBullet(glm::vec2(nearestEnemy->vec2Index.x, nearestEnemy->vec2Index.y + 5), 3, TurretDamage, TurretElement, Colour);
					break;
				case TANK:
					break;
				case WRONGDIRECTION_TURRET:
					cBulletGenerator->GenerateBullet(this->vec2Index, -(nearestEnemy->vec2Index), TurretDamage, TurretElement, Colour);
					break;
				case SHOTGUN_TURRET:
					cBulletGenerator->GenerateBullet(this->vec2Index, glm::vec2(nearestEnemy->vec2Index.x - 2, nearestEnemy->vec2Index.y - 2), TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, glm::vec2(nearestEnemy->vec2Index.x, nearestEnemy->vec2Index.y), TurretDamage, TurretElement, Colour);
					cBulletGenerator->GenerateBullet(this->vec2Index, glm::vec2(nearestEnemy->vec2Index.x + 2, nearestEnemy->vec2Index.y + 2), TurretDamage, TurretElement, Colour);
					break;
				case FLIP_A_COIN_TURRET:
				{
					int Damage;
					if (rand() % 2 == 1)
					{
						Damage = 20;
					}
					else
					{
						Damage = 0;
					}
					cBulletGenerator->GenerateBullet(this->vec2Index, nearestEnemy->vec2Index, Damage, TurretElement, Colour);
					break;
				}
				case RANDOM_DIRECTION_TURRET:
				{
					int Direction = rand() % 8;
					cBulletGenerator->GenerateBullet(this->vec2Index, Direction);
					break;
				}
				case ROBOT_PLAYER:
					break;
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

	int upgradeRare = NONE;

	switch (turretType) // Setting up the stats for the different types
	{
	case STONE_WALL:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/StoneWall.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/StoneWall.png" << endl;
		}
		upgradeLeft = IRON_WALL;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case STONE_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/StoneTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/StoneTurret.png" << endl;
		}
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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/ElementalTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/ElementalTurret.png" << endl;
		}
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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/ReinforcedStoneTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/ReinforcedStoneTurret.png" << endl;
		}
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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/MultiPebbleTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/MultiPebbleTurret.png" << endl;
		}
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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/RandomDMGTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/RandomDMGTurret.png" << endl;
		}
		upgradeLeft = RANDOM_DMG_TURRETV2;
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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/FlameTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/FlameTurret.png" << endl;
		}
		upgradeLeft = FLAME_SPEAR_TURRET;
		upgradeRight = FLAMETHROWER_TURRET;
		upgradeRare = FIREWALL_TURRET;
		TurretDamage = 8;
		TurretHP = 8;
		TurretElement = BURN;
		ElementChance = 20;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 0.0f, 0.0f, 1.f);
		break;
	case FROST_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/FrostTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/FrostTurret.png" << endl;
		}
		upgradeLeft = ICE_SPEAR_TURRET;
		upgradeRight = SNOWBALL_TURRET;
		upgradeRare = ICE_FLOOR_TURRET;
		TurretDamage = 5;
		TurretHP = 8;
		TurretElement = FROZEN;
		ElementChance = 5;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.678f, 0.847f, 0.902f, 1.f);
		break;
	case MYSTERIOUS_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/MysteriousTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/MysteriousTurret.png" << endl;
		}
		upgradeLeft = WIND_TURRET;
		upgradeRight = THUNDER_TURRET;
		upgradeRare = ISTERIOUS_TURRET;
		TurretDamage = 6;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, 1);
		break;

	// Tier 3
	case SHARP_STONE_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/SharpStoneTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/SharpStoneTurret.png" << endl;
		}
		upgradeLeft = SHARPER_STONE_TURRET;
		upgradeRight = SNIPER_TURRET;
		upgradeRare = BLUNT_METAL_TURRET;
		TurretDamage = 8;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case IRON_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/IronTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/IronTurret.png" << endl;
		}
		upgradeLeft = REINFORCED_IRON_TURRET;
		upgradeRight = SHINY_IRON_TURRET;
		upgradeRare = TANK;
		TurretDamage = 8;
		TurretHP = 22;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		break;
	case ORE_GENERATOR:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/OreGenerator.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/OreGenerator.png" << endl;
		}
		upgradeLeft = GOLDEN_TURRET;
		upgradeRight = ELEMENTAL_TURRET2;
		upgradeRare = MIDAS_TOUCH;
		TurretDamage = 0;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 10.0;
		range = 100.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		break;

	case STONE_BURST_TOWER:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/StoneBurstTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/StoneBurstTurret.png" << endl;
		}
		upgradeLeft = IRON_BURST_TURRET;
		upgradeRight = HOT_IRON_TURRET;
		upgradeRare = REINFORCED_IRON_TURRET;
		TurretDamage = 4;
		TurretHP = 6;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case MULTISHOT_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/MultishotTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/MultishotTurret.png" << endl;
		}
		upgradeLeft = MULTIMULTISHOT_TURRET;
		upgradeRight = STARSHOT_TURRET;
		upgradeRare = WRONGDIRECTION_TURRET;
		TurretDamage = 4;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case TURRET2:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/Turret2.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/Turret2.png" << endl;
		}
		upgradeLeft = TURRET3;
		upgradeRight = SHOTGUN_TURRET;
		upgradeRare = TURRETINFINITY;
		TurretDamage = 4;
		TurretHP = 4;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;

	case RANDOM_DMG_TURRETV2:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/RandomDMGTurretVer2.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/RandomDMGTurretVer2.png" << endl;
		}
		upgradeLeft = RANDOM_DMG_TURRETV3;
		upgradeRight = FLIP_A_COIN_TURRET;
		upgradeRare = RANDOM_DIRECTION_TURRET;
		TurretDamage = 0;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.f, 0.f, 0.f, 1.f);
		break;
	case GLITCHED_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/GlitchedTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/GlitchedTurret.png" << endl;
		}
		upgradeLeft = UPGRADED_GLITCHED_TURRET;
		upgradeRight = GETRANDOMTURRET2;
		upgradeRare = ROBOT_PLAYER;
		TurretDamage = 4;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, 1);
		break;
	case GETRANDOMTURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/GetRandomTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/GetRandomTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case FLAME_SPEAR_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/FlameSpearTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/FlameSpearTurret.png" << endl;
		}
		upgradeLeft = ETERNAL_FLAME_SPEAR_TURRET;
		upgradeRight = BLUE_FLAME_TURRET;
		upgradeRare = DUO_FLAME_SPEAR_TURRET;
		TurretDamage = 8;
		TurretHP = 10;
		TurretElement = BURN;
		ElementChance = 20;
		TurretCooldown = 1.5;
		range = 20.0;
		Colour = glm::vec4(1.f, 0.f, 0.f, 1.f);
		break;
	case FLAMETHROWER_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/FlamethrowerTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/FlamethrowerTurret.png" << endl;
		}
		upgradeLeft = UPGRADED_FLAMETHROWER_TURRET;
		upgradeRight = FLAMEBLOWER_TURRET;
		upgradeRare = BLUE_FLAMETHROWER_TURRET;
		TurretDamage = 0;
		TurretHP = 16;
		TurretElement = BURN;
		ElementChance = 100;
		TurretCooldown = 0.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 0.f, 0.f, 1.f);
		break;
	case FIREWALL_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/FirewallTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/FirewallTurret.png" << endl;
		}
		upgradeLeft = UPGRADED_FIREWALL_TURRET;
		upgradeRight = MULTIFIRE_TURRET;
		upgradeRare = FIRE_TORNADO_TURRET;
		break;

	case ICE_SPEAR_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/IceSpearTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/IceSpearTurret.png" << endl;
		}
		upgradeLeft = ETERNAL_ICE_SPEAR_TURRET;
		upgradeRight = FROSTBITE_TURRET;
		upgradeRare = ETERNAL_BLIZZARD_TURRET;
		TurretDamage = 8;
		TurretHP = 10;
		TurretElement = FROZEN;
		ElementChance = 5;
		TurretCooldown = 1.5;
		range = 20.0;
		Colour = glm::vec4(0.678f, 0.847f, 0.902f, 1.f);
		break;
	case SNOWBALL_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/SnowballTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/SnowballTurret.png" << endl;
		}
		upgradeLeft = GIANT_SNOWBALL;
		upgradeRight = SNOWSTAR_TURRET;
		upgradeRare = ETERNAL_BLIZZARD_TURRET2;
		TurretDamage = 0;
		TurretHP = 8;
		TurretElement = FROZEN;
		ElementChance = 100;
		TurretCooldown = 0.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		break;
	case ICE_FLOOR_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/IceFloorTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/IceFloorTurret.png" << endl;
		}
		upgradeLeft = UPGRADED_ICE_FLOOR_TURRET;
		upgradeRight = FROSTBITE_TURRET;
		upgradeRare = ETERNAL_BLIZZARD_TURRET3;
		break;

	case WIND_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/WindTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/WindTurret.png" << endl;
		}
		upgradeLeft = STRONG_WIND_TURRET;
		upgradeRight = BLOWBACK_TURRET;
		upgradeRare = WINDY_TURRET;
		TurretDamage = 0;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 0.5;
		range = 10.0;
		Colour = glm::vec4(0.224f, 1.f, 0.078f, 1.f);
		break;
	case THUNDER_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/ThunderTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/ThunderTurret.png" << endl;
		}
		upgradeLeft = STORM_TURRET;
		upgradeRight = FINAL_THUNDER;
		upgradeRare = TRIPLE_THUNDER_TURRET;
		TurretDamage = 16;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 3.0;
		range = 10.0;
		Colour = glm::vec4(1.f, 1.f, 0.f, 1.f);
		break;
	case ISTERIOUS_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/IsteriousTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/IsteriousTurret.png" << endl;
		}
		upgradeLeft = YOUSTERIOUS_TURRET;
		upgradeRight = THEYSTERIOUS_TURRET;
		upgradeRare = WESTERIOUS_TURRET;
		TurretDamage = -5;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, 1);
		break;

	// Tier 4
	case SHARPER_STONE_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/SharperStoneTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/SharperStoneTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 10;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case SNIPER_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/SniperTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/SniperTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 10;
		TurretHP = 4;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 100.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case BLUNT_METAL_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/BluntMetalTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/BluntMetalTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 10;
		TurretHP = 15;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 5.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;

	case REINFORCED_IRON_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/ReinforcedIronTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/ReinforcedIronTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 10;
		TurretHP = 24;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		break;
	case SHINY_IRON_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/ShinyIronTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/ShinyIronTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 6;
		TurretHP = 24;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		break;
	case TANK:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/Tank.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/Tank.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case GOLDEN_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/GoldenTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/GoldenTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 7;
		TurretHP = 4;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 0.843f, 0.0f, 1.f);
		break;
	case ELEMENTAL_TURRET2:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/ElementalTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/ElementalTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 4;
		TurretHP = 6;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.627f, 0.125f, 0.941f, 1.f);
		break;
	case MIDAS_TOUCH:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/MidasTouch.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/MidasTouch.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 25;
		TurretHP = 30;
		TurretElement = NORMAL;
		TurretCooldown = 20.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 0.843f, 0.f, 1.f);
		break;

	case IRON_BURST_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/IronBurstTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/IronBurstTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 6;
		TurretHP = 12;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case HOT_IRON_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/HotIronTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/HotIronTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 4;
		TurretHP = 6;
		TurretElement = BURN;
		ElementChance = 10;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 0.f, 0.f, 1.f);
		break;
	case REINFORCED_IRON_TURRET2:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/ReinforcedIronTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/ReinforcedIronTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 10;
		TurretHP = 24;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		break;

	case MULTIMULTISHOT_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/MultimultishotTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/MultimultishotTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 4;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case STARSHOT_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/StarshotTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/StarshotTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 8;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 1.f, 0.f, 1.f);
		break;
	case WRONGDIRECTION_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/WrongDirectionTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/WrongDirectionTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 20;
		TurretHP = 8;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.f, 0.f, 0.f, 1.f);
		break;

	case TURRET3:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/Turret3.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/Turret3.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 9;
		TurretHP = 9;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case SHOTGUN_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/ShotgunTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/ShotgunTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 6;
		TurretHP = 4;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
		break;
	case TURRETINFINITY:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/TurretInfinity.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/TurretInfinity.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 9999;
		TurretHP = 9999;
		TurretElement = NORMAL;
		TurretCooldown = 999.9;
		range = 10.0;
		Colour = glm::vec4(static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, 1);
		break;

	case RANDOM_DMG_TURRETV3:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/RandomDMGTurretVer3.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/RandomDMGTurretVer3.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 0;
		TurretHP = 15;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.f, 0.f, 0.f, 1.f);
		break;
	case FLIP_A_COIN_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/FlipACoinTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/FlipACoinTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 20;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(1.f, 0.843f, 0.0f, 1.f);
		break;
	case RANDOM_DIRECTION_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/RandomDirectionTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/RandomDirectionTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 15;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.f, 0.f, 0.f, 1.f);
		break;

	case UPGRADED_GLITCHED_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/UpgradedGlitchedTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/UpgradedGlitchedTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 8;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, static_cast<float>((rand() % 100)) / 100.f, 1);
		break;
	case GETRANDOMTURRET2:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/GetRandomTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/GetRandomTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case ROBOT_PLAYER:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/RobotPlayer.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/RobotPlayer.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case ETERNAL_FLAME_SPEAR_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/EternalFlameSpearTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/EternalFlameSpearTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 8;
		TurretHP = 10;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 20.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		ElementChance = 100;
		break;
	case BLUE_FLAME_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/BlueFlameSpearTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/BlueFlameSpearTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 4;
		TurretHP = 6;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 10.0;
		Colour = glm::vec4(0.f, 0.f, 1.f, 1.f);
		break;
	case DUO_FLAME_SPEAR_TURRET:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/DuoFlameSpearTurret.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/DuoFlameSpearTurret.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		TurretDamage = 4;
		TurretHP = 6;
		TurretElement = NORMAL;
		TurretCooldown = 1.5;
		range = 20.0;
		Colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
		break;

	case UPGRADED_FLAMETHROWER_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case FLAMEBLOWER_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case BLUE_FLAMETHROWER_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case UPGRADED_FIREWALL_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case MULTIFIRE_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case FIRE_TORNADO_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case ETERNAL_ICE_SPEAR_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case FROSTBITE_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case ETERNAL_BLIZZARD_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case GIANT_SNOWBALL:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case SNOWSTAR_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case ETERNAL_BLIZZARD_TURRET2:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case UPGRADED_ICE_FLOOR_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case FROSTBITE_TURRET2:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case ETERNAL_BLIZZARD_TURRET3:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case STRONG_WIND_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case BLOWBACK_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case WINDY_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case STORM_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case FINAL_THUNDER:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case TRIPLE_THUNDER_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case YOUSTERIOUS_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case THEYSTERIOUS_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	case WESTERIOUS_TURRET:
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;

	case IRON_WALL:
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Turret/IronWall.png", true);
		if (iTextureID == 0)
		{
			cout << "Unable to load Image/Tiles/IronWall.png" << endl;
		}
		upgradeLeft = NONE;
		upgradeRight = NONE;
		upgradeRare = NONE;
		break;
	}

	if (upgradeLeft != NONE && upgradeRight != NONE && upgradeRare != NONE)
	{
		if (rand() % 4 == 0) // 1 out of 4
		{
			if (rand() % 2 == 0) // 1 out of 2
			{
				upgradeLeft = upgradeRare;
			}
			else
			{
				upgradeRight = upgradeRare;
			}
		}
	}
}

int CTurret::GetNextTurret(bool IsLeft)
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
	nearestEnemy = nullptr;
	for (int i = 0; i < enemyVector.size(); i++)
	{
		glm::vec2 currIndex = glm::vec2(enemyVector[i]->vec2Index.x, enemyVector[i]->vec2Index.y);
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
