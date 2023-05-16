// https://ecode.dev/transparent-framebuffer-borderless-window-using-glfw/
#include <windows.h>
#include <GLFW/glfw3.h>

// change this to int WINAPI WinMain() to avoid the console
int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, char*, int nShowCmd)
{
<<<<<<< Updated upstream
    GLFWwindow* window;
    int windowSizeW = 640, windowSizeH = 480;
    // initialize the library
    if (!glfwInit())
        return -1;
=======
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
>>>>>>> Stashed changes

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    int count, windowWidth, windowHeight, monitorX, monitorY;

    // set the position of the window
    // I am assuming that main monitor is in the 0 position
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);
    // width: 75% of the screen
    windowWidth = static_cast<int>(videoMode->width / 1.5);
    // aspect ratio 16 to 9
    windowHeight = static_cast<int>(videoMode->height / 16 * 9);

    glfwGetMonitorPos(monitors[0], &monitorX, &monitorY);

    // set the visibility window hint to false for subsequent window creation
    // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(windowSizeW, windowSizeH, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // make the window's context current
    glfwMakeContextCurrent(window);
<<<<<<< Updated upstream
=======
    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
>>>>>>> Stashed changes

    // reset the window hints to default
    glfwDefaultWindowHints();

    glfwSetWindowPos(window,
        monitorX + (videoMode->width - windowWidth) / 2,
        monitorY + (videoMode->height - windowHeight) / 2);

    // show the window
    glfwShowWindow(window);

    // uncomment following line to see the border of window
    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

    float fAngle = 0.0f;
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // render
<<<<<<< Updated upstream
=======
        // ------
>>>>>>> Stashed changes
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glRotatef(fAngle, 0, 0, 1);
        glBegin(GL_QUADS);
        glColor3f(0, 0, 1);
        glVertex3f(-0.5, -0.5, -1);
        glColor3f(0, 1, 0);
        glVertex3f(0.5, -0.5, -1);
        glColor3f(1, 0, 1);
        glVertex3f(0.5, 0.5, -1);
        glColor3f(1, 1, 0);
        glVertex3f(-0.5, 0.5, -1);
        glEnd();

        fAngle += 0.01;  // at some point this will explote! overflow: put the corresponding of

        // swap front and back buffers
        glfwSwapBuffers(window);

        // poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}