/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_Scene2D.h"

#include "../GameStateManagement/GameStateManager.h"

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
	cInventoryItem = cInventoryManager->Add("Turret", "Image/Tiles/tile038.png", 999, 0);
	cInventoryItem = cInventoryManager->Add("Coin", "Image/Tiles/tile086.png", 999, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	recipeNo = 0;
	openCrafting = false;
	openInventory = false;

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
		}

		if (cKeyboardController->IsKeyPressed(GLFW_KEY_F))
		{
			openCrafting = !openCrafting;
			openInventory = false;
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
				ImGuiWindowFlags_NoTitleBar;
			ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.6f, (float)cSettings->iWindowHeight * 0.1f));
			ImGui::SetNextWindowSize(ImVec2(300.0f * relativeScale_x, 500.0f * relativeScale_y));
			ImGui::Begin("Inventory Background", NULL, inventoryBgWinFlags);
			{
				ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.6f, (float)cSettings->iWindowHeight * 0.1f));
				ImGui::SetNextWindowSize(ImVec2(300.0f * relativeScale_x, 25.0f * relativeScale_y));
				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				{
					ImGui::Begin("Inventory Title", NULL, inventoryButtonWinFlags);
					{
						ImGui::SameLine((float)cSettings->iWindowWidth * 0.15f);
						ImGui::Text("Inventory");
					}
					ImGui::End();

					ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
					{
						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.6125f, (float)cSettings->iWindowHeight * 0.155f));
						ImGui::SetNextWindowSize(ImVec2(280.0f * relativeScale_x, 455.0f * relativeScale_y));
						ImGui::Begin("Inventory", NULL, inventoryBgWinFlags);
						{
							ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0, 0.8f, 0.87f, 1.0f));
							{
								// Plank UI
								{
									cInventoryItem = cInventoryManager->GetItem("Plank");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.62f, (float)cSettings->iWindowHeight * 0.17f));

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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x%d",
											cInventoryItem->GetCount());
									}
									ImGui::EndChild();
								}
								// Stone UI
								{
									cInventoryItem = cInventoryManager->GetItem("Stone");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.17f));

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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x%d",
											cInventoryItem->GetCount());
									}
									ImGui::EndChild();
								}
								// Iron UI
								{
									cInventoryItem = cInventoryManager->GetItem("Iron");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.62f, (float)cSettings->iWindowHeight * 0.27f));

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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x%d",
											cInventoryItem->GetCount());
									}
									ImGui::EndChild();
								}
								// Silver UI
								{
									cInventoryItem = cInventoryManager->GetItem("Silver");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.27f));

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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x%d",
											cInventoryItem->GetCount());
									}
									ImGui::EndChild();
								}
								// Bronze UI
								{
									cInventoryItem = cInventoryManager->GetItem("Bronze");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.62f, (float)cSettings->iWindowHeight * 0.37f));

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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x%d",
											cInventoryItem->GetCount());
									}
									ImGui::EndChild();
								}
								// Coal UI
								{
									cInventoryItem = cInventoryManager->GetItem("Coal");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.37f));

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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x%d",
											cInventoryItem->GetCount());
									}
									ImGui::EndChild();
								}
								// Gold UI
								{
									cInventoryItem = cInventoryManager->GetItem("Gold");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.62f, (float)cSettings->iWindowHeight * 0.47f));

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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x%d",
											cInventoryItem->GetCount());
									}
									ImGui::EndChild();
								}
								// Turret UI
								{
									cInventoryItem = cInventoryManager->GetItem("Turret");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.82f, (float)cSettings->iWindowHeight * 0.47f));

									ImGui::BeginChild("Turret Child ", ImVec2(110.0f * relativeScale_x, 85.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x%d",
											cInventoryItem->GetCount());
										ImGui::BeginChild("Turret Function ", ImVec2(60.0f * relativeScale_x, 20.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
										{
											ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
											{
												ImGui::SetWindowFontScale(0.5 * relativeScale_y);
												if (ImGui::Button(((itemEquipped==1) ? ("Equipped") : ("Equip")), ImVec2(60.0f, 20.0f)))
												{
													itemEquipped = 1;
												}
											}
											ImGui::PopStyleColor();
										}
										ImGui::EndChild();
									}
									ImGui::EndChild();
								}
							}
							ImGui::PopStyleColor();
						}
						ImGui::End();
					}
					ImGui::PopStyleColor();
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
				ImGuiWindowFlags_NoTitleBar;
			ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.25f, (float)cSettings->iWindowHeight * 0.25f));
			ImGui::SetNextWindowSize(ImVec2(400.0f * relativeScale_x, 300.0f * relativeScale_y));
			ImGui::Begin("Crafting Background", NULL, inventoryBgWinFlags);
			{
				ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.25f, (float)cSettings->iWindowHeight * 0.25f));
				ImGui::SetNextWindowSize(ImVec2(400.0f * relativeScale_x, 25.0f * relativeScale_y));
				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				{
					ImGui::Begin("Carfting Title", NULL, inventoryButtonWinFlags);
					{
						ImGui::SameLine((float)cSettings->iWindowWidth * 0.2f);
						ImGui::Text("Crafting");
					}
					ImGui::End();;

					ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
					{
						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.2625f, (float)cSettings->iWindowHeight * 0.305f));
						ImGui::SetNextWindowSize(ImVec2(380.0f * relativeScale_x, 255.0f * relativeScale_y));
						ImGui::Begin("Recipe", NULL, inventoryBgWinFlags);
						{
							if (recipeNo == 0)
							{
								ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0, 0.8f, 0.87f, 1.0f));
								{
									cInventoryItem = cInventoryManager->GetItem("Turret");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.27f, (float)cSettings->iWindowHeight * 0.42f));
									ImGui::BeginChild("Turret Child ", ImVec2(90.0f * relativeScale_x, 65.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
									{
										ImGui::BeginChild("Turret Image ", ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
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
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.50f, (float)cSettings->iWindowHeight * 0.32f));
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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x3");
									}
									ImGui::EndChild(); 

									cInventoryItem = cInventoryManager->GetItem("Stone");
									ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.50f, (float)cSettings->iWindowHeight * 0.52f));
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
										ImGui::TextColored(ImVec4(0, 0, 0, 1), "x3");
									}
									ImGui::EndChild();
								}
								ImGui::PopStyleColor();
							}
							else if (recipeNo == 1)
							{
								ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0, 0.8f, 0.87f, 1.0f));
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
								ImGui::PopStyleColor();
							}

							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.47f, (float)cSettings->iWindowHeight * 0.67f));
							ImGui::BeginChild("Craft button", ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
								{
									ImGui::SetWindowFontScale(1.5 * relativeScale_y);
									if (ImGui::Button("Craft", ImVec2(65.0f, 30.0f)))
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
								ImGui::PopStyleColor();
							}
							ImGui::EndChild();

							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.25f, (float)cSettings->iWindowHeight * 0.67f));
							ImGui::BeginChild("Left button", ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
								{
									if (ImGui::Button("<", ImVec2(60.0f, 30.0f)))
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
								ImGui::PopStyleColor();
							}
							ImGui::EndChild();


							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.65f, (float)cSettings->iWindowHeight * 0.67f));
							ImGui::BeginChild("Right button", ImVec2(100.0f * relativeScale_x, 100.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
								{
									if (ImGui::Button(">", ImVec2(60.0f, 30.0f)))
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
								ImGui::PopStyleColor();
							}
							ImGui::EndChild();
						}
						ImGui::End();
					}
					ImGui::PopStyleColor();
				}
				ImGui::PopStyleColor();
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
	ImGui::Begin("Lives", NULL, livesWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.85f, cSettings->iWindowHeight * 0.03f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	cInventoryItem = cInventoryManager->GetItem("Lives");
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x, cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
		cInventoryItem->GetCount(), cInventoryItem->GetMaxCount());
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
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);

	// Display the FPS
	ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS: %d", cFPSCounter->GetFrameRate());


	// Render the inventory items
	cInventoryItem = cInventoryManager->GetItem("Coin");
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));  // Set a background color
	ImGuiWindowFlags inventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Image", NULL, inventoryWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.03f, cSettings->iWindowHeight * 0.9f));
	ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x, cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Coins: %d", cInventoryItem->GetCount());
	ImGui::End();
	ImGui::PopStyleColor();

	ImGui::End();*/
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
