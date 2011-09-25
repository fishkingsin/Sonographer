#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){

    ofAppGlutWindow window;
    #ifdef USE_STREAMING
	ofSetupOpenGL(&window, 320 , 240 , OF_WINDOW);			// <-------- setup the GL context
#else
    ofSetupOpenGL(&window, 1024 , 768 , OF_WINDOW);			// <-------- setup the GL context
#endif
    testApp* app = new testApp;
#ifdef USE_FENSTER
	ofxFenster fenster;
	fenster.init(app);
#endif
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( app);

}
