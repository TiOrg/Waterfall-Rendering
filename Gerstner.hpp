//
//  Gerstner.hpp
//  waterfall
//
//  Created by simon on 2018/12/7.
//  Copyright © 2018 simon. All rights reserved.
//

#ifndef Gerstner_h
#define Gerstner_h


static void initWave(void)
{
    //Initialize values{}
    values.time = 0.0;
    for(int w=0; w<WAVE_COUNT; w++)
    {
        values.wave_length[w] = wave_para[w][0];
        values.wave_height[w] = wave_para[w][1];
        values.wave_dir[w] = wave_para[w][2];
        values.wave_speed[w] = wave_para[w][3];
        values.wave_start[w*2] = wave_para[w][4];
        values.wave_start[w*2+1] = wave_para[w][5];
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

static void calcuWave(void)
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
                d = (pt_strip[index] - values.wave_start[w*2] + (pt_strip[index+1] - values.wave_start[w*2+1]) * tan(values.wave_dir[w])) * cos(values.wave_dir[w]);
                if(gerstner_sort[w] == 1){
                    wave += values.wave_height[w] - gerstnerZ(values.wave_length[w], values.wave_height[w], d + values.wave_speed[w] * values.time, gerstner_pt_a);
                }else{
                    wave += values.wave_height[w] - gerstnerZ(values.wave_length[w], values.wave_height[w], d + values.wave_speed[w] * values.time, gerstner_pt_b);
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
                    xa = pt_strip[p0] - pt_strip[index], ya = pt_strip[p0+1] - pt_strip[index+1], za = pt_strip[p0+2] - pt_strip[index+2];
                    xb = pt_strip[p3] - pt_strip[index], yb = pt_strip[p3+1] - pt_strip[index+1], zb = pt_strip[p3+2] - pt_strip[index+2];
                    pt_normal[index] += ya*zb-yb*za;
                    pt_normal[index+1] += xb*za-xa*zb;
                    pt_normal[index+2] += xa*yb-xb*ya;
                }
                if(j < STRIP_LENGTH-1){
                    xa = pt_strip[p1] - pt_strip[index], ya = pt_strip[p1+1] - pt_strip[index+1], za = pt_strip[p1+2] - pt_strip[index+2];
                    xb = pt_strip[p0] - pt_strip[index], yb = pt_strip[p0+1] - pt_strip[index+1], zb = pt_strip[p0+2] - pt_strip[index+2];
                    pt_normal[index] += ya*zb-yb*za;
                    pt_normal[index+1] += xb*za-xa*zb;
                    pt_normal[index+2] += xa*yb-xb*ya;
                }
            }
            if(i < STRIP_COUNT-1){
                if(j > 0){
                    xa = pt_strip[p3] - pt_strip[index], ya = pt_strip[p3+1] - pt_strip[index+1], za = pt_strip[p3+2] - pt_strip[index+2];
                    xb = pt_strip[p2] - pt_strip[index], yb = pt_strip[p2+1] - pt_strip[index+1], zb = pt_strip[p2+2] - pt_strip[index+2];
                    pt_normal[index] += ya*zb-yb*za;
                    pt_normal[index+1] += xb*za-xa*zb;
                    pt_normal[index+2] += xa*yb-xb*ya;
                }
                if(j < STRIP_LENGTH-1){
                    xa = pt_strip[p2] - pt_strip[index], ya = pt_strip[p2+1] - pt_strip[index+1], za = pt_strip[p2+2] - pt_strip[index+2];
                    xb = pt_strip[p1] - pt_strip[index], yb = pt_strip[p1+1] - pt_strip[index+1], zb = pt_strip[p1+2] - pt_strip[index+2];
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

#endif /* Gerstner_h */
