#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>
#include <model.h>
#include <camera.h>
#include <skybox.h>
#include <console.h>
#include <overlay.h>

#include <Windows.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
// void ShowMonitor(bool* p_open, Camera& camera, ImGuiIO& io, std::ostringstream& buffer);

// settings
constexpr unsigned int SCR_WIDTH = 1600;
constexpr unsigned int SCR_HEIGHT = 1600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// fps
int framesNumber = 0;
float lastTime = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// cube color
constexpr float cubeR = 1.0f;
constexpr float cubeG = 1.0f;
constexpr float cubeB = 1.0f;

bool mouse = true;

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	std::ostringstream buffer;

	std::streambuf* coutbuf = std::cout.rdbuf();
	std::cout.rdbuf(buffer.rdbuf());

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// glfw window creation
	// --------------------  
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HaiBooLang", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(0);
	// glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

	glfwSetCursorPos(window, SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// build and compile our shader program
	// ------------------------------------
	Shader modelShader(R"(resource\shader\model_lighting.vert)", R"(resource\shader\model_lighting.frag)");

	// load models
	// -----------
	Model nanosuit(R"(resource\model\nanosuit\nanosuit.obj)");
	Model zelda(R"(resource\model\zelda\Zelda.dae)");

	vector<std::string> faces
	{
		R"(resource\texture\skybox\px.png)",
		R"(resource\texture\skybox\nx.png)",
		R"(resource\texture\skybox\py.png)",
		R"(resource\texture\skybox\ny.png)",
		R"(resource\texture\skybox\pz.png)",
		R"(resource\texture\skybox\nz.png)"
	};
	// Skybox skybox(R"(resource\texture\skybox\StandardCubeMap.png)", R"(resource\shader\skybox.vert)", R"(resource\shader\skybox.frag)");
	Skybox skybox(faces, R"(resource\shader\skybox.vert)", R"(resource\shader\skybox.frag)");

	unsigned int cubemapTexture = skybox.cubemapTexture();

	// uniform buffer
	unsigned int uboTransformMatrices;
	glGenBuffers(1, &uboTransformMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboTransformMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboTransformMatrices, 0, 2 * sizeof(glm::mat4));

	// shader configuration
	// --------------------
	modelShader.use();
	modelShader.setInt("skybox", 10);
	modelShader.setVec3("viewPos", camera.Position);
	modelShader.setFloat("material.shininess", 64.0f);

	// IMGUI
	// --------------------------------------------------------------------------------------------
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableSetMousePos;
	// io.MouseDrawCursor = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	// Our state
	// bool show_demo_window = true;
	// bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Set the font size
	io.FontGlobalScale = 2.0f;

	// Dock the window to the left edge of the screen
	// ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	// ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_Always);

	bool show_app_console = false;
	bool show_app_overlay = false;
	
	// Initialize AppConsole
	AppConsole console;

	// Redirect std::cout to AppConsole
	AppConsoleStreamBuffer streamBuffer(&console);
	std::streambuf* oldBuffer = std::cout.rdbuf(&streamBuffer);

	int n = 0;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// IMGUI
		// --------------------------------------------------------------------------------------------
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		std::cout << n++ <<std::endl;

		if (show_app_console)
			console.Draw("console", &show_app_console);
		if (show_app_overlay)
			ShowAppOverlay(&show_app_overlay);

		// ImGui Rendering
		ImGui::Render();

		// input
		// -----
		if (mouse)
		{
			processInput(window);

			if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				glfwSetScrollCallback(window, NULL);
				glfwSetCursorPosCallback(window, NULL);
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
				ImVec2 mouse_pos = ImGui::GetMousePos();
				bool is_valid = !isnan(mouse_pos.x) && !isnan(mouse_pos.y);
				if (is_valid)
					glfwSetCursorPos(window, SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);
				show_app_console = true;
				mouse = false;
			}
		}
		else
		{
			if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetScrollCallback(window, scroll_callback);
				glfwSetCursorPosCallback(window, mouse_callback);
				io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
				io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableSetMousePos;
				ImVec2 mouse_pos = ImGui::GetMousePos();
				bool is_valid = !isnan(mouse_pos.x) && !isnan(mouse_pos.y);
				if (is_valid)
					glfwSetCursorPos(window, SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);
				show_app_console = false;
				mouse = true;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		{
			show_app_overlay = !show_app_overlay;
		}

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

		// projection view model transformation matrices
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down

		// buffer transformation matrices
		glBindBuffer(GL_UNIFORM_BUFFER, uboTransformMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

		// don't forget to enable shader before setting uniforms
		modelShader.use();

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		modelShader.setMat4("model", model);
		modelShader.setVec3("viewPos", camera.Position);
		nanosuit.Draw(modelShader);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 6.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelShader.setMat4("model", model);
		zelda.Draw(modelShader);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skybox.draw(projection, view);

		// Render the imgui UI on top of the OpenGL scene
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		// glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteBuffers(1, &uboTransformMatrices);

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// std::cout.rdbuf(oldBuffer);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
inline void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1)
	{
		mouse += 1;
	}
}



//void ShowMonitor(bool* p_open, Camera& camera, ImGuiIO& io, std::ostringstream& buffer)
//{
//	const float DISTANCE = 10.0f;
//	static int corner = 0;
//	ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
//	ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
//
//	if (corner != -1)
//		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
//
//	ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
//
//	if (ImGui::Begin("Hello", p_open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
//	{
//		ImGui::Text("FPS: %.1f", io.Framerate);
//		ImGui::Separator();
//
//		ImGui::Text("Angle : %.1f", camera.Zoom);
//		ImGui::Separator();
//
//		ImGui::Text("Camera Position: (%.1f,%.1f,%.1f)", camera.Position.x, camera.Position.y, camera.Position.z);
//		ImGui::Separator();
//
//
//		ImGui::Text("%s", buffer.str().c_str());
//		ImGui::Separator();
//
//
//
//		if (ImGui::IsMousePosValid())
//		{
//			ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
//		}
//		else
//		{
//			ImGui::Text("Mouse Position: <invalid>");
//		}
//
//
//		if (ImGui::CollapsingHeader("Inputs & Focus"))
//		{
//			// Display inputs submitted to ImGuiIO
//			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//			if (ImGui::TreeNode("Inputs"))
//			{
//				if (ImGui::IsMousePosValid())
//					ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
//				else
//					ImGui::Text("Mouse pos: <INVALID>");
//				ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
//				ImGui::Text("Mouse down:");
//				for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseDown(i)) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
//				ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);
//
//				// We iterate both legacy native range and named ImGuiKey ranges, which is a little odd but this allows displaying the data for old/new backends.
//				// User code should never have to go through such hoops! You can generally iterate between ImGuiKey_NamedKey_BEGIN and ImGuiKey_NamedKey_END.
//#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
//				struct funcs { static bool IsLegacyNativeDupe(ImGuiKey) { return false; } };
//				ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;
//#else
//				struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; } }; // Hide Native<>ImGuiKey duplicates when both exists in the array
//				ImGuiKey start_key = (ImGuiKey)0;
//#endif
//				ImGui::Text("Keys down:");         for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) continue; ImGui::SameLine(); ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key); }
//				ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
//				ImGui::Text("Chars queue:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
//
//				ImGui::TreePop();
//			}
//			}
//		}
//	}
//	ImGui::End();
//}
