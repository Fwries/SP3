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
	, status(NORMAL)
	, iFSMCounter(0)
	, statusCounter(0)
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

	cInventoryItem = NULL;
	cInventoryManager = NULL;
	animatedEnemy = NULL;
	cMouseController = NULL;

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
	//srand(time(NULL));


	MoveCooldown = 0;
	AttackCooldown = 0;

	TRGE = 1;
	ARGE = 1;

	X = 0;
	Y = 0;
	elapsed = 0;
	spawnRate = 1;

	statMultiplier = 1;

	spawnDeterminer = 2;

	targetableTurret = false;

	meleeCounter = 0;

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	cInventoryManager = CInventoryManager::GetInstance();

	cMouseController = CMouseController::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();

	cScene2D = CScene2D::GetInstance();

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
	randType = rand() % cScene2D->getSpawnDeterminer();
	//randType = 69;
	


	//Determine monster difficulty
	if (cScene2D->getWaveLevel() == 7)
	{
		statMultiplier = 2;
	}
	else if (cScene2D->getWaveLevel() == 9)
	{
		statMultiplier = 3;
	}
	//int randType = 3;
	switch (randType)
	{
	case 0:
		enemyType = SKELE1;
		HP = 20 * statMultiplier;
		MAXHP = 20 * statMultiplier;
		ATK = 1 * statMultiplier;
		SPE = 1 * statMultiplier;
		break;
	case 1:
		enemyType = SKULL;
		HP = 4 * statMultiplier;
		MAXHP = 4 * statMultiplier;
		ATK = 4 * statMultiplier;
		SPE = 1.5 * statMultiplier;
		break;
	case 2:
		enemyType = VAMPIRE;
		HP = 20 * statMultiplier;
		MAXHP = 20 * statMultiplier;
		ATK = 1 * statMultiplier;
		SPE = 1 * statMultiplier;
		break;
	case 3:
		enemyType = GOBLIN;
		HP = 24 * statMultiplier;
		MAXHP = 24 * statMultiplier;
		ATK = 2 * statMultiplier;
		SPE = 2 * statMultiplier;
		break;
	case 4:
		enemyType = SLIMEBOSS;
		HP = 60 * statMultiplier;
		MAXHP = 60 * statMultiplier;
		ATK = 4 * statMultiplier;
		SPE = 0.9 * statMultiplier;
		break;
	default:
		enemyType = SKELE1;
		HP = 20 * statMultiplier;
		MAXHP = 20 * statMultiplier;
		ATK = 1 * statMultiplier;
		SPE = 1 * statMultiplier;
		break;
	}
	//cout << enemyType << endl;
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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Sprites/Skull.png", true);
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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Sprites/Skeleton1.png", true);
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
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Sprites/Vampire.png", true);
		if (iTextureID == 0)
		{
			cout << "Image/Vampire.png" << endl;
			return false;
		}

		MoveTime = 0.03;
		AttackTime = 0.9f;
	}
	else if (enemyType == SLIMEBOSS)
	{
		// Load the enemy2D texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Sprites/SlimeBig.png", true);
		if (iTextureID == 0)
		{
			cout << "Image/SlimeBig.png" << endl;
			return false;
		}

		MoveTime = 0.03;
		AttackTime = 0.9f;
	}
	else if (enemyType == GOBLIN)
	{
		// Load the enemy2D texture
		iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Sprites/Goblin.png", true);
		if (iTextureID == 0)
		{
			cout << "Image/Goblin.png" << endl;
			return false;
		}

		MoveTime = 0.03;
		AttackTime = 0.9f;
	}


	animatedEnemy = CMeshBuilder::GenerateSpriteAnimation(13, 4, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	//Movement
	animatedEnemy->AddAnimation("right", 0, 3);
	animatedEnemy->AddAnimation("left", 4, 7);
	animatedEnemy->AddAnimation("Hright", 8, 11);
	animatedEnemy->AddAnimation("Hleft", 12, 15);

	//movement with status effect
	//Burn
	animatedEnemy->AddAnimation("burnRight", 16, 19);
	animatedEnemy->AddAnimation("burnLeft", 20, 23);
	animatedEnemy->AddAnimation("burnHright", 24, 27);
	animatedEnemy->AddAnimation("burnHleft", 28, 31);
	//Frozen
	animatedEnemy->AddAnimation("frozenRight", 32, 35);
	animatedEnemy->AddAnimation("frozenLeft", 36, 39);
	animatedEnemy->AddAnimation("frozenHright", 40, 43);
	animatedEnemy->AddAnimation("frozenHleft", 44, 47);
	//Death
	animatedEnemy->AddAnimation("Dright", 48, 51);
	animatedEnemy->AddAnimation("Dleft", 52, 55);


	
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

bool CEnemy2D::babySlimeInit(glm::vec2 bossPos)
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

	statMultiplier = 1;

	targetableTurret = false;

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	cInventoryManager = CInventoryManager::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();

	cMouseController = CMouseController::GetInstance();

	cScene2D = CScene2D::GetInstance();

	//Set the position of the enemy randomly on the edge of the map
	int edge = rand() % 4;
	int X =bossPos.x, Y = bossPos.y;
	//if (cMap2D->GetMapInfo(X, Y) != 0)
	//{
	//	return false;
	//}
	//Determining enemy type randomly
	enemyType = SLIMEBABY;
	HP = 12 * statMultiplier;
	MAXHP = 12 * statMultiplier;
	ATK = 1 * statMultiplier;
	SPE = 1 * statMultiplier;

	//cout << enemyType << endl;
	Startvec2Index = vec2Index = glm::i32vec2(X, Y);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);


	// Load the enemy2D texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Sprites/SlimeSmall.png", true);
	if (iTextureID == 0)
	{
		cout << "Image/SlimeSmall.png" << endl;
		return false;
	}

	MoveTime = 0.025;
	AttackTime = 0.0f;

	animatedEnemy = CMeshBuilder::GenerateSpriteAnimation(13, 4, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	//Movement
	animatedEnemy->AddAnimation("right", 0, 3);
	animatedEnemy->AddAnimation("left", 4, 7);
	animatedEnemy->AddAnimation("Hright", 8, 11);
	animatedEnemy->AddAnimation("Hleft", 12, 15);

	//movement with status effect
	//Burn
	animatedEnemy->AddAnimation("burnRight", 16, 19);
	animatedEnemy->AddAnimation("burnLeft", 20, 23);
	animatedEnemy->AddAnimation("burnHright", 24, 27);
	animatedEnemy->AddAnimation("burnHleft", 28, 31);
	//Frozen
	animatedEnemy->AddAnimation("frozenRight", 32, 35);
	animatedEnemy->AddAnimation("frozenLeft", 36, 39);
	animatedEnemy->AddAnimation("frozenHright", 40, 43);
	animatedEnemy->AddAnimation("frozenHleft", 44, 47);
	//Death
	animatedEnemy->AddAnimation("Dright", 48, 51);
	animatedEnemy->AddAnimation("Dleft", 52, 55);

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

//Slime boss's init
bool CEnemy2D::slimeBossInit(void)
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

	statMultiplier = 1;

	targetableTurret = false;

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	cInventoryManager = CInventoryManager::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();

	cMouseController = CMouseController::GetInstance();

	cScene2D = CScene2D::GetInstance();

	//Set the position of the enemy randomly on the edge of the map
	int edge = rand() % 4;
	int X = 32, Y = 1;
	if (cMap2D->GetMapInfo(X, Y) != 0)
	{
		return false;
	}
	//if (cMap2D->GetMapInfo(X, Y) != 0)
	//{
	//	return false;
	//}
	//Determining enemy type randomly
	enemyType = SLIMEBOSS;
	HP = 120 * statMultiplier;
	MAXHP = 12 * statMultiplier;
	ATK = 3 * statMultiplier;
	SPE = 0.6 * statMultiplier;

	//cout << enemyType << endl;
	Startvec2Index = vec2Index = glm::i32vec2(X, Y);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);


	// Load the enemy2D texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Sprites/SlimeBig.png", true);
	if (iTextureID == 0)
	{
		cout << "Image/SlimeBig.png" << endl;
		return false;
	}

	MoveTime = 0.025;
	AttackTime = 0.0f;

	animatedEnemy = CMeshBuilder::GenerateSpriteAnimation(13, 4, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	//Movement
	animatedEnemy->AddAnimation("right", 0, 3);
	animatedEnemy->AddAnimation("left", 4, 7);
	animatedEnemy->AddAnimation("Hright", 8, 11);
	animatedEnemy->AddAnimation("Hleft", 12, 15);

	//movement with status effect
	//Burn
	animatedEnemy->AddAnimation("burnRight", 16, 19);
	animatedEnemy->AddAnimation("burnLeft", 20, 23);
	animatedEnemy->AddAnimation("burnHright", 24, 27);
	animatedEnemy->AddAnimation("burnHleft", 28, 31);
	//Frozen
	animatedEnemy->AddAnimation("frozenRight", 32, 35);
	animatedEnemy->AddAnimation("frozenLeft", 36, 39);
	animatedEnemy->AddAnimation("frozenHright", 40, 43);
	animatedEnemy->AddAnimation("frozenHleft", 44, 47);
	//Death
	animatedEnemy->AddAnimation("Dright", 48, 51);
	animatedEnemy->AddAnimation("Dleft", 52, 55);

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
	//Color tint display for HP
	if (HP < (MAXHP / 2))
	{
		runtimeColour = glm::vec4(1.0, 0.67, 0.0, 1.0);
	}
	if (HP < ((MAXHP / 100) * 30))
	{
		runtimeColour = glm::vec4(1.0, 0.0, 0.0, 1.0);
	}

	//Melee attack cooldown
	if (meleeCounter < 100)
	{
		meleeCounter++;
	}

	//Turret damage handler
	for (unsigned j = 0; j < cScene2D->getTurretVec().size(); ++j)
	{
		for (int i = cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector().size() - 1; i >= 0; --i)
		{
			if (cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->GetIsActive() == true)
			{
				if (glm::length(vec2Index - cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->GetBulletPos()) <= 2)
				{
					HP = HP - cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->GetDamage();
					switch (cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->GetElement())
					{
						case 1:
						{
							status = BURN;
							break;
						}
						case 2:
						{
							status = FREEZE;
							break;
						}
						case 3:
						{
							int randomDirection = rand() % 4;
							updateWindBLow(randomDirection);
							break;
						}
						case 5:
						{
							if (status == FREEZE)
							{
								HP = HP - cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->GetDamage();
							}
							break;
						}
						default:
						{
							break;
						}
					}
					cSoundController->PlaySoundByID(10);
					if (HP <= 0)
					{
						sCurrentFSM = DEAD;

						if (cScene2D->getEnemyVec().size() >= 0 && bIsActive == true)
						{
							//cout << cScene2D->getTurretVec()[j]->GetNearestEnemy() << endl;
							cScene2D->getEnemyVec().erase(cScene2D->getEnemyVec().begin() + cScene2D->getTurretVec()[j]->GetNearestEnemy());
						}
					}
					if (cScene2D->getTurretVec().size() > 0 && cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()[i]->GetIsActive() == true)
					{
						cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector().erase((cScene2D->getTurretVec()[j]->GetBulletGenerator()->GetBulletsVector()).begin() + i);
					}
				}
			}
		}
	}

	//Status handler
	switch (status)
	{
		case BURN:
		{
			if (statusCounter >= 50)
			{
				HP = HP - 1;
				statusCounter = 0;
			}
			statusCounter++;
			break;
		}
		case FREEZE:
		{
			if (statusCounter >= 40 && sCurrentFSM == MOVING || sCurrentFSM == ATTACK)
			{
				statusCounter = 0;
				iFSMCounter = 0;
				sCurrentFSM = FROZEN;
			}
			if (sCurrentFSM != FROZEN)
			{
				statusCounter++;
			}
			break;
		}
		default:
		{
			if (statusCounter <= 100)
			{
				statusCounter++;
			}
			break;
		}
	}

	//cout << bIsActive << endl;
	if (!bIsActive)
		return;
	MoveCooldown += dElapsedTime;
	AttackCooldown += dElapsedTime;

	//FSM handler
	switch (sCurrentFSM)
	{
	case MOVING:
	{
		if (faceLeft == true)
		{
			if (hitBox == true)
			{
				animatedEnemy->PlayAnimation("Hleft", -1, 1.0f);
			}
			else
			{
				animatedEnemy->PlayAnimation("Left", -1, 1.0f);
			}
		}
		else
		{
			if (hitBox == true)
			{
				animatedEnemy->PlayAnimation("Hright", -1, 1.0f);
			}
			else
			{
				animatedEnemy->PlayAnimation("Right", -1, 1.0f);
			}
		}
		switch (enemyType)
		{
			//Monster1
			case SKELE1:
			case SLIMEBOSS:
			case SLIMEBABY:
			{
				glm::vec2 posToGo = findNearestBasePart();
				//auto path = cMap2D->PathFind(vec2Index, posToGo, heuristic::euclidean, 10);
				////Calculate new destination
				//bool bFirstPosition = true;
				//for (const auto& coord : path)
				//{
				//	if (bFirstPosition == true)
				//	{
				//		// Set a destination
				//		i32vec2Destination = coord;
				//		// Calculate the direction between enemy2D and this destination
				//		i32vec2Direction = i32vec2Destination - vec2Index;
				//		bFirstPosition = false;
				//		cout << coord.x << "  " << coord.y << endl;
				//	}
				//	else
				//	{
				//		if ((coord - i32vec2Destination) == i32vec2Direction)
				//		{
				//			// Set a destination:
				//			i32vec2Destination = coord;
				//			//cout << coord.x << "  " << coord.y << endl;
				//		}
				//		else
				//		{
				//			//cout << coord.x << "  " << coord.y << endl;
				//			break;
				//		}
				//	}
				//}
				auto path = cMap2D->PathFind(vec2Index, posToGo, heuristic::euclidean, 10);
				//Calculate new destination
				bool bFirstPosition = true;
				int firstDest = 0;
				for (const auto& coord : path)
				{
					/*std::cout << coord.x << ", " << coord.y << "\n";*/
					if (bFirstPosition == true)
					{
						if (firstDest == 0)
						{
							// Set a destination
							i32vec2Destination = coord;
							// Calculate the direction between enemy2D and this destination
							i32vec2Direction = i32vec2Destination - vec2Index;
						}
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
				UpdatePosition(glm::vec2(30, 34));
				glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
				if (cPhysics2D.CalculateDistance(vec2Index, posToGo) <= 2.f)
				{
					sCurrentFSM = ATTACK;
					iFSMCounter = 0;
				}
				//else if (path.size() == 0)
				//{
				//	sCurrentFSM = BLOCKED;
				//	iFSMCounter = 0;
				//}
				break;
			}

			case SKULL:
			case GOBLIN:
			{
				auto path = cMap2D->PathFind(vec2Index, cPlayer2D->vec2Index, heuristic::euclidean, 10);
				//Calculate new destination
				bool bFirstPosition = true;
				int firstDest = 0;
				for (const auto& coord : path)
				{
					/*std::cout << coord.x << ", " << coord.y << "\n";*/
					if (bFirstPosition == true)
					{
						if (firstDest == 0)
						{
							// Set a destination
							i32vec2Destination = coord;
							// Calculate the direction between enemy2D and this destination
							i32vec2Direction = i32vec2Destination - vec2Index;
						}
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
				UpdatePosition(cPlayer2D->vec2Index);
				glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
				//Insert damaging part here
				if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) <= 0.5f)
				{
					sCurrentFSM = ATTACK;
					iFSMCounter = 0;
				}
				break;
			}
			case VAMPIRE:
			{
				//Check if there is a targetable turret in the map
				targetableTurret = false;
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
				//cout << targetableTurret << endl;

				//Pathfinding method
				if (targetableTurret == true)
				{
					glm::vec2 posToGo = findNearestTurret();
					//cout << findNearestTurret().x << "   " << findNearestTurret().y << endl;
					auto path = cMap2D->PathFind(vec2Index, posToGo, heuristic::euclidean, 10);
					//Calculate new destination
					bool bFirstPosition = true;
					int firstDest = 0;
					for (const auto& coord : path)
					{
						/*std::cout << coord.x << ", " << coord.y << "\n";*/
						if (bFirstPosition == true)
						{
							if (firstDest == 0)
							{
								// Set a destination
								i32vec2Destination = coord;
								// Calculate the direction between enemy2D and this destination
								i32vec2Direction = i32vec2Destination - vec2Index;
							}
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
					UpdatePosition(posToGo);
					glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
					//Insert damaging part here
					if (cPhysics2D.CalculateDistance(vec2Index, posToGo) <= 1.f)
					{
						sCurrentFSM = ATTACK;
						iFSMCounter = 0;
					}
					//else if (path.size() == 0)
					//{
					//	sCurrentFSM = BLOCKED;
					//	iFSMCounter = 0;
					//}
				}
				else
				{
					glm::vec2 posToGo = findNearestBasePart();
					//cout << posToGo.x << "   " << posToGo.y << endl;
					auto path = cMap2D->PathFind(vec2Index, posToGo, heuristic::euclidean, 10);
					//Calculate new destination
					bool bFirstPosition = true;
					int firstDest = 0;
					for (const auto& coord : path)
					{
						/*std::cout << coord.x << ", " << coord.y << "\n";*/
						if (bFirstPosition == true)
						{
							if (firstDest == 0)
							{
								// Set a destination
								i32vec2Destination = coord;
								// Calculate the direction between enemy2D and this destination
								i32vec2Direction = i32vec2Destination - vec2Index;
							}
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
					UpdatePosition(glm::vec2(30, 34));
					glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
					if (cPhysics2D.CalculateDistance(vec2Index, posToGo) <= 2.f)
					{
						sCurrentFSM = ATTACK;
						iFSMCounter = 0;
					}
					//else if (path.size() == 0)
					//{
					//	sCurrentFSM = BLOCKED;
					//	iFSMCounter = 0;
					//}
					break;
				}
			}

		}

		iFSMCounter++;
		break;
	}
	case BLOCKED:
	{
		runtimeColour = glm::vec4(0.67, 0.0, 0.50, 1.0);
		iFSMCounter++;
		break;
	}
	case ATTACK:
	{
		switch (enemyType)
		{
			case SKELE1:
			case SLIMEBOSS:
			case SLIMEBABY:
			{
				if (iFSMCounter >= 40)
				{
					cPlayer2D->changeBaseHP(ATK);
					cSoundController->PlaySoundByID(9);
					iFSMCounter = 0;
				}
				break;
			}
			case SKULL:
			{
				cPlayer2D->UpdateHealthLives();
				cSoundController->PlaySoundByID(7);
				if (cScene2D->getEnemyVec().size() >= 0 && bIsActive == true)
				{
					cPlayer2D->findNearestEnemy();
					cScene2D->getEnemyVec().erase(cScene2D->getEnemyVec().begin() + cPlayer2D->getNearestEnemy());
				}
				iFSMCounter = 0;
				break;
			}
			case VAMPIRE:
			{
				if (iFSMCounter >= 40 && targetableTurret == false)
				{
					cPlayer2D->changeBaseHP(ATK);
					cSoundController->PlaySoundByID(9);
					iFSMCounter = 0;
				}
				else if (iFSMCounter >= 40 && targetableTurret == true)
				{
					for (unsigned i = 0; i < cScene2D->getTurretVec().size(); ++i)
					{
						if (cScene2D->getTurretVec()[i]->getTurretPos() == findNearestTurret())
						{
							cScene2D->getTurretVec()[i]->SetGetTurretHP((cScene2D->getTurretVec()[i]->GetTurretHP() - ATK));
							cSoundController->PlaySoundByID(9);
							if (cScene2D->getTurretVec()[i]->GetTurretHP() <= 0)
							{
								sCurrentFSM = MOVING;
							}
							iFSMCounter = 0;
							/*cout << cScene2D->getTurretVec()[i]->GetTurretHP() << "     " << cScene2D->getTurretVec().size() << endl;*/
						}
					}
				}
				break;
			}
			case GOBLIN:
			{
				if (iFSMCounter >= 40)
				{
					if (cInventoryManager->GetItem("Iron")->GetCount() > 0 || cInventoryManager->GetItem("Bronze")->GetCount() > 0 ||
						cInventoryManager->GetItem("Silver")->GetCount() > 0 || cInventoryManager->GetItem("Gold")->GetCount() > 0)
					{
						if (cInventoryManager->GetItem("Iron")->GetCount() > 0)
						{
							cInventoryItem = cInventoryManager->GetItem("Iron");
							cInventoryItem->Remove(1);
						}
						if (cInventoryManager->GetItem("Bronze")->GetCount() > 0)
						{
							cInventoryItem = cInventoryManager->GetItem("Bronze");
							cInventoryItem->Remove(1);
						}
						if (cInventoryManager->GetItem("Silver")->GetCount() > 0)
						{
							cInventoryItem = cInventoryManager->GetItem("Silver");
							cInventoryItem->Remove(1);
						}
						if (cInventoryManager->GetItem("Gold")->GetCount() > 0)
						{
							cInventoryItem = cInventoryManager->GetItem("Gold");
							cInventoryItem->Remove(1);
						}
						cSoundController->PlaySoundByID(11);
					}
					else
					{
						if (iFSMCounter >= 40)
						{
							cPlayer2D->UpdateHealthLives();
							cSoundController->PlaySoundByID(12);
							iFSMCounter = 0;
						}
					}
					iFSMCounter = 0;
				}
				if (distance(vec2Index, cPlayer2D->vec2Index) >= 2)
				{
					sCurrentFSM = MOVING;
					iFSMCounter = 0;
				}
				break;
			}
		}
		iFSMCounter++;
		break;
	}
	case DEAD:
	{
		if (status == GOLD)
		{
			cInventoryItem = cInventoryManager->GetItem("Gold");
			cInventoryItem->Add(1);
		}
		if (enemyType == SKULL || enemyType == SLIMEBABY)
		{
			bIsActive = false;
		}
		else if (enemyType == SKELE1 || enemyType == VAMPIRE || enemyType == GOBLIN)
		{
			if (faceLeft == true)
			{
				animatedEnemy->PlayAnimation("Dleft", -1, 1.0f);
				if (iFSMCounter >= 43)
				{
					bIsActive = false;
				}
			}
			else
			{
				animatedEnemy->PlayAnimation("Dright", -1, 1.0f);
				if (iFSMCounter >= 43)
				{
					bIsActive = false;
				}
			}
		}
		else if (enemyType == SLIMEBOSS)
		{
			cScene2D->setSlimeBPos(vec2Index);
			cScene2D->spawnExtraEnemy(4);
			bIsActive = false;
		}

		iFSMCounter++;
		break;
	}
	case FROZEN:
	{
		if (faceLeft == true)
		{
			if (hitBox == true)
			{
				animatedEnemy->PlayAnimation("frozenHleft", -1, 1.0f);
			}
			else
			{
				animatedEnemy->PlayAnimation("frozenLeft", -1, 1.0f);
			}
		}
		else
		{
			if (hitBox == true)
			{
				animatedEnemy->PlayAnimation("frozenHright", -1, 1.0f);
			}
			else
			{
				animatedEnemy->PlayAnimation("frozenRight", -1, 1.0f);
			}
		}
		if (iFSMCounter >= 70)
		{
			sCurrentFSM = MOVING;
			iFSMCounter = 0;
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
				switch (status)
				{
				case NORMAL:
					animatedEnemy->PlayAnimation("Hleft", -1, 1.0f);
					break;
				case BURN:
					animatedEnemy->PlayAnimation("burnHleft", -1, 1.0f);
					break;
				}
				//if (sCurrentFSM == FROZEN)
				//{
				//	animatedEnemy->PlayAnimation("frozenHleft", -1, 1.0f);
				//}
			}
			else
			{
				switch (status)
				{
				case NORMAL:
					animatedEnemy->PlayAnimation("left", -1, 1.0f);
					break;
				case BURN:
					animatedEnemy->PlayAnimation("burnLeft", -1, 1.0f);
					break;
				}
				//if (sCurrentFSM == FROZEN)
				//{
				//	animatedEnemy->PlayAnimation("frozenleft", -1, 1.0f);
				//}
			}
		}
		else
		{
			if (GetHitBox() == true)
			{
				switch (status)
				{
				case NORMAL:
					animatedEnemy->PlayAnimation("Hright", -1, 1.0f);
					break;
				case BURN:
					animatedEnemy->PlayAnimation("burnHright", -1, 1.0f);
					break;
				}
				//if (sCurrentFSM == FROZEN)
				//{
				//	animatedEnemy->PlayAnimation("frozenHright", -1, 1.0f);
				//}
			}
			else
			{
				switch (status)
				{
				case NORMAL:
					animatedEnemy->PlayAnimation("right", -1, 1.0f);
					break;
				case BURN:
					animatedEnemy->PlayAnimation("burnRight", -1, 1.0f);
					break;
				}
				//if (sCurrentFSM == FROZEN)
				//{
				//	animatedEnemy->PlayAnimation("frozenRight", -1, 1.0f);
				//}
			}
		}

		// Interact with the Player
		if (InteractWithPlayer())
		{
			if (enemyType == SKULL)
			{
				cPlayer2D->UpdateHealthLives();
				cSoundController->PlaySoundByID(7);
				sCurrentFSM = DEAD;
				if (cScene2D->getEnemyVec().size() >= 0 && bIsActive == true)
				{
					cPlayer2D->findNearestEnemy();
					cScene2D->getEnemyVec().erase(cScene2D->getEnemyVec().begin() + cPlayer2D->getNearestEnemy());
				}

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
void CEnemy2D::Render(const glm::mat4& view, const glm::mat4& projection)
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
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection * view * transform));
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
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 1) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 1) >= 100) && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150 ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x - 1) >= 100) && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150)
			{
				return false;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100) && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150 ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100) && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150)
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
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100) && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150 ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100) && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150)
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
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150 ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100) && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 150)
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

	//Player ranged attack interaction
	for (unsigned i = 0; i < cPlayer2D->GetBulletGenerator()->GetBulletsVector().size(); ++i)
	{
		if (cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->GetIsActive() == true)
		{
			if (glm::length(vec2Index - cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->GetBulletPos()) <= 2)
			{
				HP = HP - cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->GetDamage();
				cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->SetbIsActive(false);
			}
			if (HP <= 0)
			{
				if (status == GOLD)
				{
					cInventoryItem = cInventoryManager->GetItem("Gold");
					cInventoryItem->Add(1);
				}
				sCurrentFSM = DEAD;
				if (cScene2D->getEnemyVec().size() >= 0 && bIsActive == true)
				{
					cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->findNearestEnemy();
					cout << cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->getNearestEnemy() << endl;
					cScene2D->getEnemyVec().erase(cScene2D->getEnemyVec().begin() + (cPlayer2D->GetBulletGenerator()->GetBulletsVector()[i]->getNearestEnemy()));
				}
			}
		}
	}

	//Player melee attack interaction
	if (GetHitBox() == true)
	{
		if (cMouseController->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT) && meleeCounter >= 40)
		{
			cSoundController->PlaySoundByID(5);
			if (enemyType == SKULL)
			{
				HP = HP - 4;
			}
			else if (enemyType == SKELE1 || enemyType == VAMPIRE || enemyType == GOBLIN)
			{
				HP = HP - 4;
			}

			if (HP <= 0)
			{
				sCurrentFSM = DEAD;
				cSoundController->PlaySoundByID(7);
				if (cScene2D->getEnemyVec().size() >= 0 && bIsActive == true)
				{
					cPlayer2D->findNearestEnemy();
					cScene2D->getEnemyVec().erase(cScene2D->getEnemyVec().begin() + cPlayer2D->getNearestEnemy());
				}
			}
			meleeCounter = 0;
		}
	}

	// Check if the enemy2D is within 1.5 indices of the player2D
	//if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) && 
	//	(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
	//	&& 
	//	((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
	//	(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	//{
	//	// Since the player has been caught, then reset the FSM
	//	iFSMCounter = 0;
	//	return true;
	//}
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
void CEnemy2D::FlipVerticalDirection(void)
{
	i32vec2Direction.y *= -1;
}

