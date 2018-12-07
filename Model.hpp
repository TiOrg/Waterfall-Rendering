//
//  Model.hpp
//  waterfall
//
//  Created by simon on 2018/12/7.
//  Copyright © 2018 simon. All rights reserved.
//

#ifndef Model_h
#define Model_h


class Model
{
private:
    std::vector<glm::vec3> model_vertices;
    
    GLuint model_vertex_array;
    GLuint model_vertex_buffer;
    Shader *shader;
public:
    Model(const char *objpath, Shader *modelShader)
    {
        
        glGenVertexArrays(1, &model_vertex_buffer);
        glBindVertexArray(model_vertex_buffer);
        
        
        shader = modelShader;
        
        // Read our .obj file
        loadOBJ(objpath, model_vertices);

        // Load it into a VBO
        glGenBuffers(1, &model_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, model_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, model_vertices.size() * sizeof(glm::vec3), &model_vertices[0], GL_STATIC_DRAW);
    }
    
    // Use this if you want to move the model
    Model(const char *objpath, glm::vec3 modelCenter, glm::vec3 targetCenter, Shader *modelShader)
    {
        
        glGenVertexArrays(1, &model_vertex_buffer);
        glBindVertexArray(model_vertex_buffer);
        
        
        shader = modelShader;
        
        // Read our .obj file
        loadOBJ(objpath, model_vertices);
        
        glm::vec3 offset = targetCenter - modelCenter;
        for(unsigned int i = 0;i < model_vertices.size(); i++)
        {
            model_vertices[i] += offset;
        }
        
        // Load it into a VBO
        glGenBuffers(1, &model_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, model_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, model_vertices.size() * sizeof(glm::vec3), &model_vertices[0], GL_STATIC_DRAW);
        
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
        glBindBuffer(GL_ARRAY_BUFFER, model_vertex_buffer);
        glVertexAttribPointer(
                              0,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : UVs
        glDrawArrays(GL_TRIANGLES, 0, model_vertices.size() );
        glDisableVertexAttribArray(0);
        
    }
};

#endif /* Model_h */
