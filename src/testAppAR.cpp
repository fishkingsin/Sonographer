//
//  testAppAR.cpp
//  AROpenNI
//
//  Created by James' Mac on 20/06/2011.
//  Copyright 2011 yU+Co [lab] Hong Kong. All rights reserved.
//

#include "testApp.h"
void testApp::setupAR()
{
    
#ifdef USE_AR
    width = 640;
    height = 480;
    
    // Print the markers from the "AllBchThinMarkers.png" file in the data folder
#ifdef CAMERA_CONNECTED
    vidGrabber.initGrabber(width, height);
#else
    vidPlayer.loadMovie("marker.mov");
    vidPlayer.play();	
#endif
    
    colorImage.allocate(width, height);
    grayImage.allocate(width, height);
    grayThres.allocate(width, height);
    
    // This uses the default camera calibration and marker file
    
#ifdef CAMERA_CONNECTED
    artk.setup(width, height,true);
#else	
    artk.setup(width, height);
#endif
    
    // The camera calibration file can be created using GML:
    // http://graphics.cs.msu.ru/en/science/research/calibration/cpp
    // and these instructions:
    // http://studierstube.icg.tu-graz.ac.at/doc/pdf/Stb_CamCal.pdf
    // This only needs to be done once and will aid with detection
    // for the specific camera you are using
    // Put that file in the data folder and then call setup like so:
    // artk.setup(width, height, "myCamParamFile.cal", "markerboard_480-499.cfg");
    
    // Set the threshold
    // ARTK+ does the thresholding for us
    // We also do it in OpenCV so we can see what it looks like for debugging
    threshold = 85;
    artk.setThreshold(threshold);

    gui.setWhichPanel("ARToolkitPlus");
    gui.setWhichColumn(0);
    gui.addDrawableRect("video", &colorImage, 200, 150);
    gui.addDrawableRect("grayThres", &grayThres, 200, 150);
    gui.addSlider("ARThreshold","threshold",threshold,1,255,true);
#endif
}
//--------------------------------------------------------------
void testApp::updateAR(){
#ifdef USE_AR
    artk.setThreshold(gui.getValueI("threshold"));

#ifdef CAMERA_CONNECTED
    vidGrabber.grabFrame();
    bool bNewFrame = vidGrabber.isFrameNew();
#else
    vidPlayer.idleMovie();
    bool bNewFrame = vidPlayer.isFrameNew();
#endif
    
    if(bNewFrame) {
        
#ifdef CAMERA_CONNECTED
        colorImage.setFromPixels(vidGrabber.getPixels(), width, height);
#else
        colorImage.setFromPixels(vidPlayer.getPixels(), width, height);
#endif
        
        // convert our camera image to grayscale
        grayImage = colorImage;
        // apply a threshold so we can see what is going on
        grayThres = grayImage;
        grayThres.threshold(threshold);
        
        // Pass in the new image pixels to artk
        artk.update(grayImage.getPixels());
        
    }
#endif
}

//--------------------------------------------------------------
void testApp::drawAR(){
#ifdef USE_AR
    // Main image
    ofSetHexColor(0xffffff);
    grayImage.draw(320*2, 0, 320, 240);
    ofSetHexColor(0x666666);	
    ofDrawBitmapString(ofToString(artk.getNumDetectedMarkers()) + " marker(s) found", 10, 20);
    
    // Threshold image
    ofSetHexColor(0xffffff);
    grayThres.draw(320*3, 0, 320, 240);
    ofSetHexColor(0x666666);	
    ofDrawBitmapString("Threshold: " + ofToString(threshold), 650, 20);
    ofDrawBitmapString("Use the Up/Down keys to adjust the threshold", 650, 40);
    
    // ARTK draw
    // An easy was to see what is going on
    // Draws the marker location and id number
    
    artk.draw(320*3, 0);
    
    
    // ARTK 2D stuff
    // See if marker ID '0' was detected
    // and draw blue corners on that marker only
    int myIndex = artk.getMarkerIndex(0);
    if(myIndex >= 0) {	
        // Get the corners
        vector<ofPoint> corners;
        artk.getDetectedMarkerBorderCorners(myIndex, corners);
        // Can also get the center like this:
        // ofPoint center = artk.getDetectedMarkerCenter(myIndex);
        ofSetHexColor(0x0000ff);
        for(int i=0;i<corners.size();i++) {
            ofCircle(corners[i].x, corners[i].y, 10);
        }
    }
    
    // ARTK 3D stuff
    // First apply the projection matrix once
    artk.applyProjectionMatrix(ofGetWidth(),ofGetHeight());
    // Find out how many markers have been detected
    int numDetected = artk.getNumDetectedMarkers();
    ofEnableAlphaBlending();
    // Draw for each marker discovered
    for(int i=0; i<numDetected; i++) {
        
        // Set the matrix to the perspective of this marker
        // The origin is in the middle of the marker	
        artk.applyModelMatrix(i);		
        
        // Draw a line from the center out
        ofNoFill();
        ofSetLineWidth(5);
        ofSetHexColor(0xffffff);
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0); 
        glVertex3f(0, 0, 50);
        glEnd();
        
        // Draw a stack of rectangles by offseting on the z axis
        ofFill();
        ofSetColor(255, 255, 0, 50);	
        for(int i=0; i<10; i++) {		
            ofRect(-25, -25, 50, 50);
            ofTranslate(0, 0, i*1);
        }
    }
#endif
}
//--------------------------------------------------------------
#ifdef USE_VBO
void testApp::startFadeIn()
{
    
}
//--------------------------------------------------------------
void testApp::startFadeOut()
{
    
}
#endif