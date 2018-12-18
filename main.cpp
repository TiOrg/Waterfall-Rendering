#define GLM_ENABLE_EXPERIMENTAL
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include "tools/texture.hpp"
//#include "tools/objloader.hpp"

#include "Particle.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "Model.hpp"
#include "Water.hpp"
//#include "SimpleModel.hpp"
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

glm::vec3 upCenter(9.42f, 11.f, -8.22f);
Camera upCamera(upCenter, glm::vec3(-0.35f, 0.88f, 0.34f), -224.8, -26.9);


glm::vec3 sceneCenter(0.f, 0.f, 0.f);
Camera *camera = &upCamera;
Model *p_ship;

Camera downCamera(sceneCenter+glm::vec3(0.f, 3.f, 15.f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ship noise
float rotate_offset = 0.f;
int rotate_dir = -1;
float max_left = 5;

float height_offset = 0.f;
int height_dir = -1;
float max_down = 0.2f;

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
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    

    //======================
    // prepare skybox
    //======================

    float skyboxScale = 500.f;
    Shader skyboxShader("shader/skybox.vs", "shader/skybox.fs");
    Skybox skybox(skyboxScale, &skyboxShader);

    //======================
    // prepare particle
    //======================

    Shader particleShader( "shader/particle.vs", "shader/particle.fs" );
    glm::vec3 particleCenter(2.f, 3.f, 10.f);
    Particles waterfall(particleCenter, &particleShader);


    //======================
    // prepare mountain
    //======================

    Shader modelShader( "shader/model.vs", "shader/model.fs" );

//    glm::vec3 mountainCenter(1.42f, 0.1f, 0.f);
//    glm::vec3 offset = sceneCenter - mountainCenter;


    Model mountain("material/mountain/plane-7.obj", &modelShader);
    mountain.ModelMatrix = glm::scale(mountain.ModelMatrix, glm::vec3(1/70.f, 1/50.f, 1/70.f));
    mountain.ModelMatrix = glm::translate(mountain.ModelMatrix, glm::vec3(0,0,0.f));

    //======================
    // prepare ship
    //======================

    Model ship("material/ship/ShipMoscow.obj", &modelShader);
    p_ship = &ship;
    ship.ModelMatrix = glm::translate(ship.ModelMatrix, glm::vec3(0, 0, 8.f));

    ship.ModelMatrix = glm::rotate(ship.ModelMatrix, glm::radians(-90.f), glm::vec3(1,0,0));
    ship.ModelMatrix = glm::rotate(ship.ModelMatrix, glm::radians(-40.f), glm::vec3(0,0,1));
    ship.ModelMatrix = glm::scale(ship.ModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
    ship.ModelMatrix = glm::translate(ship.ModelMatrix, glm::vec3(50.f, 0.f, 17.4f));

    
    //======================
    // prepare water
    //======================

    Shader waterShader("shader/water.vs", "shader/water.fs");
    Water water(&waterShader);
    water.ModelMatrix = glm::scale(water.ModelMatrix, glm::vec3(2.f, 2.f, 2.f));
    water.ModelMatrix = glm::rotate(water.ModelMatrix, glm::radians(90.f), glm::vec3(0,1,0));
    water.ModelMatrix = glm::translate(water.ModelMatrix, glm::vec3(-10.f, 0.8f, -6.5f));

    waterShader.use();
    glm::vec3 lightPos(0.0f, -20.0f, 0.0f);
    GLfloat materAmbient[] = { 0.1, 0.1, 0.3, 1.0 };
    GLfloat materSpecular[] = { 0.8, 0.8, 0.9, 1.0 };
    GLfloat lightDiffuse[] = { 0.7, 0.7, 0.8, 1.0 };
    GLfloat lightAmbient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat lightSpecular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat envirAmbient[] = { 0.1, 0.1, 0.3, 1.0 };
    
    waterShader.setVec3("lightPos", lightPos);
    waterShader.setVec3("viewPos", camera->Position);
    glUniform4fv(glGetUniformLocation(waterShader.ID, "materAmbient"), 1, materAmbient);
    glUniform4fv(glGetUniformLocation(waterShader.ID, "materSpecular"), 1, materSpecular);
    glUniform4fv(glGetUniformLocation(waterShader.ID, "lightDiffuse"), 1, lightDiffuse);
    glUniform4fv(glGetUniformLocation(waterShader.ID, "lightAmbient"), 1, lightAmbient);
    glUniform4fv(glGetUniformLocation(waterShader.ID, "lightSpecular"), 1, lightSpecular);
    glUniform4fv(glGetUniformLocation(waterShader.ID, "envirAmbient"), 1, envirAmbient);
    
    
//    if(fork() == 0)
//    {
//        while(1)
//        {
//            system("afplay material/waterfall.mp3 -v 5");
//        }
//        exit(0);
//    }
    

    while( !glfwWindowShouldClose(window))
    {
        
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set the frame
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // MVP matrix
        glm::mat4 ViewMatrix = camera->GetViewMatrix();
        glm::mat4 ProjectionMatrix = glm::perspective(
              glm::radians(camera->Zoom),
              (float)SCR_WIDTH / (float)SCR_HEIGHT,
              0.1f,
              100.0f
              );
        glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
        glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

        
        
        //======================
        // water render
        //======================
        
        water.UpdateWave(currentFrame);
        water.draw(ViewMatrix, ProjectionMatrix, currentFrame);

        //======================
        // skybox render
        //======================

        skybox.draw(ViewProjectionMatrix);
        
        //======================
        // model render
        //======================
        
        mountain.draw(ViewProjectionMatrix);
        ship.draw(ViewProjectionMatrix);
        


        //======================
        // particle render
        //======================

        int newparticles = (int)(deltaTime*10000.0);
        if (newparticles > (int)(0.016f*10000.0))
            newparticles = (int)(0.016f*10000.0);

        waterfall.SpawnParticles(currentFrame, newparticles);
        waterfall.UpdateParticles(deltaTime, CameraPosition);


        waterfall.draw(ViewMatrix, ViewProjectionMatrix);

        //======================
        // render end
        //======================
        
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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if( camera == &downCamera )
        {
//            camera = &upCamera;
        }
        else
        {
            camera = &downCamera;
        }
    }
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT, deltaTime);
    
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera->ProcessKeyboard(UPWORD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        camera->ProcessKeyboard(DOWNWORD, deltaTime);
    
    
    glm::vec3 ship_front(1.f, 0, 0);
    const float translate_v = 5.f;
    const float turn_v = 0.2f;
    const float rotate_v = 0.1f;
    const float float_v = 0.005f;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        p_ship->ModelMatrix = glm::translate(p_ship->ModelMatrix, deltaTime * (-translate_v) * ship_front);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        p_ship->ModelMatrix = glm::translate(p_ship->ModelMatrix, deltaTime * translate_v * ship_front);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        if(rotate_offset!=0)
        {
            p_ship->ModelMatrix = glm::rotate(p_ship->ModelMatrix, glm::radians(-rotate_offset), glm::vec3(1,0,0));
            rotate_offset = 0;
        }
        p_ship->ModelMatrix = glm::translate(p_ship->ModelMatrix, deltaTime * (-translate_v) * ship_front * 0.4f);
        p_ship->ModelMatrix = glm::rotate(p_ship->ModelMatrix, turn_v * deltaTime, glm::vec3(0, 0, 1));
    }
    else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        if(rotate_offset!=0)
        {
            p_ship->ModelMatrix = glm::rotate(p_ship->ModelMatrix, glm::radians(-rotate_offset), glm::vec3(1,0,0));
            rotate_offset = 0;
        }
        p_ship->ModelMatrix = glm::translate(p_ship->ModelMatrix, deltaTime * (-translate_v) * ship_front * 0.4f);
        p_ship->ModelMatrix = glm::rotate(p_ship->ModelMatrix, -turn_v * deltaTime, glm::vec3(0, 0, 1));
    }
    else
    {
        if((rotate_offset < -max_left && rotate_dir == -1) || (rotate_offset > max_left && rotate_dir == 1))
        {
            max_left = (rand()%1000/1000.f) * 8.f + 3.f;
            rotate_dir = -rotate_dir;
        }
        
        float single_rotate = (rand()%2+1) * rotate_dir * rotate_v;
        rotate_offset += single_rotate;
        p_ship->ModelMatrix = glm::rotate(p_ship->ModelMatrix, glm::radians(single_rotate), glm::vec3(1,0,0));
        
        
        if((height_offset < -0.2) || (height_offset > 0.2))
            height_dir = -height_dir;
        
        float single_translate = (rand()%2+1) * height_dir * float_v;
        height_offset += single_translate;
        p_ship->ModelMatrix = glm::translate(p_ship->ModelMatrix, glm::vec3(0,0,single_translate));
    }
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
    
    camera->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera->ProcessMouseScroll(yoffset);
}


