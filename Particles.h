#include <stdlib.h>
#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "Shader.h"

// This structure is used for interleaved vertex arrays (see the
// draw_particles function)

typedef struct
{
    GLfloat x, y, z;      // Vertex coordinates
    GLfloat s, t;         // Texture coordinates
    GLuint  rgba;         // Color (four ubytes packed into an uint)
} Vertex;

//========================================================================
// Texture declarations (we hard-code them into the source code, since
// they are so simple)
//========================================================================

#define P_TEX_WIDTH  8    // Particle texture dimensions
#define P_TEX_HEIGHT 8

// Particle texture (a simple spot)
const unsigned char particle_tex_spot[ P_TEX_WIDTH * P_TEX_HEIGHT ] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x22, 0x22, 0x11, 0x00, 0x00,
    0x00, 0x11, 0x33, 0x88, 0x77, 0x33, 0x11, 0x00,
    0x00, 0x22, 0x88, 0xff, 0xee, 0x77, 0x22, 0x00,
    0x00, 0x22, 0x77, 0xee, 0xff, 0x88, 0x22, 0x00,
    0x00, 0x11, 0x33, 0x77, 0x88, 0x33, 0x11, 0x00,
    0x00, 0x00, 0x11, 0x33, 0x22, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//========================================================================
// These are fixed constants that control the particle engine. In a
// modular world, these values should be variables...
//========================================================================

// Maximum number of particles
#define MAX_PARTICLES   3000

// Particle size
#define PARTICLE_SIZE   1.0f

// Life span of a particle (in seconds)
#define LIFE_SPAN       4.f

// A new particle is born every [BIRTH_INTERVAL] second
#define BIRTH_INTERVAL (LIFE_SPAN/(float)MAX_PARTICLES)

// Gravitational constant (m/s^2)
#define GRAVITY         9.8f

// Base initial velocity (m/s)
#define VELOCITY        8.f

// Bounce friction (1.0 = no friction, 0.0 = maximum friction)
#define FRICTION        0.2f

// "Fountain" height (m)
#define FOUNTAIN_HEIGHT 5.f

// Fountain radius (m)
#define FOUNTAIN_RADIUS 1.6f

// Minimum delta-time for particle phisics (s)
#define MIN_DELTA_T     (BIRTH_INTERVAL * 0.5f)

// Number of particles to draw in each batch
// (70 corresponds to 7.5 KB = will not blow
// the L1 data cache on most CPUs)
#define BATCH_PARTICLES 70

// Number of vertices per particle
#define PARTICLE_VERTS  4

typedef struct {
    float x,y,z;     // Position in space
    float vx,vy,vz;  // Velocity vector
    float r,g,b;     // Color of particle
    float life;      // Life of particle (1.0 = newborn, < 0.0 = dead)
    int   active;    // Tells if this particle is active
} PARTICLE;


class Particles
{
private:
    // Texture object IDs
    GLuint particleTexture;
    GLuint particleVAO;
    GLuint particleVBO;

    // Global vectors holding all particles. We use two vectors for double
    // buffering.
    PARTICLE particles[MAX_PARTICLES];
    
    // Global variable holding the age of the youngest particle
    float min_age;
    
    GLFWwindow* window;

protected:
    void init_particle(PARTICLE *p, double t);
    void update_particle(PARTICLE *p, float dt);
    

public:
    

    Particles(GLFWwindow* w,  GLuint tex_id, GLuint vao, GLuint vbo)
    {
        window = w;
        particleTexture = tex_id;
        particleVAO=vao;
        particleVBO=vbo;
    }
    ~Particles(){}
    void calculate_particles(double t, float dt);
    void draw_particles(double t, float dt, glm::mat4 modelview_martix);
    

};
