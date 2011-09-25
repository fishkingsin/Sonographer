#ifndef _TEST_APP
#define _TEST_APP
//definetion
#define kNumTestNodes 11
//#define kNumTestNodes 24
#define PORT 1337
#define NUM_MSG_STRINGS 20
#define USE_RECORDING

//#define USE_STREAMING
#ifdef USE_STREAMING
    #include "FrameSender.h"

    //Globally known constants
    #define CAM1_WIDTH 320 
    #define CAM1_HEIGHT 240
    #define SERVER1_PORT 11999
    #define SERVER1_IP "127.0.0.1"
#else
    #define USE_OSC
    #define USE_WEBSERVER
#endif

//#define USE_AR

//#define KINECT_CONNECTED
//#define CAMERA_CONNECTED
//#define USE_FENSTER
//#define USE_HTTPSERVER

//#define TRACK_USER
//#define TRACK_DEPTH
#define NUM_PTS			640*480
#define NUM_PIXELS NUM_PTS*4
#define USE_VBO


#include "ofMain.h"

#include "ofxOpenNI.h"
#ifdef USE_HTTPSERVER
#include "ofxHTTPServer.h"
#endif
#ifdef USE_WEBSERVER
#include "ofxWebServer.h"
#endif
#ifdef USE_AR
#include "ofxARToolkitPlus.h"
#include "ofxQTKitVideoGrabber.h"
#endif

//#include "ofxOpenCV.h"
#include "ofxControlPanel.h"
#include "ofxAutoControlPanel.h"
//#include "ofxQtVideoSaver.h"
#include "ofxTween.h"


#ifdef USE_OSC
#include "ofxOsc.h"
#endif
#ifdef USE_FENSTER
#include "ofxFenster.h"
#endif
#ifdef USE_VBO
#include "MyVBO.h"
#include "ofxTimer.h"
#endif
using namespace xn;




class CameraProfile{
public:
    float distance;
    float rotationX;
    float rotationY;
    float radius;
    float stageX;
    float stageY;
    float duration;
    ofxTween            tweenDistance;
    ofxTween            tweenRotationX;
    ofxTween            tweenRotationY;
    ofxTween            tweenRadius;
    ofxTween            tweenStageX;
    ofxTween            tweenStageY;
    ofxTween            tweenDuration;
    ofxEasingLinear 	easinglinear;
};

class BoneTest : public ofNode {

public:

	ofQuaternion bindPoseOrientation;

	BoneTest()
    {
        baseDraw = NULL;
    }

	void customDraw() {

        ofEnableAlphaBlending();
		ofSetColor(255, 255, 255,120);

		glPushMatrix();
		glTranslatef(0, -40/2, 0);
		glScalef(*scaleX,*scaleY,*scaleZ);
		glColor4ub(*r, *g, *b,*a);
		ofBox(0,0,0,1);
        glPopMatrix();

        glPushMatrix();
        glColor4ub(255,255,255,255);
        glTranslatef(0, -40/2, 0);
        glScalef(*i_scale, *i_scale,1);
        if(baseDraw!=NULL)
            baseDraw->draw(-baseDraw->getWidth()/2,-baseDraw->getHeight()/2);
        glPopMatrix();
        ofDisableAlphaBlending();
	}
    float *scaleX,*scaleY,*scaleZ;
    float *r, *g, *b, *a;
    float *i_scale;

    ofImage *baseDraw;

};
#ifdef USE_FENSTER
#ifdef USE_WEBSERVER
class testApp : public ofBaseApp , public ofxWSRequestHandler, public ofxFensterListener{
#else
    class testApp : public ofBaseApp, public ofxFensterListener{
#endif

#else
#ifdef USE_WEBSERVER
        class testApp : public ofBaseApp , public ofxWSRequestHandler{
#else
            class testApp : public ofBaseApp{
#endif
#endif
            public:
                ~testApp()
                {
#ifdef USE_STREAMING
                    frameSender->stop();
#endif                    
                }
                void setup();
                void update();
                void draw();
                void keyPressed(int key);
                void dragEvent(ofDragInfo dragInfo);
                void draw3DScene();
                void drawPointCloud();
                void drawPointCloud(ofxUserGenerator * user_generator, int userID);
                void drawUser();
                void transformNode(int nodeNum, XnSkeletonJoint skelJoint);

#ifdef USE_HTTPSERVER
                ofxHTTPServer * server;
                void getRequest(ofxHTTPServerResponse & response);
                void postRequest(ofxHTTPServerResponse & response);
                bool imageServed;
                bool imageSaved;

