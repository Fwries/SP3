#pragma once

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include vector
#include <vector>

class CBullet : public CEntity2D
{
private:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT,
		UP,
		DOWN,
		LEFT_UP,
		LEFT_DOWN,
		RIGHT_UP,
		RIGHT_DOWN,
		NUM_DIRECTIONS
	};

	enum ELEMENT
	{
		NONE = 0,
		FLAME = 1,
		FREEZE = 2,
		WIND = 3,
		GOLD = 4,
		FORST = 5,
	};

	DIRECTION dir;
	int ElementType;
	bool bIsActive;
	float RotateAngle;
	int Damage;
	bool Directional;
	float BulletSpeed;

	glm::vec2 Targetvec2Index;
	glm::vec2 DivVector;

	// Current color
	glm::vec4 runtimeColour;

	// A vector containing the instance of CEnemy2Ds
	vector<CEntity2D*> enemyVector;

	glm::vec2 nearestLive;
	CEntity2D* nearestEnemy;
	int nearestEnemyInt;

public:
	CMesh* quadMesh;

	CBullet(glm::vec2 vec2Index, int direction);
	CBullet(glm::vec2 vec2Index, int direction, int NEWDamage, int NewELEMENT, glm::vec4 Colour);
	CBullet(glm::vec2 vec2Index, glm::vec2 targetvec2Index, int NEWDamage, int NewELEMENT, glm::vec4 Colour);
	virtual ~CBullet();

	void Update();

	void PreRender();

	void Render();

	void PostRender();

	bool GetIsActive();

	int GetDamage();

	int GetElement();

	void SetbIsActive(bool);

	glm::vec2 GetBulletPos();

	void SetEnemyVector(vector<CEntity2D*>);

	void findNearestEnemy();
};