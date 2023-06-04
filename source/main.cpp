#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nlohmann/json.hpp>

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

#include <Windows.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void showFPS(GLFWwindow* pWindow);

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


GLFWwindow* loadGLFW(std::string path, int* status);

int main()
{
	int status = 0;
	GLFWwindow* window = loadGLFW(R"(global.json)", &status);
	if (status == -1)
	{
		std::cerr << "error\n";
		// return -1;
	}

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
	glEnable(GL_PROGRAM_POINT_SIZE);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 420");

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// build and compile our shader program
	// ------------------------------------
	Shader modelShader(R"(resource\shader\model_lighting.vert)", R"(resource\shader\model_lighting.frag)");
	Shader planeShader(R"(resource\shader\plane.vert)", R"(resource\shader\plane.frag)");

	// load models
	// -----------
	Model nanosuit(R"(resource\model\nanosuit\nanosuit.obj)");
	Model zelda(R"(resource\model\zelda\Zelda.dae)");
	Model nahida(R"(resource\model\nahida\nahida.pmx)");
	Model creeper(R"(resource\model\\creeper\source\creeper.fbx)");

	vector<std::string> faces
	{
		R"(resource\texture\skybox\px.png)",
		R"(resource\texture\skybox\nx.png)",
		R"(resource\texture\skybox\py.png)",
		R"(resource\texture\skybox\ny.png)",
		R"(resource\texture\skybox\pz.png)",
		R"(resource\texture\skybox\nz.png)"
	};
	Skybox skybox(faces, R"(resource\shader\skybox.vert)", R"(resource\shader\skybox.frag)");

	unsigned int cubemapTexture = skybox.cubemapTexture();

	float plane_vertices[] = {
		 2.0f,  0.0f,  -2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, 0.0f,  2.0f,  0.0f, 1.0f, 0.0f,
		 2.0f,  0.0f,  2.0f,  0.0f, 1.0f, 0.0f,

		-2.0f, 0.0f,  -2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, 0.0f,  2.0f,  0.0f, 1.0f, 0.0f,
		2.0f,  0.0f,  -2.0f, 0.0f, 1.0f, 0.0f,

	};

	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

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
	modelShader.setFloat("material.shininess", 64.0f);

	planeShader.use();
	planeShader.setInt("skybox", 10);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// show fps in window title 
		showFPS(window);

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!		

		// set up shader
		modelShader.use();

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		modelShader.setVec3("viewPos", camera.Position);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		// buffer transformation matrices
		glBindBuffer(GL_UNIFORM_BUFFER, uboTransformMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

		// draw nanosuit
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(nanosuit.getScalingY()));	// it's a bit too big for our scene, so scale it down
		modelShader.setMat4("model", model);

		nanosuit.Draw(modelShader);

		// draw zelda
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(zelda.getScalingY()));	// it's a bit too big for our scene, so scale it down
		modelShader.setMat4("model", model);

		zelda.Draw(modelShader);

		// draw nahida
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(nahida.getScalingY()));	// it's a bit too big for our scene, so scale it down
		modelShader.setMat4("model", model);

		nahida.Draw(modelShader);

		// draw creeper
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(creeper.getScalingZ()));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-2.0f, 0.0f, 0.0f));
		modelShader.setMat4("model", model);

		creeper.Draw(modelShader);

		// draw skybox
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix

		skybox.draw(projection, view);

		planeShader.use();
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		planeShader.setVec3("viewPos", camera.Position);
		model = glm::mat4(1.0f);
		planeShader.setMat4("model", model);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteBuffers(1, &uboTransformMatrices);

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}


inline void showFPS(GLFWwindow* pWindow)
{
	// Measure speed
	float currentTime = static_cast<float>(glfwGetTime());
	float deltaTime = currentTime - lastTime;
	framesNumber++;

	if (deltaTime >= 0.6) { // If last cout was more than 0.6 sec ago
		// std::cout << 1000.0 / static_cast<float>(framesNumber) << std::endl;

		float fps = static_cast<float>(framesNumber) / deltaTime;

		std::stringstream sstream;
		sstream << "HaiBooLang     " << "[ " << fps << " FPS ]";

		glfwSetWindowTitle(pWindow, sstream.str().c_str());

		framesNumber = 0;
		lastTime = currentTime;
	}
}

inline GLFWwindow* loadGLFW(std::string path, int* status)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		std::cerr << "open file error��\n";
		*status = -1;
		return nullptr;
	}

	nlohmann::json data = nlohmann::json::parse(file);
	if (data.is_null())
	{
		std::cerr << "parse error��\n";
		*status = -1;
		return nullptr;
	}

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, data["version"]["major"]);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, data["version"]["minor"]);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (data["transparent_framebuffer"] == true)
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	if (data["multiple_sample"] == true)
		glfwWindowHint(GLFW_SAMPLES, 4);

	// glfw window creation
	// --------------------  
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HaiBooLang", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		*status = -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(0);
	// glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return window;
}