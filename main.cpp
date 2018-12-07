#define GLM_ENABLE_EXPERIMENTAL
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

//#include "common/shader.hpp"
#include "tools/texture.hpp"
#include "tools/objloader.hpp"
//#include "common/controls.hpp"
#include "Particle.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "Terrain.hpp"
#pragma comment(lib, "glfw3.lib")


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);


// window settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera: initiliazed by start location
vec3 sceneCenter(80.f, 0.f, 180.f);
Camera camera(sceneCenter+vec3(0.f, 0.f, 15.f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Fix compilation on OS X
#endif

	// Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Waterfall", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
    
    // Set event listener
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glfwPollEvents();
	// Dark blue background
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

    
    
    //===============================================================================================
    // prepare skybox
    //===============================================================================================
    float sceneScale = 500.f;

    Shader skyboxShader("shader/skybox.vs", "shader/skybox.fs");
    Skybox skybox(sceneScale, sceneCenter, &skyboxShader);
    
    //===============================================================================================
    // prepare particle
    //===============================================================================================
    
    // Create and compile our GLSL program from the shaders
    Shader particleShader( "shader/particle.vs", "shader/particle.fs" );
    Particles waterfall(sceneCenter, &particleShader);
    
    
    //===============================================================================================
    // prepare terrain
    //===============================================================================================
    // Create and compile our GLSL program from the shaders
    Shader terrainShader( "shader/terrain.vs", "shader/terrain.fs" );
    Terrain mountain(&terrainShader);
    
    while( !glfwWindowShouldClose(window))
    {
        
        //===============================================================================================
        // render init
        //===============================================================================================
        
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 ViewMatrix = camera.GetViewMatrix();
        glm::mat4 ProjectionMatrix = glm::perspective(
                                                      glm::radians(camera.Zoom),
                                                      (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                      0.1f,
                                                      100.0f
                                                      );
        glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

        
        
        //===============================================================================================
        // skybox render
        //===============================================================================================
        
        skybox.draw(ViewProjectionMatrix);
        
        //===============================================================================================
        // terrain render
        //===============================================================================================
        
        mountain.draw(MVP);
        
        //===============================================================================================
        // particle render
        //===============================================================================================
        
        int newparticles = (int)(deltaTime*10000.0);
        if (newparticles > (int)(0.016f*10000.0))
            newparticles = (int)(0.016f*10000.0);
        
        waterfall.SpawnParticles(currentFrame, newparticles);
        waterfall.UpdateParticles(deltaTime, CameraPosition);
        

        waterfall.draw(ViewMatrix, ViewProjectionMatrix);
        
        //===============================================================================================
        // render end
        //===============================================================================================
        
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
    
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
    
	return 0;
}



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera.ProcessKeyboard(UPWORD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWNWORD, deltaTime);
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
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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
    camera.ProcessMouseScroll(yoffset);
}


