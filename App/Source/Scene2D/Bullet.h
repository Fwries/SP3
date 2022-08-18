#pragma once

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

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
	};

	DIRECTION dir;
	int ElementType;
	bool bIsActive;
	float RotateAngle;
	int Damage;
	bool FromTurret;

	glm::vec2 Targetvec2Index;

public:
	CMesh* quadMesh;

	CBullet(glm::vec2 vec2Index, int direction);
	CBullet(glm::vec2 vec2Index, glm::vec2 targetvec2Index, int NEWDamage, int NewELEMENT);
	virtual ~CBullet();

	void Update();

	void PreRender();

	void Render();

	void PostRender();

	bool GetIsActive();

	int GetDamage();

	void SetbIsActive(bool);

	glm::vec2 GetBulletPos();
};