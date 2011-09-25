#include "testApp.h"

//#include "ofxControlPanel.h"
//static ofxControlPanel *gui;


float scaleX,scaleY,scaleZ;
float imageS;
float colorR,colorG,colorB,colorA;
static int curPlaymode=0;
float stageX,stageY;
int currentSelected = 0;
int counter=0;
bool bRecord = false;
bool bRecord2 = false;
int convertSec = 0;
int convertMin = 0;
float smoothMouseX = 0;
float smoothMouseY = 0;
float mouseSmooth = 0.5000000f;
string target_path;
string deviceID;
guiTypeTextInput *textinput;
guiTypeTextDropDown *textDropDown;

#ifdef USE_WEBSERVER
ofColor color;
void testApp::httpGet(string url)
{
    string colorString = getRequestParameter("color");
    if (colorString=="red")
    {
        color.r = 255;
        color.g = 0;
        color.b = 0;
    }
    else if (colorString=="green")
    {
        color.r = 0;
        color.g = 255;
        color.b = 0;
    }
    else if (colorString=="blue")
    {
        color.r = 0;
        color.g = 0;
        color.b = 255;
    }
    httpResponse("Color value: "
                 + ofToString(color.r)
                 + " " + ofToString(color.g)
                 + " " + ofToString(color.b));
}
#endif
//--------------------------------------------------------------
void testApp::setup()
{
    exportImage.allocate(640,480,OF_IMAGE_COLOR_ALPHA);
    fileCount = 0;
    bRecCamera = false;
    //http server
#ifdef USE_HTTPSERVER
    server = ofxHTTPServer::getServer(); // get the instance of the server
    server->setServerRoot("www");		 // folder with files to be served
    server->setUploadDir("upload");		 // folder to save uploaded files
    server->setCallbackExtension("of");	 // extension of urls that aren't files but will generate a post or get event
    ofAddListener(server->getEvent,this,&testApp::getRequest);
    ofAddListener(server->postEvent,this,&testApp::postRequest);
    server->start(8888);
#endif
#ifdef USE_WEBSERVER
    server.start("httpdocs");
    server.addHandler(this, "actions/*");
#endif
    keyPressed('S');
    ofxXmlSettings xml;
    xml.loadFile("config.xml");
    
    deviceID = xml.getValue("SETTINGS:DEVICE_ID","James-iPhone");
    target_path = xml.getValue("SETTINGS:TARGET_PATH","recording/");
    if(xml.getValue("SETTINGS:FULLSCREEN",0))
    {
        ofSetFullscreen(true);
        ofHideCursor();
    }
#ifdef USE_STREAMING
    ofSetWindowShape(CAM1_WIDTH,CAM1_HEIGHT);
#else
    ofSetWindowShape(xml.getValue("SETTINGS:STAGE_WIDTH",1024),xml.getValue("SETTINGS:STAGE_HEIGHT",768));
#endif
    head.loadImage("images/head.png");
    ofShowCursor();
    ofSetFrameRate(60);
    
#ifdef USE_FENSTER
    fenster->setFPS(60);
    fenster->setBounds(xml.getValue("SETTINGS:FENSTER:X",0),
                       xml.getValue("SETTINGS:FENSTER:Y",0),
                       xml.getValue("SETTINGS:FENSTER:WIDTH",1024),
                       xml.getValue("SETTINGS:FENSTER:HEIGHT",768));
    if (xml.getValue("SETTINGS:FENSTER:FULLSCREEN",0))fenster->toggleFullscreen();
    bTexInit = false;;
#endif
    //    ofFbo::Settings s;
    //	s.width				= 1024;
    //	s.height			= 768;
    //	s.numColorbuffers	= GL_RGBA;
    //	s.numSamples		= 0;
    fbo.setup(320,480,GL_RGBA,0);
    //fbo2.setup(ofGetWidth(),ofGetHeight(),GL_RGB,0);
    bCloud = true;
    
    //    gui = NULL;
    ofEnableSmoothing();
    
    ofSetFrameRate(30);
    ofBackground(1,1,1);
    sndPlayer.loadSound("sounds/Aria.aif");
    sndPlayer.play();
    sndPlayer.setLoop(true);
#ifdef USE_RECORDING
    int num = inputList.listDir("oni/");
    //    ofDirectory dir;
    //    dir.allowExt("oni");
    //    int num = dir.listDir("oni/");
    context = new ofxOpenNIContext[num];
    for (int i = 0 ; i < num ; i++)
    {
        context[i].shutdown();
        context[i].setupUsingRecording(inputList.getPath(i));
        context[i].toggleRegisterViewport();
        context[i].toggleMirror();
        
        //        context[i].setupUsingRecording(ofToDataPath("oni/kinectRecord162011214033.oni"));
    }
    //    targetContext = &context[0];
#else
    
    //context.setup();//UsingXMLFile();
    context.setup();	// all nodes created by code -> NOT using the xml config file at all
    targetContext = &context;
#endif
    
    
    
    
#ifdef KINECT_CONNECTED
#ifdef TARGET_OSX // only working on Mac at the moment
    hardware.setup();				// libusb direct control of motor, LED and accelerometers
    hardware.setLedOption(LED_OFF); // turn off the led just for yacks (or for live installation/performances ;-)
#endif
#endif
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofDisableArbTex();
    ofSetVerticalSync(true);
    
    bOrbit = bDebug = false;
    distance = 1500;
    rotationX=rotationY = 0;
    
    cam.setFov(57.8);
    cam.setFarClip(20000.f);
    cam.setNearClip(10.f);
    
    scaleX=1;
    scaleY=30;
    scaleZ=1;
    
    
    
#ifdef  USE_OSC
    receiver.setup( PORT );
    
    current_msg_string = 0;
    bSetupSender = false;
#endif
    
    
    for (int i=0; i<kNumTestNodes; i++)
    {
        
        testNodes[i].setPosition(ofRandomf()*10-5, ofRandomf()*10-5, ofRandomf()*10-5);
        testNodes[i].setScale(10);
        
        // setup a traditional humanoid bind pose
        
        // LEFT ARM
        if (i==2||i==4)
        {
            testNodes[i].bindPoseOrientation.makeRotate(-90.0, ofVec3f(0.0f, 0.0f, 1.0f));
            // RIGHT ARM
        }
        else if (i==3||i==5)
        {
            testNodes[i].bindPoseOrientation.makeRotate(90.0, ofVec3f(0.0f, 0.0f, 1.0f));
            // NECK
        }
        else if (i==10)
        {
            testNodes[i].bindPoseOrientation.makeRotate(180.0, ofVec3f(0.0f, 0.0f, 1.0f));
            testNodes[i].baseDraw = &head;
        }
        else
        {
            testNodes[i].bindPoseOrientation.set(0, 0, 0, 1);
        }
        testNodes[i].scaleX = &scaleX;
        testNodes[i].scaleY = &scaleY;
        testNodes[i].scaleZ = &scaleZ;
        testNodes[i].r = &colorR;
        testNodes[i].g = &colorG;
        testNodes[i].b = &colorB;
        testNodes[i].a = &colorA;
        testNodes[i].i_scale = &imageS;
    }
    
    
    ofxControlPanel::setBackgroundColor(simpleColor(30, 30, 60, 200));
    ofxControlPanel::setTextColor(simpleColor(240, 50, 50, 255));
    
    gui.loadFont("MONACO.TTF", 8);
    gui.setup("AM2", 0, 0,ofGetWidth()*(2.0f/3.0f), ofGetHeight());
    
    gui.addPanel("OpenNi",3, false);
    
    ofxControlPanel::setBackgroundColor(simpleColor(60, 30, 30, 200));
    gui.addPanel("ARToolkitPlus", 2, false);
    
    ofxControlPanel::setBackgroundColor(simpleColor(70, 70, 30, 200));
    gui.addPanel("other",2, false);
    
    ofxControlPanel::setBackgroundColor(simpleColor(30, 30, 30, 200));
    
    //--------- PANEL 1
    gui.setWhichPanel(0);
    
    gui.setWhichColumn(0);
    //some dummy vars we will update to show the variable lister object
    elapsedTime		= ofGetElapsedTimef();
    appFrameCount	= ofGetFrameNum();
    appFrameRate	= ofGetFrameRate();
    
    gui.enableIgnoreLayoutFlag();
    
    textinput = gui.addTextInput("DeviceID", deviceID, 200);
    
    
    gui.addFileLister("input", &inputList, 200, 100);
    
    gui.disableIgnoreLayoutFlag();
    vector <guiVariablePointer> vars;
    vars.push_back( guiVariablePointer("ellapsed time", &elapsedTime, GUI_VAR_FLOAT, 2) );
    vars.push_back( guiVariablePointer("ellapsed frames", &appFrameCount, GUI_VAR_INT) );
    vars.push_back( guiVariablePointer("app fps", &appFrameRate, GUI_VAR_FLOAT, 2) );
    vars.push_back( guiVariablePointer("mouse x", &mouseX, GUI_VAR_INT) );
    vars.push_back( guiVariablePointer("mouse y", &mouseY, GUI_VAR_INT) );
    
    gui.addVariableLister("app vars", vars);
    gui.addChartPlotter("acc chart", guiStatVarPointer("osc accX", &r_x, GUI_VAR_FLOAT, true, 2), 200, 100, 100, 0, 1);
    gui.addChartPlotter("acc chart", guiStatVarPointer("osc accY", &r_y, GUI_VAR_FLOAT, true, 2), 200, 100, 100, 0, 1);
    gui.addChartPlotter("acc chart", guiStatVarPointer("osc accZ", &r_z, GUI_VAR_FLOAT, true, 2), 200, 100, 100, -1, 1);
    //    gui.setWhichPanel("other");
    //    gui.setWhichColumn(0);
    gui.setWhichPanel("OpenNi");
    gui.setWhichColumn(1);
    gui.addToggle("bOrbit", "TOGGLE_ORBIT", 0);
    gui.addToggle("bOsc", "TOGGLE_OSC", 0);
    gui.addToggle("bCamera", "TOGGLE_CAMERA", 0);
    
    vector<string> play_mode;
    play_mode.push_back("PointCloud");
    play_mode.push_back("Skeleton");
    play_mode.push_back("All");
    gui.addMultiToggle("play_mode", "play_mode", 0, play_mode);
    for (int i=0; i<fbo.getNumTextures(); i++)
    {
        gui.addDrawableRect("FBO Texture"+ofToString(i), &fbo.getTexture(i), 200, 300);
    }
    
    gui.setWhichColumn(2);
    gui.addSlider("stage x", "STAGE_X",stageX, -3000,3000,false);
    gui.addSlider("stage y", "STAGE_Y",stageY, -3000,3000,false);
    gui.addSlider("camera distance", "CAMERA_DISTANCE",distance, -5000,5000,false);
    gui.addSlider("camera radius", "CAMERA_RADIUS",radius, -5000,5000,false);
    gui.addSlider("camera rotationX", "CAMERA_ROTATION_X",180, 0, 360,false);
    gui.addSlider("camera rotationY", "CAMERA_ROTATION_Y",180, -180, 180,false);
    gui.addSlider("mouseSmooth","MOUSE_SMOOTH",mouseSmooth,0.000000f,0.100000f,false);
    gui.addToggle("toggle vbo", "TOGGLE_VBO", 0);
    
    gui.addToggle("toggle camera profile", "TOGGLE_PROFILE", 0);
    vector <string> boxNames;
    boxNames.push_back("Profile1");
    boxNames.push_back("Profile2");
    boxNames.push_back("Profile3");
    boxNames.push_back("Profile4");
    boxNames.push_back("Profile5");
    boxNames.push_back("Profile6");
    boxNames.push_back("Profile7");
    boxNames.push_back("Profile8");
    boxNames.push_back("Profile9");
    boxNames.push_back("Profile10");
    textDropDown = gui.addTextDropDown("camera pofile", "CAMERA_PROFILE", currentSelected, boxNames);
    
    
    gui.setWhichPanel("other");
    gui.addDrawableRect("Head Texture",&head,100,200);
    gui.addSlider("user x", "USER_X",_userX, 0,ofGetWidth(),true);
    gui.addSlider("user y", "USER_Y",_userY, 0,ofGetHeight(),true);
    gui.addSlider("user scale", "USER_SCALE",_userScale, 0,5,true);
    
    gui.addSlider("scaleX", "scaleX",scaleX, 0,200,false);
    gui.addSlider("scaleY", "scaleY",scaleY, 0,200,false);
    gui.addSlider("scaleZ", "scaleZ",scaleZ, 0,200,false);
    gui.addSlider("colorR", "colorR",colorR, 0,255,false);
    gui.addSlider("colorG", "colorG",colorG, 0,255,true);
    gui.addSlider("colorB", "colorB",colorB, 0,255,true);
    gui.addSlider("colorA", "colorA",colorA, 0,255,true);
    gui.addSlider("imageS", "imageS",imageS, 0,10.0f,false);
    gui.enableIgnoreLayoutFlag();
    gui.addLogger("events logger", &logger, 400, 200);
    gui.disableIgnoreLayoutFlag();
    
    setupAR();
    
    gui.setDraw(false);
#ifdef USE_VBO
    vbo.setup();
    gui.setWhichPanel("OpenNi");
    gui.setWhichColumn(2);
    gui.addSlider("vbo smooth", "vbo_smooth",vbo.smooth, 0,1,false);
    
#endif
    
    
    
    
    
    //  -- SPECIFIC EVENTS -- this approach creates an event group and only sends you events for the elements you describe.
    //	vector <string> list;
    //	list.push_back("FIELD_DRAW_SCALE");
    //	list.push_back("DIFF_MODE");
    //	gui.createEventGroup("TEST_GROUP", list);
    //	ofAddListener(gui.getEventGroup("TEST_GROUP"), this, &testApp::eventsIn);
    
    //  -- PANEL EVENTS -- this approach gives you back an ofEvent for only the events from panel 0
    //	ofAddListener(gui.getEventsForPanel(0), this, &testApp::eventsIn);
    
    //  -- this gives you back an ofEvent for all events in this control panel object
    ofAddListener(gui.guiEvent, this, &testApp::eventsIn);
    gui.loadSettings("control.xml");
    
    
#ifdef USE_RECORDING
    setupInput();
#else
    depth.setup(targetContext);
    image.setup(targetContext);
    
#ifdef TRACK_USER
    user.setup(targetContext);
    user.setSmoothing(0.1);				// built in openni skeleton smoothing...
    //	user.setUseMaskPixels(isMasking);
    user.setUseCloudPoints(bCloud);
#endif    
#endif
    
    //tween
    tweenDistance.setParameters(0,easinglinear,ofxTween::easeOut, distance,distance,0 ,0 );
    tweenRotationX.setParameters(0,easinglinear,ofxTween::easeOut, rotationX ,rotationX ,0 ,0 );
    tweenRotationY.setParameters(0,easinglinear,ofxTween::easeOut, rotationY, rotationY , 0 ,0 );
    tweenRadius.setParameters(0,easinglinear,ofxTween::easeOut, radius , radius , 0 ,0 );
    tweenStageX.setParameters(0,easinglinear,ofxTween::easeOut, stageX , stageX , 0 ,0 );
    tweenStageY.setParameters(0,easinglinear,ofxTween::easeOut, stageY ,stageY , 0 ,0 );
#ifdef USE_STREAMING
    frame.allocate(CAM1_WIDTH, CAM1_HEIGHT);
    //setup server
	frameSender = new FrameSender(SERVER1_PORT, CAM1_WIDTH, CAM1_HEIGHT);
	frameSender->start();
#endif
}
//this captures all our control panel events - unless its setup differently in testApp::setup
//--------------------------------------------------------------
void testApp::eventsIn(guiCallbackData & data)
{
    
    //lets send all events to our logger
    if ( !data.isElement( "events logger" ) )
    {
        string logStr = data.getXmlName();
        
        for (int k = 0; k < data.getNumValues(); k++)
        {
            logStr += " - " + data.getString(k);
        }
        
        logger.log(OF_LOG_NOTICE, "event - %s", logStr.c_str());
    }
    
    // print to terminal if you want to
    //this code prints out the name of the events coming in and all the variables passed
    printf("testApp::eventsIn - name is %s - \n", data.getXmlName().c_str());
    if ( data.getDisplayName() != "" )
    {
        printf(" element name is %s \n", data.getDisplayName().c_str());
    }
    for (int k = 0; k < data.getNumValues(); k++)
    {
        if ( data.getType(k) == CB_VALUE_FLOAT )
        {
            printf("%i float  value = %f \n", k, data.getFloat(k));
        }
        else if ( data.getType(k) == CB_VALUE_INT )
        {
            printf("%i int    value = %i \n", k, data.getInt(k));
        }
        else if ( data.getType(k) == CB_VALUE_STRING )
        {
            printf("%i string value = %s \n", k, data.getString(k).c_str());
        }
    }
    
    printf("\n");
}

