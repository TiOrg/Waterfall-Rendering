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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Waterfall", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
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
    // skybox VAO
    GLuint skyboxVertexArray;
    glGenVertexArrays(1, &skyboxVertexArray);
    glBindVertexArray(skyboxVertexArray);

    float sceneScale = 500.f;
    for(unsigned int i = 0;i < 36*3; i++)
    {
        skyboxVertices[i] *= sceneScale;
        skyboxVertices[i] += sceneCenter[i%3];
    }
    
    Shader skyboxShader("shader/skybox.vs", "shader/skybox.fs");

    GLuint skybox_vertex_buffer;
    glGenBuffers(1, &skybox_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    

    std::vector<std::string> faces
    {
        "material/right.jpg",
        "material/left.jpg",
        "material/top.jpg",
        "material/bottom.jpg",
        "material/front.jpg",
        "material/back.jpg"
    };
    GLuint cubemapTexture = loadCubemap(faces);

    
    //===============================================================================================
    // prepare particle
    //===============================================================================================
    
	GLuint particleVertexArray;
	glGenVertexArrays(1, &particleVertexArray);
	glBindVertexArray(particleVertexArray);

	// Create and compile our GLSL program from the shaders
    Shader particleShader( "shader/particle.vs", "shader/particle.fs" );

	static GLfloat* g_particle_position_data = new GLfloat[MaxParticles * 4];
	static GLubyte* g_particle_color_data    = new GLubyte[MaxParticles * 4];

    // init particles
	for(int i=0; i<MaxParticles; i++)
    {
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	GLuint particleTexture = loadDDS("material/particle.DDS");

    // init VBO
	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat g_container_vertex_data[] = {
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.5f,  0.5f, 0.0f,
	};
	GLuint container_vertex_buffer;
	glGenBuffers(1, &container_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, container_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_container_vertex_data), g_container_vertex_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	GLuint particles_position_buffer;
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	GLuint particles_color_buffer;
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

    //===============================================================================================
    // prepare terrain
    //===============================================================================================
    
    GLuint terrainVertexArray;
    glGenVertexArrays(1, &terrainVertexArray);
    glBindVertexArray(terrainVertexArray);
    
    // Create and compile our GLSL program from the shaders
    Shader terrainShader( "shader/terrain.vs", "shader/terrain.fs" );
    
    // Read our .obj file
    std::vector<glm::vec3> vertices;
    loadOBJ("material/waterfall-less.obj", vertices);
    
    // Load it into a VBO
    GLuint terrain_vertex_buffer;
    glGenBuffers(1, &terrain_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    
    
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
        glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

        
        
        //===============================================================================================
        // skybox render
        //===============================================================================================
        
        
        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        skyboxShader.setMat4("VP", ViewProjectionMatrix);
        // skybox cube
        
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        skyboxShader.setInt("skybox", 0);
        
        
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, skybox_vertex_buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDisableVertexAttribArray(0);
        
        glDepthFunc(GL_LESS); // set depth function back to default

        
        //===============================================================================================
        // terrain render
        //===============================================================================================
        
        terrainShader.use();
        // Compute the MVP matrix from keyboard and mouse input
        
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        terrainShader.setMat4("MVP", MVP);
        
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_buffer);
        glVertexAttribPointer(
                              0,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : UVs
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() );
        glDisableVertexAttribArray(0);
        
        
        //===============================================================================================
        // particle render
        //===============================================================================================
        
        // update particles
        
		// Generate new particles each millisecond,
		// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
		// new particles will be huge and the next frame even longer.
		int newparticles = (int)(deltaTime*10000.0);
		if (newparticles > (int)(0.016f*10000.0))
			newparticles = (int)(0.016f*10000.0);
		
		for(int i = 0; i < newparticles; i++){
			int particleIndex = FindUnusedParticle();
            
            // params of particles' moving are shown below
            
            // life span (s)
			ParticlesContainer[particleIndex].life = 1.5f;
            
            // init position
            float radius = 7.5f;
            float theta = (rand()%1000)/1000.0f*3.14f-3.14f; // -pi~pi
//            vec3 posOffset(((rand()%2000 - 1000.0f)/500.0f), 3.5f, -15.f);
            vec3 posOffset(cos(theta)*radius-1.f, 3.5f, sin(theta)*radius-7.f);
            ParticlesContainer[particleIndex].pos = glm::vec3(sceneCenter+posOffset);

            // init speed direction
            // scale of diffuse
            float spread = 0.15f;
            // velocity is time-varying
            float velocity = 10.f * (0.8f + 0.1f * (float) (sin(0.5 * currentFrame) + sin(1.31 * currentFrame)));
            // front side speed
            glm::vec3 maindir = glm::vec3(0.0f, 0.2f, 0.4f);
            // round side speed
//            glm::vec3 maindir = glm::vec3(-0.4f*cos(theta), 0.2f, -0.4f*sin(theta));
            glm::vec3 randomdir = glm::vec3(
				(rand()%2000 - 1000.0f)/1000.0f,
				(rand()%2000 - 1000.0f)/1000.0f,
				(rand()%2000 - 1000.0f)/1000.0f
			);
			ParticlesContainer[particleIndex].speed = (maindir + randomdir*spread) * velocity;

            // blue color with random alpha
			ParticlesContainer[particleIndex].r = 0.4 * 256;
			ParticlesContainer[particleIndex].g = 0.8 * 256;
			ParticlesContainer[particleIndex].b = 0.8 * 256 + 0.2 * (rand() % 256);
			ParticlesContainer[particleIndex].a = (rand() % 256) / 3 +100;

            // random size
            if(randomdir.z > 0.9f) // particles outside is smaller
                ParticlesContainer[particleIndex].size = 0.1f;
            else if(randomdir.z > 0.5f)
                ParticlesContainer[particleIndex].size = (rand()%1000)/5000.0f + 0.1f;
            else
                ParticlesContainer[particleIndex].size = (rand()%1000)/2000.0f + 0.1f;
			
		}

        // draw particles
        
		// Simulate all particles
		int ParticlesCount = 0;
		for(int i=0; i<MaxParticles; i++)
        {
			Particle& p = ParticlesContainer[i]; // shortcut
            
			if(p.life > 0.0f)
            {
				p.life -= deltaTime; // Decrease life
				if (p.life > 0.0f)
                {
					// Simulate simple physics : gravity only, no collisions
					p.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)deltaTime;
					p.pos += p.speed * (float)deltaTime;
					p.cameradistance = glm::length2( p.pos - CameraPosition );

					// Fill the GPU buffer
					g_particle_position_data[4*ParticlesCount+0] = p.pos.x;
					g_particle_position_data[4*ParticlesCount+1] = p.pos.y;
					g_particle_position_data[4*ParticlesCount+2] = p.pos.z;
												   
					g_particle_position_data[4*ParticlesCount+3] = p.size;
												   
					g_particle_color_data[4*ParticlesCount+0] = p.r;
					g_particle_color_data[4*ParticlesCount+1] = p.g;
					g_particle_color_data[4*ParticlesCount+2] = p.b;
					g_particle_color_data[4*ParticlesCount+3] = p.a;

				}
                else
                {
					// Particles that just died will be put at the end of the buffer in SortParticles();
					p.cameradistance = -1.0f;
				}
				ParticlesCount++;
			}
		}
		SortParticles();

        // main buffer draw
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particle_position_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particle_color_data);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Use our shader
		particleShader.use();

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, particleTexture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
        particleShader.setInt("myTextureSampler", 0);

		// Set Uniforms
        particleShader.setVec3("CameraRight_worldspace", ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
        particleShader.setVec3("CameraUp_worldspace", ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
        particleShader.setMat4("VP", ViewProjectionMatrix);

		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, container_vertex_buffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		
		// 2nd attribute buffer : positions of particles' centers
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : x + y + z + size => 4
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : particles' colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : r + g + b + a => 4
			GL_UNSIGNED_BYTE,                 // type
			GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

		// Draw particles
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

        
        //===============================================================================================
        // render end
        //===============================================================================================
        
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
    
    // Do some cleans
    delete[] g_particle_color_data;
	delete[] g_particle_position_data;
	// Cleanup VBO and shader
	glDeleteBuffers(1, &particles_color_buffer);
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &container_vertex_buffer);
	glDeleteVertexArrays(1, &particleVertexArray);
    particleShader._delete();
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