                string postedImgName;
                string postedImgFile;
                string prevPostedImg;
#endif
#ifdef USE_WEBSERVER
                ofxWebServer server;
                void httpGet(string url);
#endif
                ofFbo fbo;
                
#ifdef USE_RECORDING
                ofxOpenNIContext *context;

#else
                ofxOpenNIContext context;
#endif
                ofxOpenNIContext *targetContext;
                ofxImageGenerator image;
                ofxDepthGenerator depth;
                #ifdef TRACK_USER
                ofxUserGenerator user;
#endif
#ifdef KINECT_CONNECTED
#ifdef TARGET_OSX // only working on Mac at the moment
                ofxHardwareDriver	hardware;
#endif
#endif
                BoneTest	testNodes[kNumTestNodes];
                //	ofLight		light;
                ofCamera	cam;

            private:

                bool bOrbit, bDebug, bCloud , bOsc , bCamera;
                float distance,radius,rotationX,rotationY;
                float r_x,r_y,r_z,_userX,_userY,_userScale;


#ifdef USE_AR

                //AR
                /* Size of the image */
                int width, height;

                /* Use either camera or a video file */
#ifdef CAMERA_CONNECTED
                //#ifdef TARGET_OSX
                //    ofxQTKitVideoGrabber vidGrabber;
                //#else
                ofVideoGrabber vidGrabber;
                //#endif
#else
                ofVideoPlayer vidPlayer;
#endif

                /* ARToolKitPlus class */
                ofxARToolkitPlus artk;
                int threshold;

                /* OpenCV images */
                ofxCvColorImage colorImage;
                ofxCvGrayscaleImage grayImage;
                ofxCvGrayscaleImage	grayThres;
#endif
                void setupAR();
                void updateAR();
                void drawAR();

                //    ofxXmlSettings configXml;
                ofxXmlSettings camreRecording;
                int pointCount;
                int lineCount;
                int lastTagNumber;
                bool bRecCamera;
                string xmlStructure;
                vector<ofPoint> dragPts;

                float elapsedTime;
                int appFrameCount;
                float appFrameRate;
                ofxAutoControlPanel gui;
                simpleLogger logger;
                void eventsIn(guiCallbackData & data);
                simpleFileLister inputList;
                void setupInput();
                void setupInput(int index);

#ifdef USE_OSC
                ofxOscReceiver	receiver;
                ofxOscSender	sender;
                string remoteHost;
                bool bSetupSender;
                int				current_msg_string;
                string		msg_strings[NUM_MSG_STRINGS];
                float			timers[NUM_MSG_STRINGS];
#endif
#ifdef USE_FENSTER
                void fensterDraw();
                void fensterUpdate();

                ofTexture tex;
                bool bTexInit;

#endif
                ofImage img;
#ifdef USE_VBO
                ofxTimer timer;
                myVbo vbo;
                bool bVBO;
                void startFadeIn();
                void startFadeOut();
                void reachedTime(ofEventArgs &arg)
                {
                    vbo.fadeIn();
                    ofRemoveListener(timer.TIMER_REACHED,this,&testApp::reachedTime);
                }
#endif
                ofImage head;

                ofSoundPlayer sndPlayer;

                ofxTween            tweenDistance;
                ofxTween            tweenRotationX;
                ofxTween            tweenRotationY;
                ofxTween            tweenRadius;
                ofxTween            tweenStageX;
                ofxTween            tweenStageY;
                ofxTween            tweenDuration;
                ofxEasingLinear 	easinglinear;
                #ifdef USE_STREAMING
                ofxCvColorImage frame;
                FrameSender* frameSender;
#endif
                int fileCount;
                ofImage exportImage;
                bool bExport;
            };

#endif
