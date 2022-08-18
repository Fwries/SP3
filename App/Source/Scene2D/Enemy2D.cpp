/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Enemy2D.h"

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
CEnemy2D::CEnemy2D(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::MOVING)
	, iFSMCounter(0)
	, quadMesh(NULL)
	, cSoundController(NULL)
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
CEnemy2D::~CEnemy2D(void)
{
	// Delete the quadMesh
	if (quadMesh)
	{
		delete quadMesh;
		quadMesh = NULL;
	}

	// We won't delete this since it was created elsewhere
	cPlayer2D = NULL;

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
bool CEnemy2D::Init(void)
{

	// Rand seeding
	srand(time(NULL));


	MoveCooldown = 0;
	AttackCooldown = 0;

	TRGE = 1;
	ARGE = 1;

	X = 0;
	Y = 0;
	elapsed = 0;
	spawnRate = 1;

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	//// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	//unsigned int uiRow = -1;
	//unsigned int uiCol = -1;
	////if (cMap2D->FindValue(300, uiRow, uiCol) == false)
	////{
	////	return false;	// Unable to find the start position of the player, so quit this game
	////}
	//
	//if (cMap2D->FindValue(300, uiRow, uiCol) == true)
	//{
	//	enemyType = SKULL;
	//}
	//else if (cMap2D->FindValue(301, uiRow, uiCol) == true)
	//{
	//	enemyType = SKELE1;
	//}
	//else if (cMap2D->FindValue(302, uiRow, uiCol) == true)
	//{
	//	enemyType = VAMPIRE;
	//}
	//else
	//{
	//	return false;	// Unable to find the start position of the player, so quit this game
	//}

	//// Erase the value of the player in the arrMapInfo
	//cMap2D->SetMapInfo(uiRow, uiCol, 0);


	//Set the position of the enemy randomly on the edge of the map
	int edge= rand() % 4;
	int X = 0, Y = 0;
	switch (edge)
	{
	case 0:
		X = rand() % 62;
		Y = 62;
		break;
	case 1:
		X = 1;
		Y = rand() % 62;
		break;
	case 2:
		X = rand() % 62;
		Y = 1;
		break;
	case 3:
		X = 62;
		Y = rand() % 62;
		break;
	}
	if (cMap2D->GetMapInfo(X, Y) != 0)
	{
		return false;
	}
	//Determining enemy type randomly
	//int randType = rand() % 3;
	int randType = 2;
	switch (randType)
	{
	case 0:
		enemyType = SKELE1;
		HP = 20;
		ATK = 1;
		SPE = 1;
		break;
	case 1:
		enemyType = SKULL;
		HP = 12;
		ATK = 4;
		SPE = 1.5;
		break;
	case 2:
		enemyType = VAMPIRE;
		HP = 20;
		ATK = 1;
		SPE = 1;
		break;
	default:
		enemyType = SKELE1;
		HP = 20;
		ATK = 1;
		SPE = 1;
		break;
	}
	cout << enemyType << endl;
	Startvec2Index = vec2Index = glm::i32vec2(X, Y);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	if (enemyType == SKULL)
	{
		// Load the enemy2D texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Skull.png", true);
		if (iTextureID == 0)
		{
			cout << "Image/Skull.png" << endl;
			return false;
		}

		MoveTime = 0.025;
		AttackTime = 0.0f;
	}
	else if (enemyType == SKELE1)
	{
		// Load the enemy2D texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Skeleton1.png", true);
		if (iTextureID == 0)
		{
			cout << "Image/Skeleton1.png" << endl;
			return false;
		}

		MoveTime = 0.03;
		AttackTime = 0.9f;
	}
	else if (enemyType == VAMPIRE)
	{
		// Load the enemy2D texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Vampire.png", true);
		if (iTextureID == 0)
		{
			cout << "Image/Vampire.png" << endl;
			return false;
		}

		MoveTime = 0.03;
		AttackTime = 0.9f;
	}


	animatedEnemy = CMeshBuilder::GenerateSpriteAnimation(5, 4, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedEnemy->AddAnimation("right", 0, 3);
	animatedEnemy->AddAnimation("left", 4, 7);
	animatedEnemy->AddAnimation("Hright", 8, 11);
	animatedEnemy->AddAnimation("Hleft", 12, 15);
	animatedEnemy->AddAnimation("Dright", 18, 19);
	animatedEnemy->AddAnimation("Dleft", 16, 17);
	
	//CS: Play the "idle" animation as default
	animatedEnemy->PlayAnimation("left", -1, 1.0f);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
	
	// Set the Physics to fall status by default
	cPhysics2D.Init();
	//cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;
	faceLeft = true;

	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	cScene2D = CScene2D::GetInstance();

	return true;
}

/**
 @brief Update this instance
 */
void CEnemy2D::Update(const double dElapsedTime)
{
	//Turret damage handler
	for (unsigned j = 0; j < cScene2D->getTurretVec().size(); ++j)
	{
		for (unsigned i = 0; i < cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector().size(); ++i)
		{
			if (cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->GetIsActive() == true)
			{
				if (glm::length(vec2Index - cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->GetBulletPos()) <= 2)
				{
					HP = HP - cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->GetDamage();
					cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->SetbIsActive(false);

					if (HP <= 0)
					{
						cScene2D->getEnemyVec().erase(cScene2D->getEnemyVec().begin() + (cScene2D->getTurretVec()[j]->GetNearestEnemy()));
					}
				}
			}
		}
	}

	//cout << bIsActive << endl;
	if (!bIsActive)
		return;
	MoveCooldown += dElapsedTime;
	AttackCooldown += dElapsedTime;

	switch (sCurrentFSM)
	{
	case MOVING:
	{
		switch (enemyType)
		{
			//Monster1
			case SKELE1:
			{
				//Pathfinding method
				auto path = cMap2D->PathFind(vec2Index, glm::vec2(30, 34), heuristic::euclidean, 10);
				//Calculate new destination
				bool bFirstPosition = true;
				int firstDest = 0;
				for (const auto& coord : path)
				{
					if (bFirstPosition == true)
					{
						// Set a destination
						i32vec2Destination = coord;
						// Calculate the direction between enemy2D and this destination
						i32vec2Direction = i32vec2Destination - vec2Index;
						/*std::cout << coord.x << ", " << coord.y << "\n";*/
						bFirstPosition = false;
					}
					else
					{
						if ((coord - i32vec2Destination) == i32vec2Direction)
						{
							// Set a destination:
							i32vec2Destination = coord;
						}
						else
						{
							break;
						}
					}
					firstDest++;
				}
				/*cout << toX << "    " << toY << endl;*/
				UpdatePosition();
				glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
				if (cPhysics2D.CalculateDistance(vec2Index, glm::vec2(30, 34)) < 1.0f)
				{
					sCurrentFSM = ATTACK;
					iFSMCounter = 0;
				}
				break;
			}
			case SKULL:
			{
				//Pathfinding method
				auto path = cMap2D->PathFind(vec2Index, cPlayer2D->vec2Index, heuristic::euclidean, 10);
				//Calculate new destination
				bool bFirstPosition = true;
				int firstDest = 0;
				for (const auto& coord : path)
				{
					if (bFirstPosition == true)
					{
						// Set a destination
						i32vec2Destination = coord;
						// Calculate the direction between enemy2D and this destination
						i32vec2Direction = i32vec2Destination - vec2Index;
						/*std::cout << coord.x << ", " << coord.y << "\n";*/
						bFirstPosition = false;
					}
					else
					{
						if ((coord - i32vec2Destination) == i32vec2Direction)
						{
							// Set a destination:
							i32vec2Destination = coord;
						}
						else
						{
							break;
						}
					}
					firstDest++;
				}
				UpdatePosition();
				glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
				//Insert damaging part here
				if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 1.0f)
				{
					sCurrentFSM = ATTACK;
					iFSMCounter = 0;
				}
				break;
			}
			case VAMPIRE:
			{
				//Check if there is a targetable turret in the map
				bool targetableTurret = false;
				for (int i = 0; i < /*y*/ 63; i++)
				{
					for (int j = 0; j < /*x*/ 63; j++)
					{
						if (cMap2D->GetMapInfo(i, j) == 150)
						{
							targetableTurret = true;
							break;
						}
					}
				}
				cout << targetableTurret << endl;

				//Pathfinding method
				if (targetableTurret == true)
				{
					glm::vec2 posToGo = findNearestTurret();
					cout << findNearestTurret().x << "   " << findNearestTurret().y << endl;
					auto path = cMap2D->PathFind(vec2Index, posToGo, heuristic::euclidean, 10);
					//Calculate new destination
					bool bFirstPosition = true;
					for (const auto& coord : path)
					{
						if (bFirstPosition == true)
						{
							// Set a destination
							i32vec2Destination = coord;
							// Calculate the direction between enemy2D and this destination
							i32vec2Direction = i32vec2Destination - vec2Index;
							/*std::cout << coord.x << ", " << coord.y << "\n";*/
							bFirstPosition = false;
						}
						else
						{
							if ((coord - i32vec2Destination) == i32vec2Direction)
							{
								// Set a destination:
								i32vec2Destination = coord;
							}
							else
							{
								break;
							}
						}
					}
					UpdatePosition();
					glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
					//Insert damaging part here
					if (cPhysics2D.CalculateDistance(vec2Index, posToGo) < 1.0f)
					{
						sCurrentFSM = ATTACK;
						iFSMCounter = 0;
					}
				}
				else
				{
					auto path = cMap2D->PathFind(vec2Index, glm::vec2(30, 34), heuristic::euclidean, 10);
					//Calculate new destination
					bool bFirstPosition = true;
					for (const auto& coord : path)
					{
						if (bFirstPosition == true)
						{
							// Set a destination
							i32vec2Destination = coord;
							// Calculate the direction between enemy2D and this destination
							i32vec2Direction = i32vec2Destination - vec2Index;
							/*std::cout << coord.x << ", " << coord.y << "\n";*/
							bFirstPosition = false;
						}
						else
						{
							if ((coord - i32vec2Destination) == i32vec2Direction)
							{
								// Set a destination:
								i32vec2Destination = coord;
							}
							else
							{
								break;
							}
						}
					}
				}
				UpdatePosition();
				glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
				if (cPhysics2D.CalculateDistance(vec2Index, glm::vec2(30, 34)) < 1.0f)
				{
					sCurrentFSM = ATTACK;
					iFSMCounter = 0;
				}
			}

		}

		//Checking HP: (Should be outside of enemy type check cos everyone needs this)
		if (HP <= 0)
		{
			sCurrentFSM = DEAD;
			iFSMCounter = 0;
		}

		iFSMCounter++;
		break;
	}
	case BLOCKED:
	{
		
		iFSMCounter++;
		break;
	}
	case ATTACK:
	{
		switch (enemyType)
		{
			case SKELE1:
			{
				if (iFSMCounter >= 40)
				{
					cPlayer2D->changeBaseHP(ATK);
					iFSMCounter = 0;
				}
				break;
			}
			case SKULL:
			{
				if (iFSMCounter >= 40)
				{
					cPlayer2D->changeBaseHP(ATK);
					iFSMCounter = 0;
				}
				break;
			}
			case VAMPIRE:
			{
				if (iFSMCounter >= 40)
				{
					cPlayer2D->changeBaseHP(ATK);
					iFSMCounter = 0;
				}
				break;
			}
		}
		//Checking HP:
		if (HP <= 0)
		{
			sCurrentFSM = DEAD;
			iFSMCounter = 0;
		}
		iFSMCounter++;
		break;
	}
	case DEAD:
	{
		if (enemyType == SKULL)
		{
			bIsActive = false;
		}
		else if (enemyType == SKELE1 || enemyType == VAMPIRE)
		{
			if (faceLeft == true)
			{
				animatedEnemy->PlayAnimation("Dleft", -1, 1.0f);
				if (iFSMCounter >= 50)
				{
					bIsActive = false;
				}
			}
			else
			{
				animatedEnemy->PlayAnimation("Dright", -1, 1.0f);
				if (iFSMCounter >= 50)
				{
					bIsActive = false;
				}
			}
		}
		iFSMCounter++;
		break;
	}
	default:
		break;
	}

	if (sCurrentFSM != DEAD)
	{
		if (faceLeft == true)
		{
			if (GetHitBox() == true)
			{
				animatedEnemy->PlayAnimation("Hleft", -1, 1.0f);
			}
			else
			{
				animatedEnemy->PlayAnimation("left", -1, 1.0f);
			}
		}
		else
		{
			if (GetHitBox() == true)
			{
				animatedEnemy->PlayAnimation("Hright", -1, 1.0f);
			}
			else
			{
				animatedEnemy->PlayAnimation("right", -1, 1.0f);
			}
		}

		// Interact with the Player
		if (InteractWithPlayer())
		{
			if (enemyType == SKULL)
			{
				cPlayer2D->UpdateHealthLives();
				cSoundController->PlaySoundByID(7);
				bIsActive = false;

			}
			else if ((enemyType == SKELE1 || enemyType == VAMPIRE) && sCurrentFSM != DEAD)
			{
				cPlayer2D->SetHitBox(true);
				sCurrentFSM = ATTACK;
			}
		}
	}

	//CS: Update the animated sprite
	animatedEnemy->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CEnemy2D::PreRender(void)
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
}

/**
 @brief Render this instance
 */
void CEnemy2D::Render(void)
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
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	animatedEnemy->Render();
	glBindVertexArray(0);

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CEnemy2D::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Set the indices of the enemy2D
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void CEnemy2D::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void CEnemy2D::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void CEnemy2D::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}