/**
@brief Update position.
*/
void CEnemy2D::UpdatePosition(glm::vec2 destination)
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
			if (i32vec2NumMicroSteps.x <= -((int)cSettings->NUM_STEPS_PER_TILE_XAXIS))
			{
				i32vec2NumMicroSteps.x = 0;
				vec2Index.x--;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(LEFT);
		//Find a feasible position for the enemy2D's current position
		if (CheckPosition(LEFT) == false)
		{
			//FlipHorizontalDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
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
			//FlipHorizontalDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
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
			//FlipVerticalDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.y = 0;
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
			if (i32vec2NumMicroSteps.y <= 0)
			{
				i32vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS);
				vec2Index.y--;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(DOWN);
		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(DOWN) == false)
		{
			//FlipVerticalDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS);
		}
		InteractWithPlayer();
	}
}

glm::vec2& CEnemy2D::findNearestTurret()
{
	nearestLive = glm::vec2(1000, 1000);
	for (int i = cScene2D->getTurretVec().size() - 1; i >= 0; i--)
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

glm::vec2& CEnemy2D::findNearestBasePart()
{
	nearestLive = glm::vec2(1000, 1000);
	for (int i = 64 - 1; i >= 0; i--)
	{
		for (int j = 64 - 1; j >= 0; j--)
		{
			if (cMap2D->GetMapInfo(j, i) >= 136 && cMap2D->GetMapInfo(j, i) <= 139)
			{
				glm::vec2 currIndex = glm::vec2(j, i);
				if (glm::length(currIndex - vec2Index) < glm::length(nearestLive - vec2Index))
				{
					nearestLive = currIndex;
					nearestBasePart = glm::vec2(((int)cSettings->NUM_TILES_XAXIS) - j - 1, ((int)cSettings->NUM_TILES_YAXIS) - i - 1);
				}
			}
		}
	}
	return nearestBasePart;
}

void CEnemy2D::updateWindBLow(int direction)
{
	int distancedBlown = 0;
	switch (direction)
	{
		case 0:
		{
			Constraint(LEFT);
			if (cMap2D->GetMapInfo(vec2Index.x - 2, vec2Index.y) == 0)
			{
				vec2Index.x -= 2;
			}
			break;
		}
		case 1:
		{
			Constraint(RIGHT);
			if (cMap2D->GetMapInfo(vec2Index.x + 2, vec2Index.y) == 0)
			{
				vec2Index.x += 2;
			}
			break;
		}
		case 2:
		{
			Constraint(UP);
			if (cMap2D->GetMapInfo(vec2Index.x, vec2Index.y + 2) == 0)
			{
				vec2Index.y += 2;
			}
			break;
		}
		case 3:
		{
			Constraint(DOWN);
			if (cMap2D->GetMapInfo(vec2Index.x, vec2Index.y - 2) == 0)
			{
				vec2Index.y -= 2;
			}
			break;
		}
	}
}