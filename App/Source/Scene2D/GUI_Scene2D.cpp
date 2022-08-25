/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
class CTurret;

#include "GUI_Scene2D.h"

#include "../GameStateManagement/GameStateManager.h"

// Include Image loader
#include "System/ImageLoader.h"

#include "Turret.h"

#include <iostream>
#include <string>
using namespace std;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::CGUI_Scene2D(void)
	: cSettings(NULL)
	, m_fProgressBar(0.0f)
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::~CGUI_Scene2D(void)
{
	if (cInventoryManager)
	{
		cInventoryManager->Destroy();
		cInventoryManager = NULL;
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// We won't delete this since it was created elsewhere
	cSettings = NULL;
}

/**
  @brief Initialise this instance
  */
bool CGUI_Scene2D::Init(void)
{
	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Store the CFPSCounter singleton instance here
	cFPSCounter = CFPSCounter::GetInstance();

	cPlayer2D = CPlayer2D::GetInstance();

	cScene2D = CScene2D::GetInstance();

	CImageLoader* Image = CImageLoader::GetInstance();
	InventoryBG.fileName = "Image\\GUI\\Inventory.png";
	InventoryBG.textureID = Image->LoadTextureGetID(InventoryBG.fileName.c_str(), true);
	EquipButton.fileName = "Image\\GUI\\Equip.png";
	EquipButton.textureID = Image->LoadTextureGetID(EquipButton.fileName.c_str(), true);
	EquippedButton.fileName = "Image\\GUI\\Equipped.png";
	EquippedButton.textureID = Image->LoadTextureGetID(EquippedButton.fileName.c_str(), true);
	CraftingBG.fileName = "Image\\GUI\\Crafting.png";
	CraftingBG.textureID = Image->LoadTextureGetID(CraftingBG.fileName.c_str(), true);
	LeftButton.fileName = "Image\\GUI\\LeftButton.png";
	LeftButton.textureID = Image->LoadTextureGetID(LeftButton.fileName.c_str(), true);
	RightButton.fileName = "Image\\GUI\\RightButton.png";
	RightButton.textureID = Image->LoadTextureGetID(RightButton.fileName.c_str(), true);
	CraftButton.fileName = "Image\\GUI\\Craft.png";
	CraftButton.textureID = Image->LoadTextureGetID(CraftButton.fileName.c_str(), true);
	UpgradeBG.fileName = "Image\\GUI\\UpgradeMenu.png";
	UpgradeBG.textureID = Image->LoadTextureGetID(UpgradeBG.fileName.c_str(), true);
	UpgradeButton.fileName = "Image\\GUI\\Upgrade.png";
	UpgradeButton.textureID = Image->LoadTextureGetID(UpgradeButton.fileName.c_str(), true);
	DestroyButton.fileName = "Image\\GUI\\Destroy.png";
	DestroyButton.textureID = Image->LoadTextureGetID(DestroyButton.fileName.c_str(), true);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(CSettings::GetInstance()->pWindow, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Define the window flags
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	//// Show the mouse pointer
	//glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Initialise the cInventoryManager
	cInventoryManager = CInventoryManager::GetInstance();
	// Add a Tree as one of the inventory items
	cInventoryItem = cInventoryManager->Add("Plank", "Image/Materials/Plank.png", 999, 0);
	cInventoryItem = cInventoryManager->Add("Stone", "Image/Materials/Stone.png", 999, 0);
	cInventoryItem = cInventoryManager->Add("Iron", "Image/Materials/Iron.png", 999, 0);
	cInventoryItem = cInventoryManager->Add("Silver", "Image/Materials/Silver.png", 999, 0);
	cInventoryItem = cInventoryManager->Add("Bronze", "Image/Materials/Bronze.png", 999, 0);
	cInventoryItem = cInventoryManager->Add("Gold", "Image/Materials/Gold.png", 999, 0);
	cInventoryItem = cInventoryManager->Add("Coal", "Image/Materials/Coal.png", 999, 0);
	cInventoryItem = cInventoryManager->Add("Turret", "Image/Turret/Turret.png", 999, 100);
	cInventoryItem = cInventoryManager->Add("WoodWall", "Image/Turret/WoodWall.png", 999, 0);
	cInventoryItem = cInventoryManager->Add("StoneWall", "Image/Turret/StoneWall.png", 999, 100);
	cInventoryItem = cInventoryManager->Add("IronWall", "Image/Turret/IronWall.png", 999, 100);
	cInventoryItem = cInventoryManager->Add("Coin", "Image/Tiles/tile086.png", 999, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	// All Turret Images
	{
		for(int i=0;i<2;i++)
		{ 
			TurretImg.push_back(ImageData());
		}
		TurretImg[0].fileName = "Image/Turret/StoneTurret.png";
		TurretImg[1].fileName = "Image/Turret/ElementalTurret.png";
	}
	for (int i = 0; i < TurretImg.size(); i++)
	{
		TurretImg[i].textureID = Image->LoadTextureGetID(TurretImg[i].fileName.c_str(), false);
	}

	recipeNo = 0;
	openCrafting = false;
	openInventory = false;
	openUpgrade = false;
	Upgrade = 0;

	// Variables for buildings
	itemEquipped = 0;
	return true;
}

/**
 @brief Update this instance
 */

void CGUI_Scene2D::Update(const double dElapsedTime)
{
	// Calculate the relative scale to our default windows width
	const float relativeScale_x = cSettings->iWindowWidth / 800.0f;
	const float relativeScale_y = cSettings->iWindowHeight / 600.0f;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// If the OpenGL window is minimised, then don't render the IMGUI widgets
	if ((relativeScale_x == 0.0f) || (relativeScale_y == 0.0f))
	{
		return;
	}

	// Create an invisible window which covers the entire OpenGL window
	ImGui::Begin("Invisible window", NULL, window_flags);
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
	{
		ImGuiWindowFlags inventoryButtonWinFlags =
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;

		if (cKeyboardController->IsKeyPressed(GLFW_KEY_E))
		{
			openInventory = !openInventory;
			openCrafting = false;
			openUpgrade = false;
		}

		if (cKeyboardController->IsKeyPressed(GLFW_KEY_F))
		{
			openCrafting = !openCrafting;
			openInventory = false;
			openUpgrade = false;
		}
		if (!openCrafting || !openInventory || !openUpgrade)
		{
			iTextureID = NULL;
			LeftUpgrade = NULL;
			RightUpgrade = NULL;
		}
		if (openInventory)
		{
			// Enable cursor
			if (CSettings::GetInstance()->bDisableMousePointer == true)
				glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			ImGuiWindowFlags inventoryBgWinFlags =
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoBackground;
			ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.6f, (float)cSettings->iWindowHeight * 0.1f));
			ImGui::SetNextWindowSize(ImVec2(400.0f * relativeScale_x, 500.0f * relativeScale_y));
			ImGui::Begin("Inventory Background", NULL, inventoryBgWinFlags);
			{
				ImGui::Image((ImTextureID)InventoryBG.textureID,
					ImVec2(300.0f * relativeScale_x,
						500.0f * relativeScale_y),
				ImVec2(0, 1), ImVec2(1, 0));
				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
				{
					ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.6125f, (float)cSettings->iWindowHeight * 0.155f));
					ImGui::SetNextWindowSize(ImVec2(280.0f * relativeScale_x, 455.0f * relativeScale_y));
					ImGui::Begin("Inventory", NULL, inventoryBgWinFlags);
					{
						// Plank UI
						{
							cInventoryItem = cInventoryManager->GetItem("Plank");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.65f, (float)cSettings->iWindowHeight * 0.20f));
							ImGui::BeginChild("Plank Child ", ImVec2(110.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Plank Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
							}
							ImGui::EndChild();
						}
						// Stone UI
						{
							cInventoryItem = cInventoryManager->GetItem("Stone");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.20f));

							ImGui::BeginChild("Stone Child ", ImVec2(110.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Stone Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
							}
							ImGui::EndChild();
						}
						// Iron UI
						{
							cInventoryItem = cInventoryManager->GetItem("Iron");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.65f, (float)cSettings->iWindowHeight * 0.30f));

							ImGui::BeginChild("Iron Child ", ImVec2(110.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Iron Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
							}
							ImGui::EndChild();
						}
						// Silver UI
						{
							cInventoryItem = cInventoryManager->GetItem("Silver");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.30f));

							ImGui::BeginChild("Silver Child ", ImVec2(110.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Silver Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
							}
							ImGui::EndChild();
						}
						// Bronze UI
						{
							cInventoryItem = cInventoryManager->GetItem("Bronze");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.65f, (float)cSettings->iWindowHeight * 0.40f));

							ImGui::BeginChild("Bronze Child ", ImVec2(110.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Bronze Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
							}
							ImGui::EndChild();
						}
						// Gold UI
						{
							cInventoryItem = cInventoryManager->GetItem("Gold");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.40f));

							ImGui::BeginChild("Gold Child ", ImVec2(110.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Gold Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
							}
							ImGui::EndChild();
						}
						// Turret UI
						{
							cInventoryItem = cInventoryManager->GetItem("Turret");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.65f, (float)cSettings->iWindowHeight * 0.50f));

							ImGui::BeginChild("Turret Child ", ImVec2(110.0f * relativeScale_x, 90.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Turret Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
								if (itemEquipped == 1)
								{
									ImGui::Image((ImTextureID)EquippedButton.textureID, ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								}
								else
								{
									ImGui::Image((ImTextureID)EquipButton.textureID, ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.65f, (float)cSettings->iWindowHeight * 0.61f));
								ImGui::BeginChild("Turret Function ", ImVec2(110.0f * relativeScale_x, 80.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
									{
										ImGui::SetWindowFontScale(0.5 * relativeScale_y);
										if (ImGui::Button((" "), ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y)))
										{
											if (itemEquipped == 1)
											{
												itemEquipped = 0;
											}
											else if(cInventoryItem->GetCount() != 0)
											{
												itemEquipped = 1;
											}
										}
									}
									ImGui::PopStyleColor(3);
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}
						// Wood Wall UI
						{
							cInventoryItem = cInventoryManager->GetItem("WoodWall");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.50f));
							ImGui::BeginChild("Wood Wall Child ", ImVec2(110.0f * relativeScale_x, 90.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Wood Wall Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
								if (itemEquipped == 2)
								{
									ImGui::Image((ImTextureID)EquippedButton.textureID, ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								}
								else
								{
									ImGui::Image((ImTextureID)EquipButton.textureID, ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.61f));
								ImGui::BeginChild("Wood Wall Function ", ImVec2(110.0f * relativeScale_x, 80.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
									{
										ImGui::SetWindowFontScale(0.5 * relativeScale_y);
										if (ImGui::Button((" "), ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y)))
										{
											if (itemEquipped == 2)
											{
												itemEquipped = 0;
											}
											else if (cInventoryItem->GetCount() != 0)
											{
												itemEquipped = 2;
											}
										}
									}
									ImGui::PopStyleColor(3);
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}
						// Stone Wall UI
						{
							cInventoryItem = cInventoryManager->GetItem("StoneWall");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.65f, (float)cSettings->iWindowHeight * 0.67f));

							ImGui::BeginChild("Stone Wall Child ", ImVec2(110.0f * relativeScale_x, 90.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Stone Wall Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
								if (itemEquipped == 3)
								{
									ImGui::Image((ImTextureID)EquippedButton.textureID, ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								}
								else
								{
									ImGui::Image((ImTextureID)EquipButton.textureID, ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.65f, (float)cSettings->iWindowHeight * 0.78f));
								ImGui::BeginChild("Stone Wall Function ", ImVec2(110.0f * relativeScale_x, 80.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
									{
										ImGui::SetWindowFontScale(0.5 * relativeScale_y);
										if (ImGui::Button((" "), ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y)))
										{
											if (itemEquipped == 3)
											{
												itemEquipped = 0;
											}
											else if (cInventoryItem->GetCount() != 0)
											{
												itemEquipped = 3;
											}
										}
									}
									ImGui::PopStyleColor(3);
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}
						// Iron Wall UI
						{
							cInventoryItem = cInventoryManager->GetItem("IronWall");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.67f));

							ImGui::BeginChild("Iron Wall Child ", ImVec2(110.0f * relativeScale_x, 90.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Iron Wall Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%d",
									cInventoryItem->GetCount());
								if (itemEquipped == 4)
								{
									ImGui::Image((ImTextureID)EquippedButton.textureID, ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								}
								else
								{
									ImGui::Image((ImTextureID)EquipButton.textureID, ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.78f));
								ImGui::BeginChild("Iron Function ", ImVec2(110.0f * relativeScale_x, 80.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
									{
										ImGui::SetWindowFontScale(0.5 * relativeScale_y);
										if (ImGui::Button((" "), ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y)))
										{
											if (itemEquipped == 4)
											{
												itemEquipped = 0;
											}
											else if (cInventoryItem->GetCount() != 0)
											{
												itemEquipped = 4;
											}
										}
									}
									ImGui::PopStyleColor(3);
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}
					}
					ImGui::End();
				}
				ImGui::PopStyleColor(1);
			}
			ImGui::End();
		}
		else if (openCrafting)
		{
			// Enable Cursor
			if (CSettings::GetInstance()->bDisableMousePointer == true)
				glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			ImGuiWindowFlags inventoryBgWinFlags =
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoBackground;
			ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.20f, (float)cSettings->iWindowHeight * 0.25f));
			ImGui::SetNextWindowSize(ImVec2(500.0f * relativeScale_x, 300.0f * relativeScale_y));
			ImGui::Begin("Crafting Background", NULL, inventoryBgWinFlags);
			{
				ImGui::Image((ImTextureID)CraftingBG.textureID, ImVec2(500.0f * relativeScale_x, 300.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));

				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
				{
					ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.20f, (float)cSettings->iWindowHeight * 0.25f));
					ImGui::SetNextWindowSize(ImVec2(500.0f * relativeScale_x, 300.0f * relativeScale_y));
					ImGui::Begin("Recipe", NULL, inventoryBgWinFlags);
					{
						if (recipeNo == 0)
						{
							cInventoryItem = cInventoryManager->GetItem("Turret");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.64f, (float)cSettings->iWindowHeight * 0.47f));
							ImGui::BeginChild("Turret Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Turret Image ", ImVec2(65.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(65.0f * relativeScale_x,
											65.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							cInventoryItem = cInventoryManager->GetItem("Plank");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.32f, (float)cSettings->iWindowHeight * 0.38f));
							ImGui::BeginChild("Plank Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Plank Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x3");
							}
							ImGui::EndChild(); 

							cInventoryItem = cInventoryManager->GetItem("Stone");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.32f, (float)cSettings->iWindowHeight * 0.58f));
							ImGui::BeginChild("Stone Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Stone Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x3");
							}
							ImGui::EndChild();
						}
						else if (recipeNo == 1)
						{
							cInventoryItem = cInventoryManager->GetItem("WoodWall");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.64f, (float)cSettings->iWindowHeight * 0.47f));
							ImGui::BeginChild("Wood Wall Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Wood Wall Image ", ImVec2(65.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(65.0f * relativeScale_x,
											65.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							cInventoryItem = cInventoryManager->GetItem("Plank");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.32f, (float)cSettings->iWindowHeight * 0.48f));

							ImGui::BeginChild("Plank Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Plank Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x3");
							}
							ImGui::EndChild();
						}
						else if (recipeNo == 2)
						{
							cInventoryItem = cInventoryManager->GetItem("StoneWall");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.64f, (float)cSettings->iWindowHeight * 0.47f));
							ImGui::BeginChild("Stone Wall Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Stone Wall Image ", ImVec2(65.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(65.0f * relativeScale_x,
											65.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
							cInventoryItem = cInventoryManager->GetItem("Stone");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.32f, (float)cSettings->iWindowHeight * 0.48f));

							ImGui::BeginChild("Stone Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Stone Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x3");
							}
							ImGui::EndChild();
						}
						else if (recipeNo == 3)
						{
							cInventoryItem = cInventoryManager->GetItem("IronWall");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.64f, (float)cSettings->iWindowHeight * 0.47f));
							ImGui::BeginChild("Iron Wall Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Iron Wall Image ", ImVec2(65.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(65.0f * relativeScale_x,
											65.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
							cInventoryItem = cInventoryManager->GetItem("Iron");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.32f, (float)cSettings->iWindowHeight * 0.48f));

							ImGui::BeginChild("Iron Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Iron Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((ImTextureID)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
								ImGui::SetWindowFontScale(1.5 * relativeScale_y);
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x3");
							}
							ImGui::EndChild();
						}

						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.47f, (float)cSettings->iWindowHeight * 0.67f));
						ImGui::BeginChild("Craft button", ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{
							ImGui::Image((ImTextureID)CraftButton.textureID, ImVec2(90.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.47f, (float)cSettings->iWindowHeight * 0.67f));
							ImGui::BeginChild("Craft button function", ImVec2(90.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
								{
									if (ImGui::Button("  ", ImVec2(90.0f*relativeScale_x, 30.0f*relativeScale_y)))
									{
										switch (recipeNo)
										{
										case 0:
											if (cInventoryManager->GetItem("Plank")->GetCount() >= 3 && cInventoryManager->GetItem("Plank")->GetCount() >= 3)
											{
												cInventoryManager->GetItem("Plank")->Remove(3);
												cInventoryManager->GetItem("Stone")->Remove(3);
												cInventoryManager->GetItem("Turret")->Add(1);
											}
											break;
										case 1:
											if (cInventoryManager->GetItem("Plank")->GetCount() >= 3)
											{
												cInventoryManager->GetItem("Plank")->Remove(3);
												cInventoryManager->GetItem("WoodWall")->Add(1);
											}
											break; 
										case 2:
											if (cInventoryManager->GetItem("Stone")->GetCount() >= 3)
											{
												cInventoryManager->GetItem("Stone")->Remove(3);
												cInventoryManager->GetItem("StoneWall")->Add(1);
											}
											break;
										case 3:
											if (cInventoryManager->GetItem("Iron")->GetCount() >= 3)
											{
												cInventoryManager->GetItem("Iron")->Remove(3);
												cInventoryManager->GetItem("IronWall")->Add(1);
											}
											break;
										default:
											break;
										}
									}
								}
								ImGui::PopStyleColor(3); 
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();

						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.25f, (float)cSettings->iWindowHeight * 0.67f));
						ImGui::BeginChild("Left button", ImVec2(30.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{

							ImGui::Image((ImTextureID)LeftButton.textureID, ImVec2(30.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.25f, (float)cSettings->iWindowHeight * 0.67f));
							ImGui::BeginChild("Left button function", ImVec2(30.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
								{
									if (ImGui::Button(" ", ImVec2(30.0f*relativeScale_x, 30.0f*relativeScale_y)))
									{
										if (recipeNo != 0)
										{
											recipeNo--;
										}
										else
										{
											recipeNo = 3;
										}
									}
								}
								ImGui::PopStyleColor(3);
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();


						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.76f, (float)cSettings->iWindowHeight * 0.67f));
						ImGui::BeginChild("Right button", ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{

							ImGui::Image((ImTextureID)RightButton.textureID, ImVec2(30.0f * relativeScale_x, 30.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.76f, (float)cSettings->iWindowHeight * 0.67f));
							ImGui::BeginChild("Right button function", ImVec2(30.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
								{
									if (ImGui::Button(" ", ImVec2(30.0f*relativeScale_x, 30.0f*relativeScale_y)))
									{
										if (recipeNo != 3)
										{
											recipeNo++;
										}
										else
										{
											recipeNo = 0;
										}
									}
								}
								ImGui::PopStyleColor(3);
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();
					}
					ImGui::End();
				}
				ImGui::PopStyleColor();
			}
			ImGui::End();
		}
		else if (openUpgrade)
		{
			turretVector = cScene2D->getTurretVec();
			if (CSettings::GetInstance()->bDisableMousePointer == true)
				glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			ImGuiWindowFlags inventoryBgWinFlags =
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoBackground;
			ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.10f, (float)cSettings->iWindowHeight * 0.20f));
			ImGui::SetNextWindowSize(ImVec2(700.0f * relativeScale_x, 450.0f * relativeScale_y));
			ImGui::Begin("Upgrade Background", NULL, inventoryBgWinFlags);
			{
				ImGui::Image((ImTextureID)UpgradeBG.textureID,
					ImVec2(690.0f * relativeScale_x,
						450.0f * relativeScale_y),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SetNextWindowSize(ImVec2(700.0f * relativeScale_x, 600.0f * relativeScale_y));
				ImGui::Begin("Upgrade", NULL, inventoryBgWinFlags);
				{
					//Turret Upgrade Icons
					{
						CImageLoader* Image = CImageLoader::GetInstance();
						// Left side Upgrades
						switch (turretVector[cScene2D->GetTurretNo()]->GetNextTurret(true))
						{
						case 1011:
							LeftUpgrade = TurretImg[0].textureID;
							LeftDesc = "Base Class for Physical Damage.";
							LeftTowerCosts = 1;
							break;
						case 2011:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/ReinforcedStoneTurret.png", true);
							LeftDesc = "A Heavy-Duty Turret with improved Damage and Health.";
							break;
						case 2021:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/FlameTurret.png", true);
							LeftDesc = "A Turret that shoots Elemental Flame Bullets. 20% to Burn.";
							break;
						case 3011:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/SharpStoneTurret.png", true);
							LeftDesc = "An improved Turret. This time shooting sharp rocks!";
							break;
						case 3021:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/StoneBurstTurret.png", true);
							LeftDesc = "A Tower that shoots a burst of bullets and goes on cooldown.";
							break;
						case 3031:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/RandomDMGTurretVer2.png", true);
							LeftDesc = "An upgraded Turret that shoots Bullets that deal a random damage from 6 to 12.";
							break;
						case 3041:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/FlameSpearTurret.png", true);
							LeftDesc = "An upgraded Turret with extra range. 20% to Burn.";
							break;
						case 3051:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/IceSpearTurret.png", true);
							LeftDesc = "An upgraded Turret with extra range. 5% to Freeze.";
							break;
						case 3061:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/WindTurret.png", true);
							LeftDesc = "This mysterious turret consumes and fires bullets, but only air exits the barrel.";
							break;
						case 4011:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/SharperStoneTurret.png", true);
							LeftDesc = "The improved an improved Turret. This time shooting the sharper rocks!";
							break;
						case 4021:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/ReinforcedIronTurret.png", true);
							LeftDesc = "A powerful Turret with high raw stats.";
							break;
						case 4031:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/GoldenTurret.png", true);
							LeftDesc = "A Turret that attracts enemies. Useless otherwise.";
							break;
						case 4041:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/IronBurstTurret.png", true);
							LeftDesc = "An upgraded Turret that shoots a burst of bullets and goes on cooldown.";
							break;
						case 4051:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/MultimultishotTurret.png", true);
							LeftDesc = "A Turret that shoots multiple bullets in 8 directions.";
							break;
						case 4061:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/Turret3.png", true);
							LeftDesc = "A Turret that shoots 3 bullets at once.";
							break;
						case 4071:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/RandomDmgTurretVer3.png", true);
							LeftDesc = "An upgraded Turret that shoots Bullets that deal a random damage from 9 to 15.";
							break;
						case 4081:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/UpgradedGlitchTurret.png", true);
							LeftDesc = "A Turret that shoots bullets at random intervals but faster.";
							break;
						case 4091:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/EternalFlameSpearTurret.png", true);
							LeftDesc = "The improved an improved Turret with extra range. 100% to Burn.";
							break;
						case 4111:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/UpgradedFlamethrowerTurret.png", true);
							LeftDesc = "An upgraded Turret that blows Fire this time with extra range. 100% to Burn.";
							break;
						case 4121:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/UpgradedFirewallTurret.png", true);
							LeftDesc = "A Turret that shoots a Firewall 5 blocks long.";
							break;
						case 4131:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/EternalIceSpearTurret.png", true);
							LeftDesc = "An upgraded Turret with extra range. 50% to Freeze.";
							break;
						case 4141:
							LeftUpgrade = Image->LoadTextureGetID("Image/Turret/GiantSnowballTurret.png", true);
							LeftDesc = "A Turret that shoots Giant Snowballs dealing damage. 100% to Freeze.";
							break;
						case 4151:
							LeftDesc = "A Turret that shoots an Ice Floor 7 blocks long.";
							break;
						case 4161:
							LeftDesc = "This mysterious turret consumes and fires bullets, deals damage and pushes enemies in a random direction. Knockback works 70% of the time.";
							break;
						case 4171:
							LeftDesc = "Exerts Thunder at enemies at a shorter cooldown. Dealing 16 damage.";
							break;
						case 4181:
							LeftDesc = "A Turret that shoot bullets. That heales enemies. Idk, how about you find out this time.";
							break;
							// Rare turrets
						case 1013:
							break;
						case 2013:
							break;
						case 2023:
							break;
						case 3013:
							break;
						case 3023:
							break;
						case 3033:
							break;
						case 3043:
							break;
						case 3053:
							break;
						case 3063:
							break;
						case 4013:
							break;
						case 4023:
							break;
						case 4033:
							break;
						case 4043:
							break;
						case 4053:
							break;
						case 4063:
							break;
						case 4073:
							break;
						case 4083:
							break;
						case 4093:
							break;
						case 4113:
							break;
						case 4123:
							break;
						case 4133:
							break;
						case 4143:
							break;
						case 4153:
							break;
						case 4163:
							break;
						case 4173:
							break;
						case 4183:
							break;
						default:
							break;
						}
						// Right side Upgrades
						switch (turretVector[cScene2D->GetTurretNo()]->GetNextTurret(false))
						{
						case 1012:
							RightUpgrade = TurretImg[1].textureID;
							RightDesc = "Base Class for Elemental Damage.";
							RightTowerCosts = 1;
							break;
						case 2012:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/MultiPebbleTurret.png", true);
							RightDesc = "A Turret with fast Attack Speed.";
							break;
						case 2022:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/FrostTurret.png", true);
							RightDesc = "A Turret that shoots Elemental Ice Shards. 5% to Freeze.";
							break;
						case 3012:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/IronTurret.png", true);
							RightDesc = "A Heavy-Duty Turret with high Damage and Health.";
							break;
						case 3022:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/MultiShotTurret.png", true);
							RightDesc = "A Tower that shoots multiple bullets in 4 directions.";
							break;
						case 3032:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/GlitchedTurret.png", true);
							RightDesc = "A Turret that shoots bullets at random intervals.";
							break;
						case 3042:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/FlamethrowerTurret.png", true);
							RightDesc = "A Turret that blows Fire. 100% to Burn.";
							break;
						case 3052:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/SnowballTurret.png", true);
							RightDesc = "A Turret that shoots Snowballs dealing no damage. 50% to Freeze.";
							break;
						case 3062:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/ThunderTurret.png", true);
							RightDesc = "Exerts Thunder at enemies. Dealing 16 damage.";
							break;
						case 4012:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/SniperTurret.png", true);
							RightDesc = "A Turret with extrandory range. But can only shoot straight.";
							break;
						case 4022:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/ShinyIronTurret.png", true);
							RightDesc = "A very VERY Shiny Turret attracts enemies.";
							break;
						case 4032:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/ElementalTurret2.png", true);
							RightDesc = "Base Class for Elemental Damage.";
							break;
						case 4042:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/HotIronTurret.png", true);
							RightDesc = "A Turret that shoots hot bullets. 10% to Burn.";
							break;
						case 4052:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/StarShotTurret.png", true);
							RightDesc = "A Turret that shoots multiple bullets in 5 directions with increased damage.";
							break;
						case 4062:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/ShotgunTurret.png", true);
							RightDesc = "A Turret that shoots 3 bullets at once in a 1x3 line.";
							break;
						case 4072:
							break;
						case 4082:
							break;
						case 4092:
							RightUpgrade = Image->LoadTextureGetID("Image/Turret/BlueFlameSpearTurret.png", true);
							RightDesc = "An upgraded Turret with extra range. 20% to Burn. 10% to Freeze.";
							break;
						case 4112:
							break;
						case 4122:
							break;
						case 4132:
							break;
						case 4142:
							break;
						case 4152:
							break;
						case 4162:
							break;
						case 4172:
							break;
						case 4182:
							break;
						// Rare turrets
						case 1013:
							break;
						case 2013:
							break;
						case 2023:
							break;
						case 3013:
							break;
						case 3023:
							break;
						case 3033:
							break;
						case 3043:
							break;
						case 3053:
							break;
						case 3063:
							break;
						case 4013:
							break;
						case 4023:
							break;
						case 4033:
							break;
						case 4043:
							break;
						case 4053:
							break;
						case 4063:
							break;
						case 4073:
							break;
						case 4083:
							break;
						case 4093:
							break;
						case 4113:
							break;
						case 4123:
							break;
						case 4133:
							break;
						case 4143:
							break;
						case 4153:
							break;
						case 4163:
							break;
						case 4173:
							break;
						case 4183:
							break;
						default:
							break;
						}
						// Turret Costs 
						switch (LeftTowerCosts)
						{
						case 1:
							cInventoryItem = cInventoryManager->GetItem("Stone");
							Material1 = cInventoryItem->GetTextureID();
							Cost1 = "1";
							cInventoryItem = cInventoryManager->GetItem("Iron");
							Material2 = cInventoryItem->GetTextureID();
							Cost2 = "1";
							cInventoryItem = cInventoryManager->GetItem("Bronze");
							Material3 = cInventoryItem->GetTextureID();
							Cost3 = "1";
							break;
						default:
							break;
						}
						switch (RightTowerCosts)
						{
						case 1:
							cInventoryItem = cInventoryManager->GetItem("Stone");
							Material4 = cInventoryItem->GetTextureID();
							Cost4 = "1";
							cInventoryItem = cInventoryManager->GetItem("Iron");
							Material5 = cInventoryItem->GetTextureID();
							Cost5 = "1";
							cInventoryItem = cInventoryManager->GetItem("Silver");
							Material6 = cInventoryItem->GetTextureID();
							Cost6 = "1";
							break;
						default:
							break;
						}
						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.15f, (float)cSettings->iWindowHeight * 0.25f));
						ImGui::BeginChild("Left Upgrade Icons", ImVec2(300.0f * relativeScale_x, 600.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.15f, (float)cSettings->iWindowHeight * 0.36f));
							ImGui::BeginChild("Costs", ImVec2(250.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::Image((ImTextureID)Material1, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%s", Cost1.c_str());

								ImGui::SameLine((float)cSettings->iWindowWidth * 0.1f);
								ImGui::Image((ImTextureID)Material2, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.18f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%s",Cost2.c_str());

								ImGui::SameLine((float)cSettings->iWindowWidth * 0.2f);
								ImGui::Image((ImTextureID)Material3, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.28f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%s",Cost3.c_str());
							}
							ImGui::EndChild();

							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.253f, (float)cSettings->iWindowHeight * 0.52f));
							ImGui::BeginChild("Image", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::Image((ImTextureID)LeftUpgrade, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
							}
							ImGui::EndChild();
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.20f, (float)cSettings->iWindowHeight * 0.67f));
							ImGui::BeginChild("Text", ImVec2(150.0f * relativeScale_x, 70.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
								ImGui::TextWrapped("%s", LeftDesc.c_str());
								ImGui::PopStyleColor();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();

						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.51f, (float)cSettings->iWindowHeight * 0.25f));
						ImGui::BeginChild("Right Upgrade Icons", ImVec2(300.0f * relativeScale_x, 600.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.51f, (float)cSettings->iWindowHeight * 0.36f));
							ImGui::BeginChild("Costs", ImVec2(250.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::Image((ImTextureID)Material4, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.08f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%s", Cost4.c_str());

								ImGui::SameLine((float)cSettings->iWindowWidth * 0.1f);
								ImGui::Image((ImTextureID)Material5, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.18f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%s", Cost5.c_str());

								ImGui::SameLine((float)cSettings->iWindowWidth * 0.2f);
								ImGui::Image((ImTextureID)Material6, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.28f);
								ImGui::TextColored(ImVec4(1, 1, 1, 1), "x%s", Cost6.c_str());
							}
							ImGui::EndChild();

							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.611f, (float)cSettings->iWindowHeight * 0.52f));
							ImGui::BeginChild("Image", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::Image((ImTextureID)RightUpgrade, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), ImVec2(0, 1), ImVec2(1, 0));
							}
							ImGui::EndChild();
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.558f, (float)cSettings->iWindowHeight * 0.67f));
							ImGui::BeginChild("Text", ImVec2(150.0f * relativeScale_x, 70.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
								ImGui::TextWrapped("%s", RightDesc.c_str());
								ImGui::PopStyleColor();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();
					}

					ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.225f, (float)cSettings->iWindowHeight * 0.85f));
					ImGui::BeginChild("Left Upgrade button", ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
					{
						ImGui::Image((ImTextureID)UpgradeButton.textureID,
							ImVec2(100.0f * relativeScale_x,
								30.0f * relativeScale_y),
							ImVec2(0, 1), ImVec2(1, 0));
						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.225f, (float)cSettings->iWindowHeight * 0.85f));
						ImGui::BeginChild("Left upgrade function", ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
							{
								if (ImGui::Button(" ", ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y)))
								{
									switch (LeftTowerCosts)
									{
									case 1:
										if (cInventoryManager->GetItem("Stone")->GetCount() >= 1 && cInventoryManager->GetItem("Iron")->GetCount() >= 1 && cInventoryManager->GetItem("Bronze")->GetCount() >= 1)
										{
											cInventoryManager->GetItem("Stone")->Remove(1);
											cInventoryManager->GetItem("Iron")->Remove(1);
											cInventoryManager->GetItem("Bronze")->Remove(1);
											turretVector[cScene2D->GetTurretNo()]->UpgradeTurret(true);
										}
										break;
									default:
										break;
									}
									openUpgrade = false;
								}
							}
							ImGui::PopStyleColor(3);
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.585f, (float)cSettings->iWindowHeight * 0.85f));
					ImGui::BeginChild("Right Upgrade button", ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
					{
						ImGui::Image((ImTextureID)UpgradeButton.textureID,
							ImVec2(100.0f * relativeScale_x,
								30.0f * relativeScale_y),
							ImVec2(0, 1), ImVec2(1, 0));
						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.585f, (float)cSettings->iWindowHeight * 0.85f));
						ImGui::BeginChild("Right upgrade function", ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
							{
								if (ImGui::Button(" ", ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y)))
								{
									switch (RightTowerCosts)
									{
									case 1:
										if (cInventoryManager->GetItem("Stone")->GetCount() >= 1 && cInventoryManager->GetItem("Iron")->GetCount() >= 1 && cInventoryManager->GetItem("Silver")->GetCount() >= 1)
										{
											cInventoryManager->GetItem("Stone")->Remove(1);
											cInventoryManager->GetItem("Iron")->Remove(1);
											cInventoryManager->GetItem("Silver")->Remove(1);
											turretVector[cScene2D->GetTurretNo()]->UpgradeTurret(false);
										}
										break;
									default:
										break;
									}
									openUpgrade = false;
								}
							}
							ImGui::PopStyleColor(3);
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.87f, (float)cSettings->iWindowHeight * 0.85f));
					ImGui::BeginChild("Destory button", ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
					{
						ImGui::Image((ImTextureID)DestroyButton.textureID,
							ImVec2(35.0f * relativeScale_x,
								35.0f * relativeScale_y),
							ImVec2(0, 1), ImVec2(1, 0));
						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.87f, (float)cSettings->iWindowHeight * 0.85f));
						ImGui::BeginChild("Destroy button function", ImVec2(35.0f * relativeScale_x, 35.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
							{
								if (ImGui::Button(" ", ImVec2(35.0f * relativeScale_x, 35.0f * relativeScale_y)))
								{
									turretVector[cScene2D->GetTurretNo()]->SetGetTurretHP(0);
									openUpgrade = false;
								}
							}
							ImGui::PopStyleColor(3);
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();
				}
				ImGui::End();
			}
			ImGui::End();
		}
		else
		{
			//Disable Cursor
			if (CSettings::GetInstance()->bDisableMousePointer == true)
				glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
	ImGui::PopStyleColor(1);
	ImGui::End();

	// Render the Lives
	ImGuiWindowFlags livesWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Player Lives", NULL, livesWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.8f, cSettings->iWindowHeight * 0.03f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	cInventoryItem = cInventoryManager->GetItem("Lives");
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
				ImVec2(cInventoryItem->vec2Size.x * relativeScale_x, cInventoryItem->vec2Size.y * relativeScale_y),
				ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d", cInventoryItem->GetCount(), cInventoryItem->GetMaxCount());
	ImGui::End();
	ImGui::Begin("Base HP", NULL, livesWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.8f, cSettings->iWindowHeight * 0.09f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	cInventoryItem = cInventoryManager->GetItem("Base HP");
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
				ImVec2(cInventoryItem->vec2Size.x* relativeScale_x, cInventoryItem->vec2Size.y* relativeScale_y),
				ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d", cInventoryItem->GetCount(), cInventoryItem->GetMaxCount());
	ImGui::End();
	// Render Prommpt 
	if (cPlayer2D->GetMaterialRange() == true)
	{
		ImGui::Begin("Prompt", NULL, livesWindowFlags);
		ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.35f, cSettings->iWindowHeight * 0.45f));
		ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
		ImGui::TextColored(ImVec4(1, 1, 1, 1), "Press 'X' to collect materials");
		ImGui::End();
	}

	/*ImGui::Begin("Invisible window", NULL, window_flags);
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);*/

	// Display the FPS
	//ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS: %d", cFPSCounter->GetFrameRate());


	//// Render the inventory items
	//cInventoryItem = cInventoryManager->GetItem("Coin");
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));  // Set a background color
	//ImGuiWindowFlags inventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("Image", NULL, inventoryWindowFlags);
	//ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.03f, cSettings->iWindowHeight * 0.9f));
	//ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x * relativeScale_x, cInventoryItem->vec2Size.y * relativeScale_y),
	//	ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::SameLine();
	//ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Coins: %d", cInventoryItem->GetCount());
	//ImGui::End();
	//ImGui::PopStyleColor();

	//ImGui::End();
}


/**
 @brief Set up the OpenGL display environment before rendering
 */
void CGUI_Scene2D::PreRender(void)
{
}

/**
 @brief Render this instance
 */
void CGUI_Scene2D::Render(void)
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CGUI_Scene2D::PostRender(void)
{
}

int CGUI_Scene2D::GetEquipped()
{
	return itemEquipped;
}

void CGUI_Scene2D::OpenUpgrade()
{
	openUpgrade = !openUpgrade;
	openCrafting = false;
	openInventory = false;
}

bool CGUI_Scene2D::UpgradeState()
{
	return openUpgrade;
}

int CGUI_Scene2D::Checkupgrade()
{
	return Upgrade;
}
