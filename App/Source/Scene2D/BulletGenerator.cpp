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

void CBulletGenerator::GenerateBullet(glm::vec2 vec2Index, int direction, int NEWDamage, int NewELEMENT, glm::vec4 Colour)
{
	bulletsVector.push_back(new CBullet(vec2Index, direction, NEWDamage, NewELEMENT, Colour));
}

void CBulletGenerator::GenerateBullet(glm::vec2 vec2Index, glm::vec2 Targetvec2Index, int NEWDamage, int NewELEMENT, glm::vec4 Colour)
{
	bulletsVector.push_back(new CBullet(vec2Index, Targetvec2Index, NEWDamage, NewELEMENT, Colour));
}

std::vector<CBullet*>& CBulletGenerator::GetBulletsVector()
{
	return bulletsVector;
}