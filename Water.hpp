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


#define START_X        -4.0
#define START_Y        -2.5
#define START_Z        0
#define LENGTH_X    0.1
#define LENGTH_Y    0.1

#define HEIGHT_SCALE    1.6

#define WAVE_COUNT        6

#define STRIP_COUNT        80
#define STRIP_LENGTH    50
#define DATA_LENGTH        STRIP_LENGTH*2*(STRIP_COUNT-1)



glm::vec4 materAmbient(0.1, 0.1, 0.3, 1.0);
glm::vec4 materSpecular(0.8, 0.8, 0.9, 1.0);
glm::vec4 lightDiffuse(0.7, 0.7, 0.8, 1.0);
glm::vec4 lightAmbient(0.0, 0.0, 0.0, 1.0);
glm::vec4 lightSpecular(1.0, 1.0, 1.0, 1.0);
glm::vec4 envirAmbient(0.1, 0.1, 0.3, 1.0);


//wave_length, wave_height, wave_dir, wave_speed, wave_start.x, wave_start.y
static const GLfloat wave_para[6][6] = {
    {    1.6,    0.12,    0.9,    0.06,    0.0,    0.0    },
    {    1.3,    0.1,    1.14,    0.09,    0.0,    0.0    },
    {    0.2,    0.01,    0.8,    0.08,    0.0,    0.0    },
    {    0.18,    0.008,    1.05,    0.1,    0.0,    0.0    },
    {    0.23,    0.005,    1.15,    0.09,    0.0,    0.0    },
    {    0.12,    0.003,    0.97,    0.14,    0.0,    0.0    }
};

static const GLfloat gerstner_pt_a[22] = {
    0.0,0.0, 41.8,1.4, 77.5,5.2, 107.6,10.9,
    132.4,17.7, 152.3,25.0, 167.9,32.4, 179.8,39.2,
    188.6,44.8, 195.0,48.5, 200.0,50.0
};
static const GLfloat gerstner_pt_b[22] = {
    0.0,0.0, 27.7,1.4, 52.9,5.2, 75.9,10.8,
    97.2,17.6, 116.8,25.0, 135.1,32.4, 152.4,39.2,
    168.8,44.8, 184.6,48.5, 200.0,50.0
};
static const GLint gerstner_sort[6] = {
    0, 0, 1, 1, 1, 1
};

class Water
{
private:
    GLuint water_vertex_array;
    GLuint water_vertex_buffer;
    GLuint water_normal_buffer;

    GLuint diffuse_texture;
    GLuint normal_texture;

    Shader *shader;
    
    
    GLfloat pt_strip[STRIP_COUNT*STRIP_LENGTH*3] = {0};
    GLfloat pt_normal[STRIP_COUNT*STRIP_LENGTH*3] = {0};
    GLfloat vertex_data[DATA_LENGTH*3] = {0};
    GLfloat normal_data[DATA_LENGTH*3] = {0};
    
    GLfloat wave_length[WAVE_COUNT];
    GLfloat wave_height[WAVE_COUNT];
    GLfloat wave_dir[WAVE_COUNT];
    GLfloat wave_speed[WAVE_COUNT];
    GLfloat wave_start[WAVE_COUNT*2];
    
    
public:
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    
    Water(Shader *waterShader)
    {
        shader = waterShader;
        
        shader->use();
        
        shader->setVec4("materAmbient", materAmbient);
        shader->setVec4("materSpecular", materSpecular);
        shader->setVec4("lightDiffuse", lightDiffuse);
        shader->setVec4("lightAmbient", lightAmbient);
        shader->setVec4("lightSpecular", lightSpecular);
        shader->setVec4("envirAmbient", envirAmbient);
        
        initWave();
        

        glGenVertexArrays(1, &water_vertex_array);
        glBindVertexArray(water_vertex_array);
        
        
        glGenBuffers(1, &water_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, water_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), NULL, GL_STATIC_DRAW);
        
