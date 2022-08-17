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

	std::vector<CBullet*> GetBulletsVector() const;
};