//--------------------------------------------------------------
void testApp::update()
{
    if (ofGetFrameNum()%30==0)
    {
        convertSec++;
        if (convertSec%60==0)
        {
            convertSec%=60;
            convertMin++;
        }
    }
    
    int selected = gui.getValueI("CAMERA_PROFILE");
    if (selected!=currentSelected)
    {
        currentSelected = selected;
        gui.setValueB("TOGGLE_PROFILE",true);
        //        float t_x = cam.getPosition().x;
        //        float t_x = cam.getPosition().y;
        //        float t_x = cam.getRotation().x;
        //        float t_x = cam.getRotation().x;
        //        unsigned delay = 0;
        unsigned duration = 1000;
        //        tween.setParameters(1,easingLinear,ofxTween::easeOut,0,ofGetWidth()-100,duration,delay);
        
        tweenDistance.setParameters(0,easinglinear,ofxTween::easeOut, distance,distance + ofRandom(-10,10), duration ,0 );
        tweenRotationX.setParameters(0,easinglinear,ofxTween::easeOut, rotationX ,rotationX + ofRandom(-10,10), duration ,0 );
        tweenRotationY.setParameters(0,easinglinear,ofxTween::easeOut, rotationY, rotationY + ofRandom(-10,10) , duration ,0 );
        tweenRadius.setParameters(0,easinglinear,ofxTween::easeOut, radius , radius  + ofRandom(-10,10), duration ,0 );
        tweenStageX.setParameters(0,easinglinear,ofxTween::easeOut, stageX , stageX  + ofRandom(-10,10), duration ,0 );
        tweenStageY.setParameters(0,easinglinear,ofxTween::easeOut, stageY ,stageY  + ofRandom(-10,10), duration ,0 );
        
        switch (currentSelected)
        {
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:
                break;
            case 9:
                break;
            case 10:
                break;
        }
    }
#ifdef USE_RECORDING
    bool reload = inputList.selectedHasChanged();
    if (reload)
    {
        setupInput();
        inputList.clearChangedFlag();
    }
#endif
    updateAR();
    if (gui.getValueB("TOGGLE_PROFILE"))
    {
        gui.setValueF("CAMERA_DISTANCE",tweenDistance.update());
        gui.setValueF("CAMERA_ROTATION_X",tweenRotationX.update());
        gui.setValueF("CAMERA_ROTATION_Y",tweenRotationY.update());
        gui.setValueF("CAMERA_RADIUS",tweenRadius.update());
        gui.setValueF("STAGE_X",tweenStageX.update());
        gui.setValueF("STAGE_Y",tweenStageY.update());
        
    }
    //cam.setFov(gui.getValueF("FVO"));
    if (textinput->valueTextHasChanged())deviceID = textinput->getValueText();
    bOrbit = gui.getValueB("TOGGLE_ORBIT");
    bOsc = gui.getValueB("TOGGLE_OSC");
    distance = gui.getValueF("CAMERA_DISTANCE");
    rotationX = gui.getValueF("CAMERA_ROTATION_X");
    rotationY = gui.getValueF("CAMERA_ROTATION_Y");
    radius = gui.getValueF("CAMERA_RADIUS");
    stageX = gui.getValueF("STAGE_X");
    stageY = gui.getValueF("STAGE_Y");
    mouseSmooth = gui.getValueF("MOUSE_SMOOTH");
    
    bCamera = gui.getValueB("TOGGLE_CAMERA");
    bVBO = gui.getValueB("TOGGLE_VBO");
    curPlaymode = gui.getValueI("play_mode");
    _userX = gui.getValueI("USER_X");
    _userY = gui.getValueI("USER_Y");
    _userScale = gui.getValueI("USER_SCALE");
    scaleX = gui.getValueI("scaleX");
    scaleY = gui.getValueI("scaleY");
    scaleZ = gui.getValueI("scaleZ");
    colorR = gui.getValueI("colorR");
    colorG = gui.getValueI("colorG");
    colorB = gui.getValueI("colorB");
    colorA = gui.getValueI("colorA");
    imageS = gui.getValueF("imageS");
    elapsedTime		= ofGetElapsedTimef();
    appFrameCount	= ofGetFrameNum();
    appFrameRate	= ofGetFrameRate();
#ifdef USE_VBO
    vbo.smooth = gui.getValueF("vbo_smooth");
    gui.setValueF("vbo_smooth",vbo.smooth+=0.1f);
    vbo.update();
#endif
#ifdef KINECT_CONNECTED
#ifdef TARGET_OSX // only working on Mac at the moment
    hardware.update();
#endif
#endif
    if (targetContext!=NULL)targetContext->update();
    depth.update();
    image.update();
#ifdef TRACK_USER
    user.update();
#endif
    //	cam.setScale(camScale);
    if(bRecord)
    {
        if(pointCount<dragPts.size()-1 && dragPts.size()>0)
        {
            smoothMouseX += (dragPts[pointCount].x-smoothMouseX)*mouseSmooth;
            smoothMouseY += (dragPts[pointCount].y-smoothMouseY)*mouseSmooth;
            pointCount++;
        }else if(pointCount>=dragPts.size()-1)
        {
            //bRecord = false;
            //std::exit(0);
            smoothMouseX += (mouseX-smoothMouseX)*mouseSmooth;
            smoothMouseY += (mouseY-smoothMouseY)*mouseSmooth;
        }
    }
    else
    {
        smoothMouseX += (mouseX-smoothMouseX)*mouseSmooth;
        smoothMouseY += (mouseY-smoothMouseY)*mouseSmooth;
    }
    if (bOrbit && !bOsc)
    {
        
        
        //		cam.orbit(smoothMouseX*360, (float)mouseY/ofGetHeight()*360-180, radius, ofVec3f(0,0,distance));
        cam.orbit((float)(smoothMouseX/ofGetWidth())*360, (float)(smoothMouseY/ofGetHeight())*360-180, radius, ofVec3f(0,0,distance));
    }
    else if (bOrbit && bOsc)
    {
        cam.orbit(r_x*360, r_y*360-180, radius, ofVec3f(0,0,distance));
    }
    
    else if (!bOrbit && !bOsc)
    {
        //        cam.setPosition(ofVec3f(0,0,distance));
        cam.orbit(rotationX,rotationY, radius, ofVec3f(stageX,stageY,distance));
    }
#ifdef TRACK_USER
    if (user.getNumberOfTrackedUsers()>0)
    {
        
        transformNode(0, XN_SKEL_TORSO);
        transformNode(1, XN_SKEL_WAIST);
        transformNode(2, XN_SKEL_LEFT_SHOULDER);
        transformNode(3, XN_SKEL_RIGHT_SHOULDER);
        transformNode(4, XN_SKEL_LEFT_ELBOW);
        transformNode(5, XN_SKEL_RIGHT_ELBOW);
        transformNode(6, XN_SKEL_LEFT_HIP);
        transformNode(7, XN_SKEL_RIGHT_HIP);
        transformNode(8, XN_SKEL_LEFT_KNEE);
        transformNode(9, XN_SKEL_RIGHT_KNEE);
        transformNode(10, XN_SKEL_NECK);
        //james added
        //        transformNode(11, XN_SKEL_HEAD);
        //
        //        transformNode(12, XN_SKEL_LEFT_COLLAR);
        //        transformNode(13, XN_SKEL_LEFT_WRIST);
        //        transformNode(14, XN_SKEL_LEFT_HAND);
        //        transformNode(15, XN_SKEL_LEFT_FINGERTIP);
        //
        //        transformNode(16, XN_SKEL_RIGHT_COLLAR);
        //        transformNode(17, XN_SKEL_RIGHT_WRIST);
        //        transformNode(18, XN_SKEL_RIGHT_HAND);
        //        transformNode(19, XN_SKEL_RIGHT_FINGERTIP);
        //
        //        transformNode(20, XN_SKEL_LEFT_ANKLE);
        //        transformNode(21, XN_SKEL_LEFT_FOOT);
        //
        //        transformNode(22, XN_SKEL_RIGHT_ANKLE);
        //        transformNode(23, XN_SKEL_RIGHT_FOOT);
        
    }
#endif
#ifdef USE_OSC
    for ( int i=0; i<NUM_MSG_STRINGS; i++ )
    {
        if ( timers[i] < ofGetElapsedTimef() )
            msg_strings[i] = "";
    }
    // check for waiting messages
    while ( receiver.hasWaitingMessages() )
    {
        
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage( &m );
        if (remoteHost!=m.getRemoteIp())
        {
            bSetupSender = false;
        }
        if (!bSetupSender)
        {
            bSetupSender = true;
            remoteHost = m.getRemoteIp();
            sender.setup(remoteHost, PORT+1);
            
        }
        
        if (m.getAddress()==("/mrmr accelerometer 0 "+deviceID) || m.getAddress()==("Accelerometer"))
        {
            
            for ( int i=0; i<m.getNumArgs(); i++ )
            {
                if ( m.getArgType( i ) == OFXOSC_TYPE_FLOAT )
                {
                    switch (i)
                    {
                        case 0:
                            r_x = m.getArgAsFloat( i ) ;
                            break;
                        case 1:
                            r_y = m.getArgAsFloat( i ) ;
                            break;
                        case 2:
                            r_z = m.getArgAsFloat( i ) ;
                            break;
                    }
                }
            }
            
            
        }
        else if (m.getAddress()=="OpenNI")
        {
            if ( m.getArgType( 0 ) == OFXOSC_TYPE_STRING )
            {
                string fn = m.getArgAsString( 0 ) ;
            }
            if ( m.getArgType( 0 ) == OFXOSC_TYPE_INT32 )
            {
                int index = m.getArgAsInt32( 0 ) ;
#ifdef USE_VBO
                vbo.fadeOut();
                timer.setup(2,false);
                timer.startTimer();
                ofAddListener(timer.TIMER_REACHED,this,&testApp::reachedTime);
#endif
                setupInput(index);
            }
            
        }
        else if (m.getAddress().find("pushbutton")!=string::npos && m.getAddress().find(deviceID)!=string::npos)
        {
            
            if ( m.getArgType( 0 ) == OFXOSC_TYPE_FLOAT )
            {
                bool bPush = m.getArgAsFloat( 0 ) ;
                if (bPush)
                {
                    if (m.getAddress().find("pushbutton 1")!=string::npos)
                    {
                        setupInput(0);
                    }
                    else if (m.getAddress().find("pushbutton 2")!=string::npos)
                    {
                        setupInput(1);
                    }
                    else if (m.getAddress().find("pushbutton 3")!=string::npos)
                    {
                        setupInput(3);
                    }
                    else if (m.getAddress().find("pushbutton 4")!=string::npos)
                    {
                        setupInput(4);
                    }
                    else if (m.getAddress().find("pushbutton 5")!=string::npos)
                    {
                        setupInput(5);
                    }
                    else if (m.getAddress().find("pushbutton 6")!=string::npos)
                    {
                        setupInput(6);
                    }
                }
            }
            
            
        }
        /*else if(m.getAddress()==("/mrmr pushbutton 2 "+deviceID))
         {
         
         if( m.getArgType( 0 ) == OFXOSC_TYPE_FLOAT )
         {
         bool bPush = m.getArgAsFloat( 0 ) ;
         if(bPush)
         {
         
         }
         }
         
         
         }*/
        
        else
        {
            // unrecognized message: display on the bottom of the screen
            string msg_string;
            msg_string = m.getAddress();
            msg_string += ": ";
            
            for ( int i=0; i<m.getNumArgs(); i++ )
            {
                // get the argument type
                msg_string += m.getArgTypeName( i );
                msg_string += ":";
                // display the argument - make sure we get the right type
                if ( m.getArgType( i ) == OFXOSC_TYPE_INT32 )
                    msg_string += ofToString( m.getArgAsInt32( i ) );
                else if ( m.getArgType( i ) == OFXOSC_TYPE_FLOAT )
                    msg_string += ofToString( m.getArgAsFloat( i ) );
                else if ( m.getArgType( i ) == OFXOSC_TYPE_STRING )
                    msg_string += m.getArgAsString( i );
                else
                    msg_string += "unknown";
            }
            // add to the list of strings to display
            msg_strings[current_msg_string] = msg_string;
            logger.log(OF_LOG_VERBOSE,msg_strings[current_msg_string].c_str());
            ofLog(OF_LOG_VERBOSE,msg_strings[current_msg_string].c_str());
            timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
            
            current_msg_string = ( current_msg_string + 1 ) % NUM_MSG_STRINGS;
            // clear the next line
            msg_strings[current_msg_string] = "";
            
        }
        
    }
#endif
    if (bRecCamera)
        
    {
        //-------
        //we change the background color based on
        //the two mouse coords coming in
        float xpct = (float)smoothMouseX / ofGetWidth();
        float ypct = (float)smoothMouseX / ofGetHeight();
        
        
        
        //-------------
        //we also want to record the CAMERA
        
        //lets store the drag of the user.
        //we will push into the most recent
        //<CAMERA> tag - add the mouse points
        //then pop out
        if ( camreRecording.pushTag("CAMERA", lastTagNumber) )
        {
            
            //now we will add a pt tag - with two
            //children - X and Y
            
            int tagNum = camreRecording.addTag("PT");
            camreRecording.setValue("PT:X", smoothMouseX, tagNum);
            camreRecording.setValue("PT:Y", smoothMouseY, tagNum);
            
            camreRecording.popTag();
        }
        
        //-------------
        //here we make a string of text that looks like how the xml data
        //is stored in the settings file - this is just so people can
        //visually see how the data is stored.
        
        //if the text is about to go off screen
        if (lineCount > 64)
        {
            //we find the first <PT> tag with the
            //x and y data and we remove it from the begining
            //this way the displayed text always shows the newest data
            //without going offscreen.
            int pos = xmlStructure.find("</PT>");
            xmlStructure = xmlStructure.substr(pos+6);
        }
        
        //here we add the most recent point to our fake xml string
        xmlStructure += "    <PT>\n        <X>"+ofToString(smoothMouseX)+"</X>\n        <Y>"+ofToString(smoothMouseY)+"</Y>\n    </PT>\n";
        lineCount+=4; //we have added 4 lines so increment by 4
        
        
    }
    
}

