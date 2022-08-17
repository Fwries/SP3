#include "BulletGenerator.h"

CBulletGenerator::CBulletGenerator()
{
}

CBulletGenerator::~CBulletGenerator()
{
	// Problem with "for (unsigned i = (bulletsVector.size() - 1); i >= 0; --i)" is that the condition of "i >= 0" is always true, when its -1, it changes to 4294967296 [OUT OF RANGE]
	for (unsigned i = bulletsVector.size(); i-- > 0;)
	{
		delete bulletsVector[i];
		bulletsVector.pop_back();
	}
}

void CBulletGenerator::GenerateBullet(glm::vec2 vec2Index, int direction)
{
	bulletsVector.push_back(new CBullet(vec2Index, direction));
}

void CBulletGenerator::GenerateBullet(glm::vec2 vec2Index, glm::vec2 Targetvec2Index)
{
	bulletsVector.push_back(new CBullet(vec2Index, Targetvec2Index));
}

std::vector<CBullet*> CBulletGenerator::GetBulletsVector() const
{
	return bulletsVector;
}