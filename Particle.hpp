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
    float camera_distance; // *Squared* distance to the camera. if dead : -1.0f
    
    bool operator<(const Particle& that) const {
        // Sort in reverse order : far particles drawn first.
        return this->camera_distance > that.camera_distance;
    }
};

const int MAXPARTICLES = 80000;

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
    glm::vec3 system_center;
    
    Particle particles_container[MAXPARTICLES];
    int last_used_particle = 0;
    int particles_count;

    // vertex position and color
    GLfloat* g_particle_position_data = new GLfloat[MAXPARTICLES * 4];
    GLubyte* g_particle_color_data    = new GLubyte[MAXPARTICLES * 4];

    // Shader pointer
    Shader *shader;
    // VBO*3
    GLuint container_vertex_buffer;
    GLuint particles_position_buffer;
    GLuint particles_color_buffer;
    // VAO
    GLuint particle_vertex_array;
    // Texture object
    GLuint particle_texture;
    
    
public:
    Particles(glm::vec3 center, Shader *particleShader)
    {
        system_center = center;
        
        glGenVertexArrays(1, &particle_vertex_array);
        glBindVertexArray(particle_vertex_array);

        shader = particleShader;
      
        // init particles
        for(int i=0; i<MAXPARTICLES; i++)
        {
            particles_container[i].life = -1.0f;
            particles_container[i].camera_distance = -1.0f;
        }
        
        particle_texture = loadDDS("material/particle.DDS");
        
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
        glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        
        // The VBO containing the colors of the particles
        glGenBuffers(1, &particles_color_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        // Initialize with empty (NULL) buffer : it will be updated later, each frame.
        glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
        
        
        
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
        glBindVertexArray(0);

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
        glDeleteVertexArrays(1, &particle_vertex_array);
        shader->_delete();
    }
    
    void SpawnParticles(float currentFrame, int newparticles)
    {
        // Generate new particles each millisecond,
        // but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
        // new particles will be huge and the next frame even longer.
        
//        int temp =sin(int(currentFrame * 8))*20;

        for(int i = 0; i < newparticles; i++){
            int particleIndex = FindUnusedParticle();
            
            // params of particles' moving are shown below
            
            // life span (s)
            particles_container[particleIndex].life = 2.f;
            
            // init position
            float radius = 10.f;
            float theta = (rand()%1000)/1000.0f*0.8f*3.14f-0.87f*3.14f; // -pi~pi
            //            vec3 posOffset(((rand()%2000 - 1000.0f)/500.0f), 3.5f, -15.f);
            
//            int temp2 = int(-theta/0.524);
//            if(temp % 6 == temp2)
//                continue;
//
            glm::vec3 posOffset(cos(theta)*radius-1.f, 3.5f, sin(theta)*radius-7.f);
            particles_container[particleIndex].pos = glm::vec3(system_center+posOffset);
            
            // init speed direction
            // scale of diffuse
            float spread = 0.2f;
            // velocity is time-varying
            float velocity = 4.f;
            // front side speed
//            glm::vec3 maindir = glm::vec3(0.0f, 0.2f, 0.4f);
            // round side speed
            glm::vec3 maindir = glm::vec3(-0.4f*cos(theta), 0.2f, -0.4f*sin(theta)+0.2f);
            glm::vec3 randomdir = glm::vec3(
                                            (rand()%2000 - 1000.0f)/1000.0f,
                                            (rand()%2000 - 1000.0f)/1000.0f,
                                            (rand()%2000 - 1000.0f)/1000.0f
                                            );
            particles_container[particleIndex].speed = (maindir + randomdir*spread) * velocity;
            
            // blue color with random alpha
            particles_container[particleIndex].r = 76;//+ 0.02 * (rand() % 256);//0.1604 * 256;
            particles_container[particleIndex].g = 129+ 0.05 * (rand() % 256);//0.5203 * 256;
            particles_container[particleIndex].b = 186 + 0.18 * (rand() % 256);//0.6400 * 256 + 0.2 * (rand() % 256);
            particles_container[particleIndex].a = (rand() % 256) / 3 +160;
            
            // random size
            if(randomdir.z > 0.9f) // particles outside is smaller
                particles_container[particleIndex].size = 0.1f;
            else if(randomdir.z > 0.5f)
                particles_container[particleIndex].size = (rand()%1000)/5000.0f + 0.1f;
            else
                particles_container[particleIndex].size = (rand()%1000)/2000.0f + 0.1f;
        }
    }
    
    void UpdateParticles(float deltaTime, glm::vec3 CameraPosition)
    {
        // Simulate all particles
        particles_count = 0;
        for(int i=0; i<MAXPARTICLES; i++)
        {
            Particle& p = particles_container[i]; // shortcut
            
            if(p.life > 0.0f)
            {
                p.life -= deltaTime; // Decrease life
                if (p.life > 0.0f)
                {
                    // Simulate simple physics : gravity only, no collisions
                    p.speed += glm::vec3(0.0f,-3.81f, 0.0f) * (float)deltaTime;
                    p.pos += p.speed * (float)deltaTime;
                    p.camera_distance = glm::length2( p.pos - CameraPosition );
                    p.a = p.pos.y * 255 / 6.f;
                    
//                    if( p.pos.y < 1.f && !p.flying)
//                    {
//                        p.flying = 1;
//                        float tempx = (rand()%1000)/1000.f*p.speed.x * 2.f;
//                        float tempy = (rand()%1000)/1000.f;
//                        float tempz = (rand()%1000)/1000.f*p.speed.z * 2.f;
//
//                        p.pos = p.pos + glm::vec3(tempx, tempy * 5, tempz);
//                        p.size = 1.f-tempy;
//                        p.r = 255;
//                        p.g = 255;
//                        p.b = 255;
//                        p.a = 255*0.1;
//                    }
                    
                    p.camera_distance = glm::length2( p.pos - CameraPosition );

                    // Fill the GPU buffer
                    g_particle_position_data[4*particles_count+0] = p.pos.x;
                    g_particle_position_data[4*particles_count+1] = p.pos.y;
                    g_particle_position_data[4*particles_count+2] = p.pos.z;
                    
                    g_particle_position_data[4*particles_count+3] = p.size;
                    
                    g_particle_color_data[4*particles_count+0] = p.r;
                    g_particle_color_data[4*particles_count+1] = p.g;
                    g_particle_color_data[4*particles_count+2] = p.b;
                    g_particle_color_data[4*particles_count+3] = p.a;
                    
                }
                else
                {
                    // Particles that just died will be put at the end of the buffer in SortParticles();
                    p.camera_distance = -1.0f;
                }
                particles_count++;
            }
        }
        SortParticles();
    }
    
    void draw(glm::mat4 ViewMatrix, glm::mat4 ViewProjectionMatrix)
    {
        // main buffer draw
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, particles_count * sizeof(GLfloat) * 4, g_particle_position_data);
        
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, particles_count * sizeof(GLubyte) * 4, g_particle_color_data);
        
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Use our shader
        shader->use();
        
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, particle_texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        shader->setInt("myTextureSampler", 0);
        
        // Set Uniforms
        shader->setVec3("CameraRight_worldspace", ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
        shader->setVec3("CameraUp_worldspace", ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
        shader->setMat4("VP", ViewProjectionMatrix);
  
        glBindVertexArray(particle_vertex_array);

        // Draw particles
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particles_count);
        glBindVertexArray(0);
        
    }
  
private:
    
    // Finds a Particle in particles_container which isn't used yet.
    // (i.e. life < 0);
    int FindUnusedParticle(){
        
        for(int i=last_used_particle; i<MAXPARTICLES; i++){
            if (particles_container[i].life < 0){
                last_used_particle = i;
                return i;
            }
        }
        
        for(int i=0; i<last_used_particle; i++){
            if (particles_container[i].life < 0){
                last_used_particle = i;
                return i;
            }
        }
        
        return 0; // All particles are taken, override the first one
    }
    
    void SortParticles(){
        std::sort(&particles_container[0], &particles_container[MAXPARTICLES]);
    }
    
};


#endif /* particle_h */
