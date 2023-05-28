#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
void processInput(GLFWwindow* window);
void showFPS(GLFWwindow* pWindow);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(vector<std::string> faces);

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

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

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
	glEnable(GL_PROGRAM_POINT_SIZE);

	// build and compile our shader program
	// ------------------------------------
	Shader modelShader(R"(resource\shader\model_loading.vs)", R"(resource\shader\model_loading.fs)");
	Shader pointModelShader(R"(resource\shader\point_model.vs)", R"(resource\shader\point_model.fs)");

	// Shader lightCubeShader(R"(resource\shader\light_cube.vs)", R"(resource\shader\light_cube.fs)");

	// load models
	// -----------
	Model justModel(R"(resource\model\nanosuit\nanosuit.obj)");

	// draw in wireframe
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	vector<std::string> faces
	{
		R"(resource\texture\skybox\right.jpg)",
		R"(resource\texture\skybox\left.jpg)",
		R"(resource\texture\skybox\top.jpg)",
		R"(resource\texture\skybox\bottom.jpg)",
		R"(resource\texture\skybox\front.jpg)",
		R"(resource\texture\skybox\back.jpg)"
	};
	Skybox skybox(faces, R"(resource\shader\skybox.vs)", R"(resource\shader\skybox.fs)");

	unsigned int cubemapTexture = skybox.cubemapTexture();

	// uniform buffer
	unsigned int uboTransformMatrices;
	glGenBuffers(1, &uboTransformMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboTransformMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboTransformMatrices, 0, 3 * sizeof(glm::mat4));

	// shader configuration
	// --------------------
	modelShader.use();
	modelShader.setInt("skybox", 5);


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
		
		// projection view model transformation matrices
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down

		// transformation matrices
		glBindBuffer(GL_UNIFORM_BUFFER, uboTransformMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));

		// don't forget to enable shader before setting uniforms
		modelShader.use();
		modelShader.setVec3("viewPos", camera.Position);
		modelShader.setFloat("material.shininess", 64.0f);
		
		/*
			Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
			the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
			by defining light types as classes and set their values in there, or by using a more efficient uniform approach
			by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
		*/
		// directional light
		modelShader.setDirLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(0.5f, 0.5f, 0.5f));
		// point light 1
		modelShader.setPointLight(0, pointLightPositions[0], glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(cubeR * 0.8f, cubeG * 0.8f, cubeB * 0.8f), glm::vec3(cubeR * 1.0f, cubeG * 1.0f, cubeB * 1.0f), 1.0f, 0.09f, 0.032f);
		// point light 2
		modelShader.setPointLight(1, pointLightPositions[1], glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(cubeR * 0.8f, cubeG * 0.8f, cubeB * 0.8f), glm::vec3(cubeR * 1.0f, cubeG * 1.0f, cubeB * 1.0f), 1.0f, 0.09f, 0.032f);
		// point light 3
		modelShader.setPointLight(2, pointLightPositions[2], glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(cubeR * 0.8f, cubeG * 0.8f, cubeB * 0.8f), glm::vec3(cubeR * 1.0f, cubeG * 1.0f, cubeB * 1.0f), 1.0f, 0.09f, 0.032f);
		// point light 4
		modelShader.setPointLight(3, pointLightPositions[3], glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(cubeR * 0.8f, cubeG * 0.8f, cubeB * 0.8f), glm::vec3(cubeR * 1.0f, cubeG * 1.0f, cubeB * 1.0f), 1.0f, 0.09f, 0.032f);

		// spotLight
		modelShader.setSpotLight(camera.Position, camera.Front, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(2.0f, 2.0f, 2.0f), 1.0f, 0.09f, 0.032f, glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)));
		
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		justModel.Draw(modelShader);


		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		model = glm::mat4(1.0f);		
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 6.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));
		justModel.Draw(pointModelShader);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skybox.draw(projection,view);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteBuffers(1, &uboTransformMatrices);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
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

void showFPS(GLFWwindow* pWindow)
{
	// Measure speed
	float currentTime = static_cast<float>(glfwGetTime());
	float deltaTime = currentTime - lastTime;
	framesNumber++;

	if (deltaTime >= 0.6) { // If last cout was more than 0.6 sec ago
		std::cout << 1000.0 / static_cast<float>(framesNumber) << std::endl;

		float fps = static_cast<float>(framesNumber) / deltaTime;

		std::stringstream ss;
		ss << "HaiBooLang     " << "[ " << fps << " FPS ]";

		glfwSetWindowTitle(pWindow, ss.str().c_str());

		framesNumber = 0;
		lastTime = currentTime;
	}
}