//--------------------------------------------------------------
void testApp::transformNode(int nodeNum, XnSkeletonJoint skelJoint)
{
    
    // Adapted code from OpenNI Simbad example
#ifdef TRACK_USER
    XnUserID userId = user.getTrackedUser(0)->id;
    
    // Get the openNI bone info
    xn::SkeletonCapability pUserSkel = user.getXnUserGenerator().GetSkeletonCap();
    
    XnSkeletonJointOrientation jointOri;
    pUserSkel.GetSkeletonJointOrientation(userId, skelJoint, jointOri);
    
    XnSkeletonJointPosition jointPos;
    pUserSkel.GetSkeletonJointPosition(userId, skelJoint, jointPos);
    
    if (jointOri.fConfidence > 0 )
    {
        float * oriM = jointOri.orientation.elements;
        
        ofMatrix4x4 rotMatrix;
        
        // Create a 4x4 rotation matrix (converting row to column-major)
        rotMatrix.set(oriM[0], oriM[3], oriM[6], 0.0f,
                      oriM[1], oriM[4], oriM[7], 0.0f,
                      oriM[2], oriM[5], oriM[8], 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
        
        ofQuaternion q = rotMatrix.getRotate();
        
        testNodes[nodeNum].setPosition(jointPos.position.X, jointPos.position.Y, jointPos.position.Z);
        
        // apply skeleton pose relatively to the bone bind pose
        // /!\ WARNING the order of the quat' multiplication does mater!!
        testNodes[nodeNum].setOrientation(testNodes[nodeNum].bindPoseOrientation*q);
    }
#endif
}

//--------------------------------------------------------------
void testApp::draw()
{
    
    ofSetColor(255, 255, 255);
    fbo.begin();
    ofPushStyle();
    ofSetHexColor(0x000000);
    ofRect(0, 0, fbo.getWidth(), fbo.getHeight());
    ofPopStyle();
    glPushMatrix();
#ifdef USE_AR
#ifdef CAMERA_CONNECTED
    vidGrabber.draw(0,0,ofGetWidth(),ofGetHeight());
#else
    vidPlayer.draw(0,0,ofGetWidth(),ofGetHeight());
#endif
    
    
#endif
    
    if (bDebug)
    {
        depth.draw(0, 0, 320, 240);
        image.draw(0, 240, 320, 240);
#ifdef TRACK_USER
        drawUser();
        
        //        user.draw();
#endif
        //        drawAR();
    }
    
    
    fbo.end();
    
    glPopMatrix();
    
    switch (curPlaymode)
    {
        case 0:
            // setup scene Rendering
            
            //glEnable(GL_LIGHTING);
            //ofEnableLighting();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            //glEnable(GL_LIGHT0);
            if (bCamera)cam.begin();
            //        depth.draw(0,0,ofGetWidth(),ofGetHeight());
            //if(user.getNumberOfTrackedUsers()>0) draw3DScene();
            if(bCloud)drawPointCloud();
            //            if(bCloud)drawPointCloud(&user, 1);
            
#ifdef USE_VBO
            if (bVBO)
            {
                vbo.draw();
            }
#endif
            if (bCamera)cam.end();
            
            ofDisableLighting();
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHT0);
            glDisable(GL_LIGHTING);
            
            break;
        case 1:
            if (bCamera)cam.begin();
#ifdef TRACK_USER
            if (user.getNumberOfTrackedUsers()>0) draw3DScene();
#endif
            
            
            if (bCamera)cam.end();
            break;
        default:
#ifdef TRACK_USER
            drawUser();
#endif
            break;
    }
#ifdef USE_STREAMING
    img.grabScreen(0, 0, ofGetWidth() , ofGetHeight());
    frame.setFromPixels(img.getPixels(), img.getWidth(), img.getHeight());
    frameSender->updateFrame(&frame);
#else
    //#ifdef USE_FENSTER
    if (bRecord || bRecord2)
    {
        
        img.grabScreen(0, 0, ofGetWidth() , ofGetHeight());
        if (     counter<10  )img.saveImage(target_path+"recording_000"+ofToString(counter++)+".tif");
        else if (counter>9   && counter<100)img.saveImage(target_path+"recording_00"+ofToString(counter++)+".tif");
        else if (counter>99  && counter<1000)img.saveImage(target_path+"recording_0"+ofToString(counter++)+".tif");
        else if (counter>999 && counter<10000)img.saveImage(target_path+"recording_"+ofToString(counter++)+".tif");
        else img.saveImage(target_path+"recording_"+ofToString(counter++)+".tif");
    }
    //#endif
#endif
    
#ifdef USE_AR
    // ARTK 3D stuff
    // First apply the projection matrix once
    /*artk.applyProjectionMatrix(ofGetWidth(),ofGetHeight());
     
     // Find out how many markers have been detected
     int numDetected = artk.getNumDetectedMarkers();
     // Draw for each marker discovered
     for(int i=0; i<numDetected; i++) {
     
     // Set the matrix to the perspective of this marker
     // The origin is in the middle of the marker
     artk.applyModelMatrix(i);
     drawPointCloud();
     }*/
#endif
    string statusHardware;
#ifdef KINECT_CONNECTED
#ifdef TARGET_OSX // only working on Mac at the moment
    ofPoint statusAccelerometers = hardware.getAccelerometers();
    stringstream	statusHardwareStream;
    
    statusHardwareStream
    << "ACCELEROMETERS:"
    << " TILT: " << hardware.getTiltAngle() << "/" << hardware.tilt_angle
    << " x - " << statusAccelerometers.x
    << " y - " << statusAccelerometers.y
    << " z - " << statusAccelerometers.z;
    
    statusHardware = statusHardwareStream.str();
    logger.log(OF_LOG_VERBOSE,statusHardware);
#endif
#endif
    //    gui.draw();
#ifdef USE_OSC
    //    ofSetHexColor(0xFFFFFF);
    //    string buf;
    //	buf = "listening for osc messages on port" + ofToString( PORT );
    //	ofDrawBitmapString( buf, 10, 20 );
    //
    //	// draw mouse state
    //	buf = "mouse: " + ofToString( mouseX, 4) +  " " + ofToString( mouseY, 4 );
    //	ofDrawBitmapString( buf, 430, 20 );
    //
    //	for ( int i=0; i<NUM_MSG_STRINGS; i++ )
    //	{
    //		ofDrawBitmapString( msg_strings[i], 10, 40+15*i );
    //        if(msg_strings[i]!="")ofLog(OF_LOG_VERBOSE, msg_strings[i]);
    //	}
#endif
    ofSetHexColor(0xFFFFFF);
    string outputStr = "Convert Time: "+ofToString(convertMin)+":"+ofToString(convertSec)+"\n"
    +"Recording frame : "+ofToString(bRecord)+
    +"\nRecording Camera : "+ofToString(bRecCamera);
    ofDrawBitmapString(outputStr , 10, ofGetHeight()-100);
    
}

