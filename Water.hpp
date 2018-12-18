//
//  Water.hpp
//  waterfall
//
//  Created by simon on 2018/12/13.
//  Copyright © 2018 simon. All rights reserved.
//

#ifndef Water_h
#define Water_h

#include "tools/texture.hpp"

#define STRIP_COUNT        150
#define STRIP_LENGTH    150
#define DATA_LENGTH        STRIP_LENGTH * 2 * (STRIP_COUNT-1)


const GLfloat PI=3.1415926;

GLfloat wave_para2[3][5] =
{  //波长     振幅      波矢量     角速度
    {0.003,     0.025,    1.0,0.0,      PI},
    {0.005,     0.025,    0.0,1.0,     2*PI},
    {0.008,     0.025,    1.0,1.0,     3*PI}
};


class Water
{
private:
    
    GLuint water_vertex_array;
    GLuint water_vertex_buffer;
    GLuint water_normal_buffer;

    
    Shader *shader;
    
    GLfloat pt_strip[STRIP_COUNT * STRIP_LENGTH * 3] = { 0 };
    GLfloat pt_strip2[STRIP_COUNT * STRIP_LENGTH * 3] = { 0 };
    GLfloat pt_normal[STRIP_COUNT * STRIP_LENGTH * 3] = { 0 };
    
    struct DATAS
    {
        GLfloat vertex_data[3];
        GLfloat normal_data[3];
        GLfloat texcoord_data[2];
        GLfloat tangents[3];
        GLfloat bitangents[3];
    }VBOdata[DATA_LENGTH];
    
    GLuint diffuseMap;
    GLuint normalMap;
    
public:
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    
    Water(Shader *waterShader)
    {
        shader = waterShader;
        
        shader->use();
        
        
        diffuseMap = loadJPG("material/5.jpg");
        normalMap = loadJPG("material/1.png");

        initWave();
        shader->setInt("diffuseMap", 0);
        shader->setInt("normalMap", 1);

        glGenVertexArrays(1, &water_vertex_array);
        glBindVertexArray(water_vertex_array);
        
        
        glGenBuffers(1, &water_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, water_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VBOdata), VBOdata, GL_STATIC_DRAW);
        


        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    void draw(glm::mat4 ViewMatrix,glm::mat4 ProjectionMatrix, float currentFrame)
    {
        shader->use();
        shader->setMat4("view", ViewMatrix);
        shader->setMat4("projection", ProjectionMatrix);
        shader->setMat4("model", ModelMatrix);
        
        glBindBuffer(GL_ARRAY_BUFFER, water_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VBOdata), VBOdata, GL_STATIC_DRAW);
        
    
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        
    

        glBindVertexArray(water_vertex_array);
        for(int c=0; c<(STRIP_COUNT-1); c++)
            glDrawArrays(GL_TRIANGLE_STRIP, STRIP_LENGTH*2*c, STRIP_LENGTH*2);
        glBindVertexArray(0);
    }
    
private:
    void initWave(void)
    {
        int index = 0;
        for (int i = 0; i<STRIP_COUNT; i++)
        {
            for (int j = 0; j<STRIP_LENGTH; j++)
            {
                float t = 0.1f;
                pt_strip[index] = -1.0f + i * t;
                pt_strip[index + 1] = 0.0f;
                pt_strip[index + 2] = -1.0f + j * t;
                pt_strip2[index] = -1.0f + i * t;
                pt_strip2[index + 1] = 0.0f;
                pt_strip2[index + 2] = -1.0f + j * t;
                index += 3;
            }
        }
    }
    
    
    void tangentandbitangent(GLint x)
    {
        int x1, x2, x3;
        if (x % 2 == 0)
        {
            x1 = x - 2;
            x2 = x - 1;
            x3 = x;
        }
        else
        {
            x1 = x - 1;
            x2 = x - 2;
            x3 = x;
        }
        glm::vec3 pos1 = glm::vec3(VBOdata[x1].vertex_data[0], VBOdata[x1].vertex_data[1], VBOdata[x1].vertex_data[2]);
        glm::vec3 pos2 = glm::vec3(VBOdata[x2].vertex_data[0], VBOdata[x2].vertex_data[1], VBOdata[x2].vertex_data[2]);
        glm::vec3 pos3 = glm::vec3(VBOdata[x3].vertex_data[0], VBOdata[x3].vertex_data[1], VBOdata[x3].vertex_data[2]);
        
        glm::vec2 uv1 = glm::vec2(VBOdata[x1].texcoord_data[0], VBOdata[x1].texcoord_data[1]);
        glm::vec2 uv2 = glm::vec2(VBOdata[x2].texcoord_data[0], VBOdata[x2].texcoord_data[1]);
        glm::vec2 uv3 = glm::vec2(VBOdata[x3].texcoord_data[0], VBOdata[x3].texcoord_data[1]);
        
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;
        
        GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        
        glm::vec3 tangent, bitangent;
        
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);
        
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent = glm::normalize(bitangent);
        
        VBOdata[x1].tangents[0] = VBOdata[x2].tangents[0] = VBOdata[x3].tangents[0] = tangent.x;
        VBOdata[x1].tangents[1] = VBOdata[x2].tangents[1] = VBOdata[x3].tangents[1] = tangent.y;
        VBOdata[x1].tangents[2] = VBOdata[x2].tangents[2] = VBOdata[x3].tangents[2] = tangent.z;
        
        VBOdata[x1].bitangents[0] = VBOdata[x2].bitangents[0] = VBOdata[x3].bitangents[0] = bitangent.x;
        VBOdata[x1].bitangents[1] = VBOdata[x2].bitangents[1] = VBOdata[x3].bitangents[1] = bitangent.y;
        VBOdata[x1].bitangents[2] = VBOdata[x2].bitangents[2] = VBOdata[x3].bitangents[2] = bitangent.z;
        
    }
    
    
