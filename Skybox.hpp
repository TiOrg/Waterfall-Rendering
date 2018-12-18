//
//  Skybox.hpp
//  waterfall
//
//  Created by simon on 2018/12/6.
//  Copyright © 2018 simon. All rights reserved.
//

#ifndef Skybox_h
#define Skybox_h

#include "tools/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    
    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
};

std::vector<std::string> faces
{
    "material/skybox/right.jpg",
    "material/skybox/left.jpg",
    "material/skybox/top.jpg",
    "material/skybox/bottom.jpg",
    "material/skybox/front.jpg",
    "material/skybox/back.jpg"
};

class Skybox
{
private:
    GLuint skybox_vertex_array;
    GLuint skybox_vertex_buffer;
    GLuint cubemapTexture;
    Shader *shader;

public:
    Skybox(float sceneScale, Shader *skyboxShader)
    {
        shader = skyboxShader;
        
        glm::mat4 Model = glm::mat4(1.0);
        Model = glm::rotate(Model, glm::radians(-90.f), glm::vec3(0,1,0));
        shader->use();
        
        shader->setMat4("Model",Model);
        // skybox VAO
        glGenVertexArrays(1, &skybox_vertex_array);
        glBindVertexArray(skybox_vertex_array);
        
        for(unsigned int i = 0;i < 36*3; i++)
            skyboxVertices[i] *= sceneScale;
        
        
        glGenBuffers(1, &skybox_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, skybox_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, skybox_vertex_buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        
        glBindVertexArray(0);

        cubemapTexture = loadCubemap(faces);
    }
    
    void draw(glm::mat4 ViewProjectionMatrix)
    {
        
        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        shader->use();
        shader->setMat4("VP", ViewProjectionMatrix);
        // skybox cube
        
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        shader->setInt("skybox", 0);
        
        
        glBindVertexArray(skybox_vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS); // set depth function back to default
    }
};
#endif /* Skybox_h */