void testApp::keyPressed(int key)
{
    
    switch (key)
    {
        case 'r':
            bRecord = !bRecord;
            if(bRecord)
            {
                convertSec=0;
                convertMin=0;
                setupInput();
            }
            break;
        case 'R':
            bRecord2 = !bRecord2;
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case']':
            gui.setSelectedPanel((gui.getSelectedPanel()+1)%gui.globalPanelList.size());
            break;
        case'[':
            gui.setSelectedPanel(((gui.getSelectedPanel()-1)<0)?gui.globalPanelList.size()-1:(gui.getSelectedPanel()-1));
            break;
        case 'o':
            bOrbit=!bOrbit;
            break;
        case ' ' :
            bDebug = !bDebug;
            if (bDebug)
            {
                //                ofShowCursor();
                gui.setDraw(true);
            }
            else
            {
                //                ofHideCursor();
                gui.setDraw(false);
            }
            break;
        case 'l':
            //            context.shutdown();
            //            context.setup();	// all nodes created by code -> NOT using the xml config file at all
            //            depth.setup(&context);
            //            image.setup(&context);
            //            user.setup(&context);
            //            user.setSmoothing(0.1);				// built in openni skeleton smoothing...
            //            //	user.setUseMaskPixels(isMasking);
            //            user.setUseCloudPoints(bCloud);
            //            context.toggleRegisterViewport();
            //            context.toggleMirror();
            break;
#ifdef USE_VBO
        case '1':
            vbo.fadeIn();
            break;
        case '2':
            vbo.fadeOut();
            break;
        case '3':
            vbo.stopFade();
            break;
#endif
            //        case 'c' : bCloud = !bCloud; break;
        case 'c':
            camreRecording.saveFile("cameraSettings.xml");
            break;
        case 's':
            bRecCamera = !bRecCamera;
            if (bRecCamera)
            {
                convertSec=0;
                convertMin=0;
                setupInput();
                
                lastTagNumber	= camreRecording.addTag("CAMERA");
                xmlStructure	= "<CAMERA>\n";
                
                //We start a new CAMERA
                lineCount		= 0;
                pointCount		= 0;
            }
            else 
            {
                camreRecording.saveFile("cameraSettings.xml");
            }
            break;
        case 'S':
            //if(!bRecCamera)
        {
            camreRecording.loadFile("cameraSettings.xml");
            //lets see how many <CAMERA> </CAMERA> tags there are in the xml file
            int numDragTags = camreRecording.getNumTags("CAMERA:PT");
            
            //if there is at least one <CAMERA> tag we can read the list of points
            //and then try and draw it as a line on the screen
            if(numDragTags > 0){
                printf("camera setting has points :%i\n",numDragTags);
                //we push into the last CAMERA tag
                //this temporarirly treats the tag as
                //the document root.
                camreRecording.pushTag("CAMERA", numDragTags-1);
                
                //we see how many points we have stored in <PT> tags
                int numPtTags = camreRecording.getNumTags("PT");
                
                if(numPtTags > 0){
                    
                    //We then read those x y values into our
                    //array - so that we can then draw the points as
                    //a line on the screen
                    
                    //we have only allocated a certan amount of space for our array
                    //so we don't want to read more than that amount of points
                    int totalToRead = MIN(numPtTags, NUM_PTS);
                    
                    for(int i = 0; i < totalToRead; i++){
                        //the last argument of getValue can be used to specify
                        //which tag out of multiple tags you are refering to.
                        int x = camreRecording.getValue("PT:X", 0, i);
                        int y = camreRecording.getValue("PT:Y", 0, i);
                        dragPts.push_back(ofPoint(x, y));
                        
                    }
                }
                pointCount=0;
                //this pops us out of the CAMERA tag
                //sets the root back to the xml document
                camreRecording.popTag();
            }
        }
            break;
            
            //        case OF_KEY_UP:camScale+=0.1; break;
            //        case OF_KEY_DOWN:camScale-=0.1; break;
        case 'e':
            bExport = !bExport;
            break;
        default:
            break;
    }
#ifdef USE_AR
    if (key == OF_KEY_LEFT)
    {
        artk.setThreshold(++threshold);
        
    }
    else if (key == OF_KEY_RIGHT)
    {
        artk.setThreshold(--threshold);
    }
#ifdef CAMERA_CONNECTED
    //    if(key == 's') {
    //        vidGrabber.videoSettings();
    //    }
#endif
#endif
}

