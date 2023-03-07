#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "stdlib.h"
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lib/shader/shader.hpp"
#include "lib/controller/joystick.hpp"
#include "lib/camera/camera.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb/stb_image.h"

#include <filesystem>

// Variables
uint32_t WINDOW_WIDTH = 800;
uint32_t WINDOW_HEIGHT = 600;

float lastMouseX = WINDOW_WIDTH / 2;
float lastMouseY = WINDOW_HEIGHT / 2;

bool firstMouse = true;

// Array in-case we want more positions
static glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f)
};

uint32_t vboId, vaoId, eboId;

float vertices[] = {
    // Back
    -1.0f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.0f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
     0.0f,  1.5f, -0.5f,  1.0f, 1.0f,

     0.0f, -0.5f, -0.5f,  0.0f, 0.0f,
     1.0f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

    // Front
    -1.0f, -0.5f, 0.0f,  0.0f, 0.0f,
     0.0f, -0.5f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,  1.0f, 1.0f,

    -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 0.0f,
     0.0f,  1.5f, 0.0f,  1.0f, 1.0f,

     0.0f, -0.5f, 0.0f,  0.0f, 0.0f,
     1.0f, -0.5f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
};

int indices[] = {
    // back

    // left, right, top
    0, 1, 2, // left
    3, 4, 5, // top
    6, 7, 8, // right

    // front
    // left, right, top
    9, 10, 11, // left
    12, 13, 14, // top
    15, 16, 17, // right

    // left wall
    5, 14, 0,
    0, 9, 14,
    // right wall
    5, 14, 7,
    7, 16, 14,
    // floor
    0, 9, 7,
    7, 9, 16,
    // inner left wall
    1, 3, 11,
    1, 10, 11,
    // inner right wall
    6, 8, 15,
    8, 15, 17,
    // ceiling wall
    3, 4, 12,
    4, 12, 13
};

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

static Shader shader;
static Joystick joystick;
static Camera camera;

uint32_t TOTAL_VERTICES = 54;

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void render(GLFWwindow* window);
void storeVertexDataOnGpu();
void draw(Camera &camera);

int main() 
{
    std::cout << "Hello, Triforce!" << std::endl;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Application", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}  

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);

    glfwSetCursorPosCallback(window, mouse_callback); 
    glfwSetScrollCallback(window, scroll_callback); 

    // play music (Note, we'll need to use sfml-audio here)

    // render window
	render(window);

	glDeleteVertexArrays(1, &vaoId);
    glDeleteBuffers(1, &vboId);
    shader.deleteProgram();

	glfwTerminate();
    return 0;
}

void render(GLFWwindow* window)
{
	storeVertexDataOnGpu();

    shader = Shader("src/data/shader/vertex.shader", "src/data/shader/fragment.shader");

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    
    camera = Camera(WINDOW_WIDTH, WINDOW_HEIGHT, shader.getID(), cameraPos, cameraPos + cameraFront, cameraUp, 45.0f, 0.0f, -90.f);

	while(!glfwWindowShouldClose(window))
	{
		processInput(window);

		draw(camera);

		glfwSwapBuffers(window);
		glfwPollEvents();    
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void joystick_callback(double xpos, double ypos)
{
    if (xpos > 0.3 || xpos < -0.3 || ypos > 0.3 || ypos < -0.3)
    {
        lastMouseX -= xpos;
        lastMouseY += ypos;
        
        const float sensitivity = 3.0f;
        float xoffset = xpos * sensitivity;
        float yoffset = ypos * sensitivity;

        camera.updateRotationAxes(xoffset, yoffset, true);
        camera.updateCameraFront();
    }
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    bool joystickPresent = glfwJoystickPresent(GLFW_JOYSTICK_1);

    uint32_t fov = joystick.calculateNewFov(joystickPresent, camera.getFov());
    camera.setFov(fov);

    JoystickButtons buttons = joystick.getJoystickButtons();

    glm::vec3 cameraPos = camera.getCameraPos();
    glm::vec3 cameraFront = camera.getCameraFront();
    glm::vec3 cameraUp = camera.getCameraUp();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || buttons.leftY < -0.3)
    {
        const float cameraSpeed = 3.5f * deltaTime; // adjust accordingly
        cameraPos += (cameraSpeed * cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || buttons.leftY > 0.3)
    {
        const float cameraSpeed = 3.5f * deltaTime; // adjust accordingly
        cameraPos -= (cameraSpeed * cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || buttons.leftX < -0.3)
    {
        const float cameraSpeed = 3.5f * deltaTime; // adjust accordingly
        cameraPos -= (glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || buttons.leftX > 0.3)
    {
        const float cameraSpeed = 3.5f * deltaTime; // adjust accordingly
        cameraPos += (glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed);
    }

    camera.setCameraPos(cameraPos);
    joystick_callback(buttons.rightX, buttons.rightY);
}

void draw(Camera &camera)
{
	// Clear the screen with a colour
	glClearColor(0.0f, 0.0f, 0.5f, 0.2f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

	// Every shader and rendering call after the `glUseProgram` call will now use this program object (and thus the shaders).
    shader.use();

    camera.view(vaoId, cubePositions);

    glDrawElements(GL_TRIANGLES, TOTAL_VERTICES, GL_UNSIGNED_INT, 0);
}

void storeVertexDataOnGpu()
{
	glGenVertexArrays(1, &vaoId);
	glGenBuffers(1, &vboId);
	glGenBuffers(1, &eboId);

	glBindVertexArray(vaoId);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    std::cout << xpos << ", " << ypos << std::endl;
    if (firstMouse)
    {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos; // reversed since y-coordinates range from bottom to top
    lastMouseX = xpos;
    lastMouseY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.updateRotationAxes(xoffset, yoffset, false);
    camera.updateCameraFront();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    float fov = camera.getFov() - (float)yoffset;
    camera.setFov(fov);
}