//
//  Terrain.hpp
//  waterfall
//
//  Created by simon on 2018/12/6.
//  Copyright © 2018 simon. All rights reserved.
//

#ifndef Terrain_h
#define Terrain_h

class Terrain
{
private:
    GLuint terrainVertexArray;
    std::vector<glm::vec3> vertices;
    GLuint terrain_vertex_buffer;
    Shader *shader;
public:
    Terrain(Shader *terrainShader)
    {
        
        glGenVertexArrays(1, &terrainVertexArray);
        glBindVertexArray(terrainVertexArray);
        

        shader = terrainShader;
        
        // Read our .obj file
        loadOBJ("material/waterfall-less.obj", vertices);
        
        // Load it into a VBO
        glGenBuffers(1, &terrain_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        
    }
    
    void draw(glm::mat4 MVP)
    {
        
        shader->use();
        // Compute the MVP matrix from keyboard and mouse input
        
   
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        shader->setMat4("MVP", MVP);
        
        
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
        
    }
};

#endif /* Terrain_h */