public:
    void UpdateWave(float time)
    {
        int index = 0;
        for (int i = 0; i<STRIP_COUNT; i++)
        {
            for (int j = 0; j<STRIP_LENGTH; j++)
            {
                glm::vec2 offset = glm::vec2(0.0f, 0.0f);
                GLfloat height = 0.0f;
                for (int k = 0; k < 3; k++)
                {
                    glm::vec2 K = glm::vec2(wave_para2[k][2], wave_para2[k][3]);
                    GLfloat Qi = 1 / (wave_para2[k][4] * wave_para2[k][1])*0.15;
                    GLfloat p = wave_para2[k][4] * dot(K, glm::vec2(pt_strip2[index], pt_strip2[index + 2])) + glfwGetTime();
                    
                    offset.x += Qi * wave_para2[k][1] * K.x*cos(p);
                    offset.y += Qi * wave_para2[k][1] * K.y*cos(p);
                    height += wave_para2[k][1] * sin(p);
                }
                pt_strip[index] = pt_strip2[index] + offset.x;
                pt_strip[index + 1] = height;
                pt_strip[index + 2] = pt_strip2[index + 2] + offset.y;
                index += 3;
            }
        }
        
        index = 0;
        for (int i = 0; i<STRIP_COUNT; i++)
        {
            for (int j = 0; j<STRIP_LENGTH; j++)
            {
                int p0 = index - STRIP_LENGTH * 3, p1 = index + 3, p2 = index + STRIP_LENGTH * 3, p3 = index - 3;
                float xa, ya, za, xb, yb, zb;
                if (i > 0)
                {
                    if (j > 0)
                    {
                        xa = pt_strip[p0] - pt_strip[index], ya = pt_strip[p0 + 1] - pt_strip[index + 1], za = pt_strip[p0 + 2] - pt_strip[index + 2];
                        xb = pt_strip[p3] - pt_strip[index], yb = pt_strip[p3 + 1] - pt_strip[index + 1], zb = pt_strip[p3 + 2] - pt_strip[index + 2];
                        pt_normal[index] += ya*zb - yb*za;
                        pt_normal[index + 1] += xb*za - xa*zb;
                        pt_normal[index + 2] += xa*yb - xb*ya;
                    }
                    if (j < STRIP_LENGTH - 1)
                    {
                        xa = pt_strip[p1] - pt_strip[index], ya = pt_strip[p1 + 1] - pt_strip[index + 1], za = pt_strip[p1 + 2] - pt_strip[index + 2];
                        xb = pt_strip[p0] - pt_strip[index], yb = pt_strip[p0 + 1] - pt_strip[index + 1], zb = pt_strip[p0 + 2] - pt_strip[index + 2];
                        pt_normal[index] += ya*zb - yb*za;
                        pt_normal[index + 1] += xb*za - xa*zb;
                        pt_normal[index + 2] += xa*yb - xb*ya;
                    }
                }
                if (i < STRIP_COUNT - 1)
                {
                    if (j > 0)
                    {
                        xa = pt_strip[p3] - pt_strip[index], ya = pt_strip[p3 + 1] - pt_strip[index + 1], za = pt_strip[p3 + 2] - pt_strip[index + 2];
                        xb = pt_strip[p2] - pt_strip[index], yb = pt_strip[p2 + 1] - pt_strip[index + 1], zb = pt_strip[p2 + 2] - pt_strip[index + 2];
                        pt_normal[index] += ya*zb - yb*za;
                        pt_normal[index + 1] += xb*za - xa*zb;
                        pt_normal[index + 2] += xa*yb - xb*ya;
                    }
                    if (j < STRIP_LENGTH - 1)
                    {
                        xa = pt_strip[p2] - pt_strip[index], ya = pt_strip[p2 + 1] - pt_strip[index + 1], za = pt_strip[p2 + 2] - pt_strip[index + 2];
                        xb = pt_strip[p1] - pt_strip[index], yb = pt_strip[p1 + 1] - pt_strip[index + 1], zb = pt_strip[p1 + 2] - pt_strip[index + 2];
                        pt_normal[index] += ya*zb - yb*za;
                        pt_normal[index + 1] += xb*za - xa*zb;
                        pt_normal[index + 2] += xa*yb - xb*ya;
                    }
                }
                index += 3;
            }
        }
        
        int pt;
        for (int c = 0; c<(STRIP_COUNT - 1); c++)
        {
            for (int l = 0; l<2 * STRIP_LENGTH; l++)
            {
                if (l % 2 == 1)
                {
                    pt = c*STRIP_LENGTH + l / 2;
                }
                else
                {
                    pt = c*STRIP_LENGTH + l / 2 + STRIP_LENGTH;
                }
                index = STRIP_LENGTH * 2 * c + l;
                for (int i = 0; i<3; i++)
                {
                    VBOdata[index].vertex_data[i] = pt_strip[pt * 3 + i];
                    VBOdata[index].normal_data[i] = pt_normal[pt * 3 + i];
                    if (i == 0)
                        VBOdata[index].texcoord_data[0] = (pt_strip[pt * 3 + i] + glfwGetTime() / 20.0) / 10.0;
                    else if(i == 2)
                        VBOdata[index].texcoord_data[1] = (pt_strip[pt * 3 + i] + glfwGetTime() / 20.0) / 10.0;
                }
            }
        }
        
        for (int i = 2; i < DATA_LENGTH; i++)
        {
            tangentandbitangent(i);
        }
        
    }
    
};

#endif /* Water_h */
