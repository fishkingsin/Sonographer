//
//  MyVbo.h
//  AROpenNI
//
//  Created by James' Mac on 21/06/2011.
//  Copyright 2011 yU+Co [lab] Hong Kong. All rights reserved.
//
#include "ofMain.h"

class myVbo
{
public:
    // the stars
	ofVbo		ptsVbo;
	ofVec3f		pts[NUM_PTS];
    ofVec3f		target[NUM_PTS];
    ofVec3f		vel[NUM_PTS];
	ofColor		color[NUM_PTS];
    float       particleTime[NUM_PTS];
    int curRow;
    int row;
    bool isFadeOut;
    bool isFadeIn;
    float smooth;
    void setup()
    {
        smooth = 0.9;
        curRow = 0;
        row = 480;
        isFadeOut = false;
        isFadeIn = false;
        
        for(int i=0; i<NUM_PTS; i++) {
            
            // randomly place the pts
            pts[i].x = ofRandom(-5000, 5000);
            pts[i].y = ofRandom(-5000, 5000);
            pts[i].z = ofRandom(-5000, 5000);
            
            particleTime[i] = ofRandom(1.0);
            
            vel[i] = 0;
            //setup vec
            setParticleTraget(i, 0,0,0,0,0,0);	
            
            // random give the stars a color
            color[i].set(1);
            setParticleTraget(i,ofRandom(-5000,5000),ofRandom(-5000,5000),ofRandom(-5000,500));
            
        }
        ptsVbo.setVertexData(pts, NUM_PTS, GL_STATIC_DRAW);
        ptsVbo.setColorData(color, NUM_PTS, GL_DYNAMIC_DRAW);
    }
    void setParticleTraget(int i, float x, float y, float z)
    {
        if(i>NUM_PTS)
            return;
        
        vel[i].x = (x-pts[i].x)*ofRandom(0.01f,0.09f);
        vel[i].y = (y-pts[i].y)*ofRandom(0.01f,0.09f);
        vel[i].z = (z-pts[i].z)*ofRandom(0.01f,0.09f);
    }
    void setParticleTraget(int i, float x, float y, float z , float r, float g, float b)
    {
        if(i>NUM_PTS)
            return;
        if(!isFadeOut)
        {
            target[i].x = x ; 
            target[i].y = y ; 
            target[i].z = z ; 
            if(r > 1)
            {
                r = r/255.0f;
                g = g/255.0f;
                b = b/255.0f;
            }
            
            color[i].r   = r;
            color[i].g   = g;
            color[i].b   = b;
        }
        else
        {
            color[i].r   = 1.0;
            color[i].g   = 1.0;
            color[i].b   = 1.0;
        }
    }
    void update()
    {
        
        // fade the stars out then fade them
		// back in, also give a new random color
        float div = 250.0;
		for(int i=0; i<NUM_PTS; i++) 
        {
            if(isFadeOut)
            {
                
                pts[i] += vel[i];
//                if(i%4==0)
//                {
//                    float t = ofGetElapsedTimef() * particleTime[i];
//                    vel[i].x   += ofSignedNoise(pts[i].z/div, t, pts[i].y/div);
//                    vel[i].y   += ofSignedNoise(pts[i].x/div, t, pts[i].z/div) + 0.9f;
//                }
                vel[i]   *= 0.89f;
//                vel[i].y   *= 0.89f;
//                vel[i].z   *= 0.89f;
            }
            else if(isFadeIn)
            {
                pts[i].x += (target[i].x-pts[i].x)*smooth;
                pts[i].y += (target[i].y-pts[i].y)*smooth;
                pts[i].z += (target[i].z-pts[i].z)*smooth;
            }
            else
            {
                pts[i].x = target[i].x;
                pts[i].y = target[i].y;
                pts[i].z = target[i].z;
            }
		}
    }
    void fadeOut()
    {
        for(int i=0; i<NUM_PTS; i++) {
            color[i].set(1,1,1,ofRandomf());
            setParticleTraget(i,ofRandom(-5000,5000),ofRandom(-5000,5000),ofRandom(-5000,500));
        }
        isFadeIn = false;
        isFadeOut = true;
    }
    void fadeIn()
    {
        for(int i=0; i<NUM_PTS; i++) {
            color[i].set(1,1,1,1);
        }
        isFadeIn = true;
        isFadeOut = false;
    }
    void stopFade()
    {
        isFadeIn = false;
        isFadeOut = false;
    }
    void draw()
    {
        // draw the star
        //update the vbo
        ofEnableAlphaBlending();
		ptsVbo.setVertexData(pts, NUM_PTS, GL_DYNAMIC_DRAW);
		ptsVbo.setColorData(color,NUM_PTS, GL_DYNAMIC_DRAW);
		ptsVbo.draw(GL_POINTS, 0, NUM_PTS);
        ofDisableAlphaBlending();
		
    }
};
