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

	void GenerateBullet(glm::vec2 vec2Index, glm::vec2 Targetvec2Index);

	std::vector<CBullet*> GetBulletsVector() const;
};
