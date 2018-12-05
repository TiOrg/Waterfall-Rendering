//
//  Particles.cpp
//  test
//
//  Created by simon on 2018/11/9.
//  Copyright © 2018 simon. All rights reserved.
//

#if defined(_MSC_VER)
// Make MS math.h define M_PI
#define _USE_MATH_DEFINES
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

//#include "tinycthread.hpp"
//#include <getopt.h>
//#include "linmath.h"

#include <glad/glad.h>
//#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Particles.h"

//========================================================================
// Initialize a new particle
//========================================================================

void Particles::init_particle(PARTICLE *p, double t)
{
    float xy_angle, velocity;
    
    // Start position of particle is at the fountain blow-out
    p->x = 0.f;
    p->y = 0.f;
    p->z = FOUNTAIN_HEIGHT;
    
    // Start velocity is up (Z)...
    p->vz = 0.7f + (0.3f / 4096.f) * (float) (rand() & 4095);
    
    // ...and a randomly chosen X/Y direction
    xy_angle = (2.f * (float) M_PI / 4096.f) * (float) (rand() & 4095);
    p->vx = 0.4f * (float) cos(xy_angle);
    p->vy = 0.4f * (float) sin(xy_angle);
    
    // Scale velocity vector according to a time-varying velocity
    velocity = VELOCITY * (0.8f + 0.1f * (float) (sin(0.5 * t) + sin(1.31 * t)));
    p->vx *= velocity;
    p->vy *= velocity;
    p->vz *= velocity;
    
    // Color is time-varying
    p->r = 0.7f + 0.3f * (float) sin(0.34 * t + 0.1);
    p->g = 0.6f + 0.4f * (float) sin(0.63 * t + 1.1);
    p->b = 0.6f + 0.4f * (float) sin(0.91 * t + 2.1);
    
    // The particle is new-born and active
    p->life = 1.f;
    p->active = 1;
}


//========================================================================
// Update a particle
//========================================================================

void Particles::update_particle(PARTICLE *p, float dt)
{
    // If the particle is not active, we need not do anything
    if (!p->active)
        return;
    
    // The particle is getting older...
    p->life -= dt * (1.f / LIFE_SPAN);
    
    // Did the particle die?
    if (p->life <= 0.f)
    {
        p->active = 0;
        return;
    }
    
    // Apply gravity
    p->vz = p->vz - GRAVITY * dt;
    
    // Update particle position
    p->x = p->x + p->vx * dt;
    p->y = p->y + p->vy * dt;
    p->z = p->z + p->vz * dt;
    
    // Simple collision detection + response
    if (p->vz < 0.f)
    {
        // Particles should bounce on the floor (with friction)
        if (p->z < PARTICLE_SIZE / 2)
        {
            p->vz = -FRICTION * p->vz;
            p->z  = PARTICLE_SIZE / 2 +
            FRICTION * (PARTICLE_SIZE / 2 - p->z);
        }
    }
}


//========================================================================
// The main frame for the particle engine. Called once per frame.
//========================================================================

void Particles::calculate_particles(double t, float dt)
{
    int i;
    float dt2;
    
    // Update particles (iterated several times per frame if dt is too large)
    while (dt > 0.f)
    {
        // Calculate delta time for this iteration
        dt2 = dt < MIN_DELTA_T ? dt : MIN_DELTA_T;
        
        for (i = 0;  i < MAX_PARTICLES;  i++)
            update_particle(&particles[i], dt2);
        
        min_age += dt2;
        
        // Should we create any new particle(s)?
        while (min_age >= BIRTH_INTERVAL)
        {
            min_age -= BIRTH_INTERVAL;
            
            // Find a dead particle to replace with a new one
            for (i = 0;  i < MAX_PARTICLES;  i++)
            {
                if (!particles[i].active)
                {
                    init_particle(&particles[i], t + min_age);
                    update_particle(&particles[i], min_age);
                    break;
                }
            }
        }
        dt -= dt2;
    }
}


//========================================================================
// Draw all active particles. We use OpenGL 1.1 vertex
// arrays for this in order to accelerate the drawing.
//========================================================================