        glGenBuffers(1, &water_normal_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, water_normal_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normal_data), NULL, GL_STATIC_DRAW);
        
        
        // set texture
        diffuse_texture = loadJPG("material/lake.jpg");
        shader->setInt("texture[0]", 0);


        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, water_vertex_buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, water_normal_buffer);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        
        glBindVertexArray(0);
    }
    
    void draw(glm::mat4 ViewMatrix,glm::mat4 Projection, float currentFrame)
    {
        glm::mat4 ModelViewMat = ViewMatrix * ModelMatrix;
        glm::mat3 NormalMat = glm::transpose(glm::inverse(glm::mat3(ModelViewMat)));
        
        shader->use();

        shader->setFloat("time", currentFrame);
        shader->setMat4("modelViewMat", ModelViewMat);
        shader->setMat4("perspProjMat", Projection);
        shader->setMat4("normalMat", NormalMat);
        
   
        glBindBuffer(GL_ARRAY_BUFFER, water_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER, water_normal_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normal_data), normal_data, GL_STATIC_DRAW);
        
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_texture);
        
        

        glBindVertexArray(water_vertex_array);
        for(int c=0; c<(STRIP_COUNT-1); c++)
            glDrawArrays(GL_TRIANGLES, STRIP_LENGTH*2*c, STRIP_LENGTH*2);
        glBindVertexArray(0);
    }
    
private:
    void initWave(void)
    {
        //Initialize values{}
        for(int w=0; w<WAVE_COUNT; w++)
        {
            wave_length[w] = wave_para[w][0];
            wave_height[w] = wave_para[w][1];
            wave_dir[w] = wave_para[w][2];
            wave_speed[w] = wave_para[w][3];
            wave_start[w*2] = wave_para[w][4];
            wave_start[w*2+1] = wave_para[w][5];
        }
        
        //Initialize pt_strip[]
        int index=0;
        for(int i=0; i<STRIP_COUNT; i++)
        {
            for(int j=0; j<STRIP_LENGTH; j++)
            {
                pt_strip[index] = START_X + i*LENGTH_X;
                pt_strip[index+1] = START_Y + j*LENGTH_Y;
                index += 3;
            }
        }
    }
    
    
    static float gerstnerZ(float w_length, float w_height, float x_in, const GLfloat gerstner[22])
    {
        x_in = x_in * 400.0 / w_length;
        
        while(x_in < 0.0)
            x_in += 400.0;
        while(x_in > 400.0)
            x_in -= 400.0;
        if(x_in > 200.0)
            x_in = 400.0 - x_in;
        
        int i = 0;
        float yScale = w_height/50.0;
        while(i<18 && (x_in<gerstner[i] || x_in>=gerstner[i+2]))
            i+=2;
        if(x_in == gerstner[i])
            return gerstner[i+1] * yScale;
        if(x_in > gerstner[i])
            return ((gerstner[i+3]-gerstner[i+1]) * (x_in-gerstner[i]) / (gerstner[i+2]-gerstner[i]) + gerstner[i+3]) * yScale;
        return 0;
    }
    
    
    static int normalizeF(float in[], float out[], int count)
    {
        int t=0;
        float l = 0.0;
        
        if(count <= 0.0){
            printf("normalizeF(): Number of dimensions should be larger than zero.\n");
            return 1;
        }
        while(t<count && in[t]<0.0000001 && in[t]>-0.0000001){
            t++;
        }
        if(t == count){
            printf("normalizeF(): The input vector is too small.\n");
            return 1;
        }
        for(t=0; t<count; t++)
            l += in[t] * in[t];
        if(l < 0.0000001){
            l = 0.0;
            for(t=0; t<count; t++)
                in[t] *= 10000.0;
            for(t=0; t<count; t++)
                l += in[t] * in[t];
        }
        l = sqrt(l);
        for(t=0; t<count; t++)
            out[t] /= l;
        
        return 0;
    }
    
