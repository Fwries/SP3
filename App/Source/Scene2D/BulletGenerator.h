#pragma once

#include "Bullet.h"

#include <vector>

class CBulletGenerator
{
private:
	std::vector<CBullet*> bulletsVector;

public:
	CBulletGenerator();
	virtual ~CBulletGenerator();

	void GenerateBullet(glm::vec2 vec2Index, int direction);

	void GenerateBullet(glm::vec2 vec2Index, glm::vec2 Targetvec2Index, int NEWDamage, int NewELEMENT, glm::vec4 Colour);

	std::vector<CBullet*> GetBulletsVector() const;
};
