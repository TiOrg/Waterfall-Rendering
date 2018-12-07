//
//  particle.hpp
//  waterfall
//
//  Created by simon on 2018/12/5.
//  Copyright © 2018 simon. All rights reserved.
//

#ifndef particle_h
#define particle_h

#include "Shader.hpp"

// CPU representation of a particle
struct Particle{
    glm::vec3 pos, speed;
    unsigned char r,g,b,a; // Color
    float size, angle, weight;
    float life; // Remaining life of the particle. if <0 : dead and unused.
    float cameradistance; // *Squared* distance to the camera. if dead : -1.0f
    
    bool operator<(const Particle& that) const {
        // Sort in reverse order : far particles drawn first.
        return this->cameradistance > that.cameradistance;
    }
};

const int MaxParticles = 100000;

// implies the relative position of container
static const GLfloat g_container_vertex_data[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
    0.5f,  0.5f, 0.0f,
};

class Particles
{
private:
    glm::vec3 sceneCenter;
    
    Particle ParticlesContainer[MaxParticles];
    int LastUsedParticle = 0;
    int ParticlesCount;

    // vertex position and color
    GLfloat* g_particle_position_data = new GLfloat[MaxParticles * 4];
    GLubyte* g_particle_color_data    = new GLubyte[MaxParticles * 4];

    // Shader pointer
    Shader *shader;
    // VBO*3
    GLuint container_vertex_buffer;
    GLuint particles_position_buffer;
    GLuint particles_color_buffer;
    // VAO
    GLuint particleVertexArray;
    // Texture object
    GLuint particleTexture;
    
    
public:
    Particles(glm::vec3 center, Shader *particleShader)
    {
        sceneCenter = center;
        
        glGenVertexArrays(1, &particleVertexArray);
        glBindVertexArray(particleVertexArray);

        shader = particleShader;
      
        // init particles
        for(int i=0; i<MaxParticles; i++)
        {
            ParticlesContainer[i].life = -1.0f;
            ParticlesContainer[i].cameradistance = -1.0f;
        }
        
        particleTexture = loadDDS("material/particle.DDS");
        
        // init VBO
        // The VBO containing the 4 vertices of the particles.
        // Thanks to instancing, they will be shared by all particles.
        
        glGenBuffers(1, &container_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, container_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_container_vertex_data), g_container_vertex_data, GL_STATIC_DRAW);
        
        // The VBO containing the positions and sizes of the particles
        glGenBuffers(1, &particles_position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        // Initialize with empty (NULL) buffer : it will be updated later, each frame.
        glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        
        // The VBO containing the colors of the particles
        glGenBuffers(1, &particles_color_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        // Initialize with empty (NULL) buffer : it will be updated later, each frame.
        glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
        
    }
    
    
    ~Particles()
    {
        // Do some cleans
        delete[] g_particle_color_data;
        delete[] g_particle_position_data;
        // Cleanup VBO and shader
        glDeleteBuffers(1, &particles_color_buffer);
        glDeleteBuffers(1, &particles_position_buffer);
        glDeleteBuffers(1, &container_vertex_buffer);
        glDeleteVertexArrays(1, &particleVertexArray);
        shader->_delete();
    }
    
    void SpawnParticles(float currentFrame, int newparticles)
    {
        // Generate new particles each millisecond,
        // but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
        // new particles will be huge and the next frame even longer.

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
    }
    
    void UpdateParticles(float deltaTime, glm::vec3 CameraPosition)
    {
        // Simulate all particles
        ParticlesCount = 0;
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
    }
    
    void draw(glm::mat4 ViewMatrix, glm::mat4 ViewProjectionMatrix)
    {
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
        shader->use();
        
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, particleTexture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        shader->setInt("myTextureSampler", 0);
        
        // Set Uniforms
        shader->setVec3("CameraRight_worldspace", ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
        shader->setVec3("CameraUp_worldspace", ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
        shader->setMat4("VP", ViewProjectionMatrix);
        
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
            
    }
  
private:
    
    // Finds a Particle in ParticlesContainer which isn't used yet.
    // (i.e. life < 0);
    int FindUnusedParticle(){
        
        for(int i=LastUsedParticle; i<MaxParticles; i++){
            if (ParticlesContainer[i].life < 0){
                LastUsedParticle = i;
                return i;
            }
        }
        
        for(int i=0; i<LastUsedParticle; i++){
            if (ParticlesContainer[i].life < 0){
                LastUsedParticle = i;
                return i;
            }
        }
        
        return 0; // All particles are taken, override the first one
    }
    
    void SortParticles(){
        std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
    }
    
};


#endif /* particle_h */