public:
    void UpdateWave(float time)
    {
        //Calculate pt_strip[z], poly_normal[] and pt_normal[]
        int index=0;
        float d, wave;
        for(int i=0; i<STRIP_COUNT; i++)
        {
            for(int j=0; j<STRIP_LENGTH; j++)
            {
                wave = 0.0;
                for(int w=0; w<WAVE_COUNT; w++){
                    d = (pt_strip[index] - wave_start[w*2] + (pt_strip[index+1] - wave_start[w*2+1]) * tan(wave_dir[w])) * cos(wave_dir[w]);
                    if(gerstner_sort[w] == 1){
                        wave += wave_height[w] - gerstnerZ(wave_length[w], wave_height[w], d + wave_speed[w] * time, gerstner_pt_a);
                    }else{
                        wave += wave_height[w] - gerstnerZ(wave_length[w], wave_height[w], d + wave_speed[w] * time, gerstner_pt_b);
                    }
                }
                pt_strip[index+2] = START_Z + wave*HEIGHT_SCALE;
                index += 3;
            }
        }
        
        index = 0;
        for(int i=0; i<STRIP_COUNT; i++)
        {
            for(int j=0; j<STRIP_LENGTH; j++)
            {
                int p0 = index-STRIP_LENGTH*3, p1 = index+3, p2 = index+STRIP_LENGTH*3, p3 = index-3;
                float xa, ya, za, xb, yb, zb;
                if(i > 0){
                    if(j > 0){
                        xa = pt_strip[p0] - pt_strip[index];
                        ya = pt_strip[p0+1] - pt_strip[index+1];
                        za = pt_strip[p0+2] - pt_strip[index+2];
                        xb = pt_strip[p3] - pt_strip[index];
                        yb = pt_strip[p3+1] - pt_strip[index+1];
                        zb = pt_strip[p3+2] - pt_strip[index+2];
                        pt_normal[index] += ya*zb-yb*za;
                        pt_normal[index+1] += xb*za-xa*zb;
                        pt_normal[index+2] += xa*yb-xb*ya;
                    }
                    if(j < STRIP_LENGTH-1){
                        xa = pt_strip[p1] - pt_strip[index];
                        ya = pt_strip[p1+1] - pt_strip[index+1];
                        za = pt_strip[p1+2] - pt_strip[index+2];
                        xb = pt_strip[p0] - pt_strip[index];
                        yb = pt_strip[p0+1] - pt_strip[index+1];
                        zb = pt_strip[p0+2] - pt_strip[index+2];
                        pt_normal[index] += ya*zb-yb*za;
                        pt_normal[index+1] += xb*za-xa*zb;
                        pt_normal[index+2] += xa*yb-xb*ya;
                    }
                }
                if(i < STRIP_COUNT-1){
                    if(j > 0){
                        xa = pt_strip[p3] - pt_strip[index];
                        ya = pt_strip[p3+1] - pt_strip[index+1];
                        za = pt_strip[p3+2] - pt_strip[index+2];
                        xb = pt_strip[p2] - pt_strip[index];
                        yb = pt_strip[p2+1] - pt_strip[index+1];
                        zb = pt_strip[p2+2] - pt_strip[index+2];
                        pt_normal[index] += ya*zb-yb*za;
                        pt_normal[index+1] += xb*za-xa*zb;
                        pt_normal[index+2] += xa*yb-xb*ya;
                    }
                    if(j < STRIP_LENGTH-1){
                        xa = pt_strip[p2] - pt_strip[index];
                        ya = pt_strip[p2+1] - pt_strip[index+1];
                        za = pt_strip[p2+2] - pt_strip[index+2];
                        xb = pt_strip[p1] - pt_strip[index];
                        yb = pt_strip[p1+1] - pt_strip[index+1];
                        zb = pt_strip[p1+2] - pt_strip[index+2];
                        pt_normal[index] += ya*zb-yb*za;
                        pt_normal[index+1] += xb*za-xa*zb;
                        pt_normal[index+2] += xa*yb-xb*ya;
                    }
                }
                if(normalizeF(&pt_normal[index], &pt_normal[index], 3))
                    printf("%d\t%d\n", index/3/STRIP_LENGTH, (index/3)%STRIP_LENGTH);
                
                index += 3;
            }
        }
        
        //Calculate vertex_data[] according to pt_strip[], and normal_data[] according to pt_normal[]
        int pt;
        for(int c=0; c<(STRIP_COUNT-1); c++)
        {
            for(int l=0; l<2*STRIP_LENGTH; l++)
            {
                if(l%2 == 1){
                    pt = c*STRIP_LENGTH + l/2;
                }else{
                    pt = c*STRIP_LENGTH + l/2 + STRIP_LENGTH;
                }
                index = STRIP_LENGTH*2*c+l;
                for(int i=0; i<3; i++){
                    vertex_data[index*3+i] = pt_strip[pt*3+i];
                    normal_data[index*3+i] = pt_normal[pt*3+i];
                }
            }
        }
    }
    
    GLuint initTexture(const char *filename)
    {
        int width, height;
        void *pixels = read_tga(filename, &width, &height);
        GLuint texture;
        
        if (!pixels)
            return 0;
        
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels);
        
        free(pixels);
        return texture;
    }
};

#endif /* Water_h */
