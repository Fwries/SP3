/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_Scene2D.h"

#include "../GameStateManagement/GameStateManager.h"

// Include Image loader
#include "System/ImageLoader.h"

#include "Turret.h"

#include <iostream>
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
	cInventoryItem = cInventoryManager->Add("Coin", "Image/Tiles/tile086.png", 999, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	recipeNo = 0;
	openCrafting = false;
	openInventory = false;
<<<<<<< Updated upstream
=======
	openUpgrade = false;
	Upgrade = 0;
>>>>>>> Stashed changes

	// Variables for buildings
	itemEquipped = 0;
	iTextureID = 0;

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
		if (cKeyboardController->IsKeyPressed(GLFW_KEY_J))
		{
			openUpgrade = !openUpgrade;
			openCrafting = false;
			openInventory = false;
		}
		// Check items in inv
		if (cInventoryManager->GetItem("Turret")->GetCount() == 0)
		{
			itemEquipped = 0;
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
				iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/Inventory.png", true);
				ImGui::Image((void*)(intptr_t)iTextureID,
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
							}
							ImGui::EndChild();
						}
						// Coal UI
						{
							cInventoryItem = cInventoryManager->GetItem("Coal");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.40f));

							ImGui::BeginChild("Coal Child ", ImVec2(110.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Coal Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
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
							}
							ImGui::EndChild();
						}
						// Gold UI
						{
							cInventoryItem = cInventoryManager->GetItem("Gold");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.65f, (float)cSettings->iWindowHeight * 0.50f));

							ImGui::BeginChild("Gold Child ", ImVec2(110.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Gold Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
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
							}
							ImGui::EndChild();
						}
						// Turret UI
						{
							cInventoryItem = cInventoryManager->GetItem("Turret");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.50f));

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
									iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/Equipped.png", true);
								}
								else
								{
									iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/Equip.png", true);
								}
								ImGui::Image((void*)(intptr_t)iTextureID,
									ImVec2(100.0f * relativeScale_x,
										30.0f * relativeScale_y),
									ImVec2(0, 1), ImVec2(1, 0));
								ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.61f));
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
											itemEquipped = 1;
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
				ImGui::PopStyleColor();
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

				iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/Crafting.png", true);
				ImGui::Image((void*)(intptr_t)iTextureID,
					ImVec2(500.0f * relativeScale_x,
						300.0f * relativeScale_y),
					ImVec2(0, 1), ImVec2(1, 0));

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
									ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
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
									ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
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

							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.34f, (float)cSettings->iWindowHeight * 0.48f));
							ImGui::BeginChild("Plus Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
							iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/Plus.png", true);
							ImGui::Image((void*)(intptr_t)iTextureID,
								ImVec2(60.0f * relativeScale_x,
									60.0f * relativeScale_y),
								ImVec2(0, 1), ImVec2(1, 0)); 
							}
							ImGui::EndChild();

							cInventoryItem = cInventoryManager->GetItem("Stone");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.32f, (float)cSettings->iWindowHeight * 0.58f));
							ImGui::BeginChild("Stone Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Stone Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
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
							cInventoryItem = cInventoryManager->GetItem("Plank");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.50f, (float)cSettings->iWindowHeight * 0.32f));

							ImGui::BeginChild("Bomb Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Bomb Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							cInventoryItem = cInventoryManager->GetItem("Plank");
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.27f, (float)cSettings->iWindowHeight * 0.32f));

							ImGui::BeginChild("Tree Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::BeginChild("Tree Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
								{
									ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
										ImVec2(60.0f * relativeScale_x,
											60.0f * relativeScale_y),
										ImVec2(0, 1), ImVec2(1, 0));
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}

						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.47f, (float)cSettings->iWindowHeight * 0.67f));
						ImGui::BeginChild("Craft button", ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{
							iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/Craft.png", true);
							ImGui::Image((void*)(intptr_t)iTextureID,
								ImVec2(90.0f * relativeScale_x,
									30.0f * relativeScale_y),
								ImVec2(0, 1), ImVec2(1, 0));
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.47f, (float)cSettings->iWindowHeight * 0.67f));
							ImGui::BeginChild("Craft button function", ImVec2(90.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
								{
									if (ImGui::Button("  ", ImVec2(90.0f*relativeScale_x, 30.0f*relativeScale_y)))
									{
										switch (recipeNo)
										{
										case 0:
											if (cInventoryManager->GetItem("Plank")->GetCount() >= 3 && cInventoryManager->GetItem("Plank")->GetCount() >= 3)
											{
												ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));
												cInventoryManager->GetItem("Plank")->Remove(3);
												cInventoryManager->GetItem("Stone")->Remove(3);
												cInventoryManager->GetItem("Turret")->Add(1);
												ImGui::PopStyleColor();
											}
											break;
										case 1:
											break;
										case 2:
											break;
										case 3:
											break;
										default:
											break;
										}
									}
								}
								ImGui::PopStyleColor(2); 
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();

						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.25f, (float)cSettings->iWindowHeight * 0.67f));
						ImGui::BeginChild("Left button", ImVec2(30.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
						{
							iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/LeftButton.png", true);
							ImGui::Image((void*)(intptr_t)iTextureID,
								ImVec2(30.0f * relativeScale_x,
									30.0f * relativeScale_y),
								ImVec2(0, 1), ImVec2(1, 0));
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
											recipeNo = 4;
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
							iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/RightButton.png", true);
							ImGui::Image((void*)(intptr_t)iTextureID,
								ImVec2(30.0f * relativeScale_x,
									30.0f * relativeScale_y),
								ImVec2(0, 1), ImVec2(1, 0));
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
<<<<<<< Updated upstream
=======
		else if (openUpgrade)
		{
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
				iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/UpgradeMenu.png", true);
				ImGui::Image((void*)(intptr_t)iTextureID,
					ImVec2(690.0f * relativeScale_x,
						450.0f * relativeScale_y),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SetNextWindowSize(ImVec2(700.0f * relativeScale_x, 600.0f * relativeScale_y));
				ImGui::Begin("Upgrade", NULL, inventoryBgWinFlags);
				{
					ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.225f, (float)cSettings->iWindowHeight * 0.85f));
					ImGui::BeginChild("Left Upgrade button", ImVec2(100.0f * relativeScale_x, 30.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
					{
						iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/Upgrade.png", true);
						ImGui::Image((void*)(intptr_t)iTextureID,
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
									Upgrade = 1;
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
						iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/Upgrade.png", true);
						ImGui::Image((void*)(intptr_t)iTextureID,
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
									Upgrade = 2;
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
						iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/GUI/Destroy.png", true);
						ImGui::Image((void*)(intptr_t)iTextureID,
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
>>>>>>> Stashed changes
		else
		{
			//Disable Cursor
			if (CSettings::GetInstance()->bDisableMousePointer == true)
				glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
	ImGui::PopStyleColor();
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

int CGUI_Scene2D::GetEquipped(void)
{
	return itemEquipped;
}

void CGUI_Scene2D::OpenUpgrade(void)
{
	openUpgrade = !openUpgrade;
}

int CGUI_Scene2D::Checkupgrade(void)
{
	return Upgrade;
}