void testApp::dragEvent(ofDragInfo dragInfo)
{
    if (dragInfo.files.size()>0)
    {
#ifdef USE_RECORDING
        if (targetContext!=NULL)targetContext->shutdown();
        if (targetContext!=NULL)targetContext->setupUsingRecording(dragInfo.files[0].c_str());
        depth.setup(targetContext);
        image.setup(targetContext);
#ifdef TRACK_USER
        user.setup(targetContext);
        user.setSmoothing(0.1);				// built in openni skeleton smoothing...#endif
        //        user.setUseMaskPixels(isMasking);
        //        user.setUseCloudPoints(bCloud);
#endif
        
#endif
    }
}
void testApp::drawPointCloud()
{
    
    DepthGenerator depth_generator = depth.getXnDepthGenerator();
    
    DepthMetaData dmd;
    depth_generator.GetMetaData(dmd);
    const XnDepthPixel* depth = dmd.Data();
    XN_ASSERT(depth);
    
    if (dmd.FrameID() == 0)
    {
        return;
    }
    if (dmd.PixelFormat() == XN_PIXEL_FORMAT_RGB24)
    {
        printf("its in yuv\n");
    }
    
    XnUInt32 nAllIndex = (dmd.YRes() + dmd.YOffset()) * dmd.XRes();
    XnVector3D projective[nAllIndex];
    int all = 0;
    for (XnUInt16 y = dmd.YOffset(); y < dmd.YRes() + dmd.YOffset(); y++)
    {
        for (XnUInt16 x = 0; x < dmd.XRes(); x++, depth++, all++)
        {
            projective[all].X = x;
            projective[all].Y = y;
            projective[all].Z = *depth;
        }
    }
    
    XnVector3D realworld[nAllIndex];
    depth_generator.ConvertProjectiveToRealWorld(nAllIndex, projective, realworld);
    
    // Draw
    
    ImageMetaData imd;
    image.getXnImageGenerator().GetMetaData(imd);
    const XnUInt8* color = imd.Data();
    XN_ASSERT(color);
    
    all = 0;
    int texture=0;
    int count=0;
    
    glPushMatrix();
    if (!bVBO)
    {
        glBegin(GL_POINTS);
        glColor3ub(255, 255, 255);
    }
    unsigned char *pixels;
    if(bExport)
    {
        pixels= new unsigned char [NUM_PIXELS];
        
    }
    int _x = 0;
    int  _y = 0;
    for (XnUInt16 y = dmd.YOffset(); y < dmd.YRes() + dmd.YOffset(); y++, _y++)
    {
        for (XnUInt16 x = 0; x < dmd.XRes(); x++, all++, texture+=3 , _x++)
        {
#ifdef USE_VBO
            if (bVBO)
            {
                
                vbo.setParticleTraget(count++,realworld[all].X, realworld[all].Y, realworld[all].Z, color[texture], color[texture+1], color[texture+2]);
            } 
            else
            {
                glColor3ub(color[texture], color[texture+1], color[texture+2]);
                
                glVertex3f(realworld[all].X, realworld[all].Y, realworld[all].Z);
            }
#else
            glColor3ub(color[texture], color[texture+1], color[texture+2]);
            glVertex3f(realworld[all].X, realworld[all].Y, realworld[all].Z);
            if(bExport)
            {
                pixels[(_x+_y*4)] = color[texture]*255;
                pixels[(_x+_y*4)+1] = color[texture+1]*255;
                pixels[(_x+_y*4)+2] = color[texture+2]*255;
                pixels[(_x+_y*4)+3] = 255;//e(unsigned char) ofClamp(ofMap(realworld[all].Z, 0, 3000, 1, 256), 1, 255);
            }
#endif
        }
    }
    if(bExport)
    {

        exportImage.setFromPixels(pixels,640,480,OF_IMAGE_COLOR_ALPHA);
        exportImage.saveImage(ofToString(fileCount++)+".png");
        delete []pixels;
    }    
    if (!bVBO)
    {
        glEnd();
    }
    glPopMatrix();
    //ofDrawBitmapString("X: "+ofToString(realworld[all].X)+"\nY: "+ofToString(realworld[all].Y)+"\nZ: "+ ofToString(realworld[all].Z), 10,ofGetHeight()-20);
    
}