/**
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CEnemy2D::Constraint(DIRECTION eDirection)
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
		cout << "CEnemy2D::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CEnemy2D::CheckPosition(DIRECTION eDirection)
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
		cout << "CEnemy2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

bool CEnemy2D::AdjustPosition(DIRECTION eDirection)
{
	if (eDirection == UP)
	{
		// If the new position is between 2 rows, then check both rows as well
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
		// If the new position is between 2 rows, then check both rows as well
		if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 100)
			{
				i32vec2NumMicroSteps.x = 0;
				return true;
			}
			else if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x + 1) < 100)
			{
				vec2Index.x++;
				i32vec2NumMicroSteps.x = 0;
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
	else if (eDirection == LEFT)
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
	else
	{
		cout << "CEnemy2D::CheckPosition: Unknown direction." << endl;
	}

	return false;
}

// Check if the enemy2D is in mid-air
bool CEnemy2D::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
		return false;

	// Check if the tile below the player's current position is empty
	if ((i32vec2NumMicroSteps.x == 0) &&
		(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 0))
	{
		return true;
	}

	return false;
}

/**
 @brief Let enemy2D interact with the player.
 */
bool CEnemy2D::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
	
	// Comment this out if Faries is not stupid
	if (glm::length(vec2Index - cPlayer2D->vec2Index) <= 2)
	{
		SetHitBox(true);
	}
	else if (glm::length(vec2Index - cPlayer2D->vec2Index) > 2)
	{
		SetHitBox(false);
	}

	for (unsigned i = 0; i < cPlayer2D->GetBulletGenerator()->GetBulletsVector().size(); ++i)
	{
		if (cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->GetIsActive() == true)
		{
			if (glm::length(vec2Index - cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->GetBulletPos()) <= 2)
			{
				HP = HP - cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->GetDamage();
				cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->SetbIsActive(false);
			}
		}
	}


	if (GetHitBox() == true)
	{
		if (cKeyboardController->IsKeyPressed(GLFW_KEY_SPACE))
		{
			if (enemyType == SKULL)
			{
				cSoundController->PlaySoundByID(7);
				sCurrentFSM = DEAD;
				bIsActive = false;
			}
			else if (enemyType == SKELE1 || enemyType == VAMPIRE)
			{
				cSoundController->PlaySoundByID(8);
				sCurrentFSM = DEAD;
			}	
		}
	}

	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) && 
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&& 
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
		(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	{
		// Since the player has been caught, then reset the FSM
		iFSMCounter = 0;
		return true;
	}
	return false;
}