void Particles::draw_particles(double t, float dt, glm::mat4 modelview_matrix)
{
    int i;
    int particle_count = 0;
    Vertex vertex_array[BATCH_PARTICLES * PARTICLE_VERTS];
    Vertex* vptr = vertex_array;
    float alpha;
    GLuint rgba;
    glm::vec3 quad_lower_left, quad_lower_right;
    PARTICLE* pptr = particles;

    GLfloat mat[16];

    const float *mat_ptr = (const float*)glm::value_ptr(modelview_matrix);
    for (int i = 0; i < 16; ++i)
        mat[i] = mat_ptr[i];
    
    quad_lower_left.x = (-PARTICLE_SIZE / 2) * (mat[0] + mat[1]);
    quad_lower_left.y = (-PARTICLE_SIZE / 2) * (mat[4] + mat[5]);
    quad_lower_left.z = (-PARTICLE_SIZE / 2) * (mat[8] + mat[9]);
    quad_lower_right.x = (PARTICLE_SIZE / 2) * (mat[0] - mat[1]);
    quad_lower_right.y = (PARTICLE_SIZE / 2) * (mat[4] - mat[5]);
    quad_lower_right.z = (PARTICLE_SIZE / 2) * (mat[8] - mat[9]);
    
    // all particles are transparent
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    
    
//    glEnable( GL_POLYGON_SMOOTH );

    
    // Select particle texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, particleTexture);
//
//    //GL_T2F_C4UB_V3F means:
//    //2 floats for texture coords, 4 ubytes for color and 3 floats for vertex coord
//    glInterleavedArrays(GL_T2F_C4UB_V3F, 0, vertex_array);
//
    
    // Loop through all particles and build vertex arrays.
    glBindVertexArray(particleVAO);
    
    for (i = 0;  i < MAX_PARTICLES;  i++)
    {
        if (pptr->active)
        {
            // Calculate particle intensity (we set it to max during 75%
            // of its life, then it fades out)
            alpha =  4.f * pptr->life;
            if (alpha > 1.f)
                alpha = 1.f;
            
            // Convert color from float to 8-bit (store it in a 32-bit
            // integer using endian independent type casting)
            ((GLubyte*) &rgba)[0] = (GLubyte)(pptr->r * 255.f);
            ((GLubyte*) &rgba)[1] = (GLubyte)(pptr->g * 255.f);
            ((GLubyte*) &rgba)[2] = (GLubyte)(pptr->b * 255.f);
            ((GLubyte*) &rgba)[3] = (GLubyte)(alpha * 255.f);
            
            // 3) Translate the quad to the correct position in modelview
            // space and store its parameters in vertex arrays (we also
            // store texture coord and color information for each vertex).
            
            // Lower left corner
            vptr->s    = 0.f;
            vptr->t    = 0.f;
            vptr->rgba = rgba;
            vptr->x    = pptr->x + quad_lower_left.x;
            vptr->y    = pptr->y + quad_lower_left.y;
            vptr->z    = pptr->z + quad_lower_left.z;
            vptr ++;
            
            // Lower right corner
            vptr->s    = 1.f;
            vptr->t    = 0.f;
            vptr->rgba = rgba;
            vptr->x    = pptr->x + quad_lower_right.x;
            vptr->y    = pptr->y + quad_lower_right.y;
            vptr->z    = pptr->z + quad_lower_right.z;
            vptr ++;
            
            // Upper right corner
            vptr->s    = 1.f;
            vptr->t    = 1.f;
            vptr->rgba = rgba;
            vptr->x    = pptr->x - quad_lower_left.x;
            vptr->y    = pptr->y - quad_lower_left.y;
            vptr->z    = pptr->z - quad_lower_left.z;
            vptr ++;
            
            // Upper left corner
            vptr->s    = 0.f;
            vptr->t    = 1.f;
            vptr->rgba = rgba;
            vptr->x    = pptr->x - quad_lower_right.x;
            vptr->y    = pptr->y - quad_lower_right.y;
            vptr->z    = pptr->z - quad_lower_right.z;
            vptr ++;
            
            // Increase count of drawable particles
            particle_count ++;
        }
        
        // If we have filled up one batch of particles, draw it as a set
        // of quads using glDrawArrays.
        if (particle_count >= BATCH_PARTICLES)
        {
            // The first argument tells which primitive type we use (QUAD)
            // The second argument tells the index of the first vertex (0)
            // The last argument is the vertex count
            glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*particle_count, vptr, GL_STREAM_DRAW);
            
            glDrawArrays(GL_QUADS, 0, PARTICLE_VERTS * particle_count);
            particle_count = 0;
            vptr = vertex_array;
        }
        
        // Next particle
        pptr++;
    }

    
    // Draw final batch of particles (if any)
    glDrawArrays(GL_QUADS, 0, PARTICLE_VERTS * particle_count);
    
    glBindVertexArray(0);
    // Disable vertex arrays (Note: glInterleavedArrays implicitly called
    // glEnableClientState for vertex, texture coord and color arrays)
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    
    glDepthMask(GL_TRUE);
}

