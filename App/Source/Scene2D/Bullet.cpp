#include "Bullet.h"

#include "System\ImageLoader.h"

#include "RenderControl\ShaderManager.h"

#include "Primitives/MeshBuilder.h"

CBullet::CBullet(glm::vec2 vec2Index, int direction)
{
	this->vec2Index = vec2Index;
	dir = (DIRECTION)direction;

	// Make sure to initialize matrix to identity matrix first
	transform = glm::mat4(1.0f);

	//this->vec2Index = glm::i32vec2(0);

	vec2NumMicroSteps = glm::i32vec2(0);

	vec2UVCoordinate = glm::vec2(0.0f);

	cSettings = CSettings::GetInstance();

	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	CShaderManager::GetInstance()->Use("Shader2D_Colour");
	SetShader("Shader2D_Colour");

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Load the player texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D/Bullet.tga", true);
	if (iTextureID == 0)
	{
		std::cout << "Unable to load Image/Scene2D/Bullet.tga" << std::endl;
	}

	bIsActive = true;
	RotateAngle = 0.0f;
}

CBullet::~CBullet()
{
	// Optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

void CBullet::Update()
{
	if (!bIsActive)
		return;

	//std::cout << vec2Index.x << " " << vec2Index.y << std::endl;
	switch (dir)
	{
	case DIRECTION::LEFT:
		--vec2Index.x;
		RotateAngle = 270;
		break;
	case DIRECTION::RIGHT:
		++vec2Index.x;
		RotateAngle = 90;
		break;
	case DIRECTION::UP:
		++vec2Index.y;
		RotateAngle = 0;
		break;
	case DIRECTION::DOWN:
		--vec2Index.y;
		RotateAngle = 180;
		break;
	case DIRECTION::LEFT_UP:
		--vec2Index.x;
		++vec2Index.y;
		RotateAngle = 315;
		break;
	case DIRECTION::LEFT_DOWN:
		--vec2Index.x;
		--vec2Index.y;
		RotateAngle = 225;
		break;
	case DIRECTION::RIGHT_UP:
		++vec2Index.x;
		++vec2Index.y;
		RotateAngle = 45;
		break;
	case DIRECTION::RIGHT_DOWN:
		++vec2Index.x;
		--vec2Index.y;
		RotateAngle = 135;
		break;
	default:
		break;
	}

	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS);

	if (vec2Index.x < 0 || vec2Index.x > cSettings->NUM_TILES_XAXIS
		|| vec2Index.y < 0 || vec2Index.y > cSettings->NUM_TILES_YAXIS)
	{
		bIsActive = false;
	}
}

void CBullet::PreRender()
{
	if (!bIsActive)
		return;

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

void CBullet::Render()
{
	if (!bIsActive)
		return;

	glBindVertexArray(VAO);
	// Get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	//transform = glm::rotate(transform, RotateAngle * (3.14159256f / 180), glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x, vec2UVCoordinate.y, 0.0f));
	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	// Bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);
		// CS: Render the animated sprite
		glBindVertexArray(VAO);
		quadMesh->Render();
		glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void CBullet::PostRender()
{
	// Disable blending
	glDisable(GL_BLEND);
}