/**
 @brief Update the enemy's direction.
 */
void CEnemy2D::UpdateDirection(void)
{
	// Set the destination to the player
	i32vec2Destination = cPlayer2D->vec2Index;

	// Calculate the direction between enemy2D and player2D
	i32vec2Direction = i32vec2Destination - vec2Index;

	// Calculate the distance between enemy2D and player2D
	float fDistance = cPhysics2D.CalculateDistance(vec2Index, i32vec2Destination);
	if (fDistance >= 0.01f)
	{
		// Calculate direction vector.
		// We need to round the numbers as it is easier to work with whole numbers for movements
		i32vec2Direction.x = (int)round(i32vec2Direction.x/ fDistance);
		i32vec2Direction.y = (int)round(i32vec2Direction.y / fDistance);
	}
	else
	{
		// Since we are not going anywhere, set this to 0.
		i32vec2Direction = glm::i32vec2(0);
	}
}

/**
 @brief Flip horizontal direction. For patrol use only
 */
void CEnemy2D::FlipHorizontalDirection(void)
{
	i32vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void CEnemy2D::UpdatePosition(void)
{
	// Store the old position
	i32vec2OldIndex = vec2Index;

	// if the player is to the left or right of the enemy2D, then jump to attack
	if (i32vec2Direction.x < 0)
	{
		// Move left
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x >= 0)
		{
			i32vec2NumMicroSteps.x -= SPE;
			if (i32vec2NumMicroSteps.x < 0)
			{
				i32vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(LEFT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(LEFT) == false)
		{
			if (AdjustPosition(LEFT) == false)
			{
				FlipHorizontalDirection();
				vec2Index = i32vec2OldIndex;
				i32vec2NumMicroSteps.x = 0;
			}
		}

		faceLeft = true;
		// Interact with the Player
		InteractWithPlayer();
	}
	else if (i32vec2Direction.x > 0)
	{
		// Move right
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		{
			i32vec2NumMicroSteps.x += SPE;

			if (i32vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
			{
				i32vec2NumMicroSteps.x = 0;
				vec2Index.x++;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(RIGHT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(RIGHT) == false)
		{
			/*if (AdjustPosition(RIGHT) == false)*/
			{
				FlipHorizontalDirection();
				/*vec2Index = i32vec2OldIndex;*/
				i32vec2NumMicroSteps.x = 0;
			}
		}

		faceLeft = false;
		// Interact with the Player
		InteractWithPlayer();
	}

	// if the player is above the enemy2D, then move upwards
	if (i32vec2Direction.y > 0)
	{

		const int iOldIndex = vec2Index.y;
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			i32vec2NumMicroSteps.y += SPE;

			if (i32vec2NumMicroSteps.y >= cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				i32vec2NumMicroSteps.y = 0;
				vec2Index.y++;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(UP);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(UP) == false)
		{
			if (AdjustPosition(UP) == false)
			{
				FlipHorizontalDirection();
				/*vec2Index = i32vec2OldIndex;*/
				i32vec2NumMicroSteps.y = 0;
			}
		}
		InteractWithPlayer();
	}
	// if the player is below the enemy2D, then move downward
	if (i32vec2Direction.y < 0)
	{

		const int iOldIndex = vec2Index.y;
		if (vec2Index.y >= 0)
		{
			i32vec2NumMicroSteps.y -= SPE;
			if (i32vec2NumMicroSteps.y < 0)
			{
				i32vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(DOWN);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(DOWN) == false)
		{
			if (AdjustPosition(DOWN) == false)
			{
				FlipHorizontalDirection();
				vec2Index = i32vec2OldIndex;
				i32vec2NumMicroSteps.y = 0;
			}
		}
		InteractWithPlayer();
	}
}

glm::vec2& CEnemy2D::findNearestTurret()
{
	nearestLive = glm::vec2(1000, 1000);
	for (int i = 0; i < cScene2D->getTurretVec().size(); i++)
	{
		glm::vec2 currIndex = glm::vec2(cScene2D->getTurretVec()[i]->vec2Index.x, (int)cSettings->NUM_TILES_YAXIS - cScene2D->getTurretVec()[i]->vec2Index.y - 1);
		if (glm::length(currIndex - vec2Index) < glm::length(nearestLive - vec2Index))
		{
			nearestLive = currIndex;
			nearestTurretInt = i;
			nearestTurret = cScene2D->getTurretVec()[i];
		}
	}
	return nearestTurret->getTurretPos();
}