void testApp::drawPointCloud(ofxUserGenerator * user_generator, int userID)
{
    
    glPushMatrix();
    
    int w = user_generator->getWidth();
    int h = user_generator->getHeight();
    
    glTranslatef(w, h/2, -500);
    ofRotateY(0);
    
    glBegin(GL_POINTS);
    
    int step = 1;
    
    for (int y = 0; y < h; y += step)
    {
        for (int x = 0; x < w; x += step)
        {
            ofPoint pos = user_generator->getWorldCoordinateAt(x, y, userID);
            //if (pos.z == 0 && ) continue;	// gets rid of background -> still a bit weird if userID > 0...
            ofColor color = user_generator->getWorldColorAt(x,y, userID);
            glColor4ub((unsigned char)color.r, (unsigned char)color.g, (unsigned char)color.b, (unsigned char)color.a);
            glVertex3f(pos.x, pos.y, pos.z);
        }
    }
    
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    
    glPopMatrix();
}



void testApp::draw3DScene()
{
    
    //light.draw();
    
    //ofDrawAxis(100);
    
    for (int i=0; i<kNumTestNodes; i++)
    {
        testNodes[i].draw();
        //img.getTextureReference().bind();
        //img.getTextureReference().unbind();
    }
    
}
void testApp::drawUser()
{
#ifdef TRACK_USER
    glPushMatrix();
    glTranslatef(_userX,-_userY,0);
    glScalef(_userScale,_userScale,1);
    user.draw();
    glPopMatrix();
#endif
}
void testApp::setupInput()
{
#ifdef USE_RECORDING
    string name = inputList.getSelectedName();
    int index = inputList.selected;
    bool usingONIFile = name.find("oni") != string::npos;
    if (usingONIFile)
    {
        targetContext = &context[index];
        logger.log(OF_LOG_VERBOSE,"Selected :%s",name.c_str());
        depth.setup(targetContext);
        image.setup(targetContext);
#ifdef TRACK_USER
        user.setup(targetContext);
        user.setSmoothing(0.1);				// built in openni skeleton smoothing...
        user.setUseCloudPoints(bCloud);
#endif
    }
#endif
}
void testApp::setupInput(int index)
{
#ifdef USE_RECORDING
    if (index<0 || index>inputList.getNumEntries())
        return ;
#ifdef USE_OSC
    if (bSetupSender)
    {
        ofxOscMessage m;
        m.setAddress("ServerLock");
        m.addIntArg(1);
        sender.sendMessage(m);
    }
#endif
    targetContext = &context[index];
    depth.setup(targetContext);
    image.setup(targetContext);
#ifdef TRACK_USER
    user.setup(targetContext);
    user.setSmoothing(0.1);				// built in openni skeleton smoothing...
    user.setUseCloudPoints(bCloud);
#endif
#ifdef USE_OSC
    if (bSetupSender)
    {
        ofxOscMessage m;
        m.setAddress("ServerLock");
        m.addIntArg(0);
        sender.sendMessage(m);
    }
#endif
#endif
}
#ifdef USE_FENSTER
void testApp::fensterUpdate()
{
    //img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
}

void testApp::fensterDraw()
{
    if (! bTexInit)
    {
        tex.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
        bTexInit = true;
    }
    tex.loadData(img.getPixels(), img.getWidth(), img.getHeight(), GL_RGB);
    tex.draw(0, 0, fenster->getWidth(), fenster->getHeight());
}
#endif
#ifdef USE_HTTPSERVER
void testApp::getRequest(ofxHTTPServerResponse & response)
{
    if (response.url=="/showScreen.of")
    {
        response.response="<html> <head> <title>oF http server</title> \
        <script> \
        function beginrefresh(){ \
        setTimeout(\"window.location.reload();\",30); \
        }\
        window.onload=beginrefresh; \
        </script>\
        </head> <body> <img src=\"screen.jpg\"/> </body> </html>";
        
        imageSaved  = false;
    }
}

void testApp::postRequest(ofxHTTPServerResponse & response)
{
    if (response.url=="/postImage.of")
    {
        postedImgName = response.requestFields["name"];
        postedImgFile = response.uploadedFiles[0];
        response.response = "<html> <head> oF http server </head> <body> image " + response.uploadedFiles[0] + " received correctly <body> </html>";
    }
}
#endif

