/**
	Defines a thread that sends frames over TCP. A frame can be an image or a
	frame from a video which results in streaming simulation.
	@author: Jan Christian Hjorth Hansen - rocknrolldk@gmail.com
 */
#ifndef FRAMESENDER_H
#define FRAMESENDER_H

#include <stdexcept>
using std::length_error;

#include "ofxOpenCv.h"
#include "Poco/Activity.h"
#include "Poco/RWLock.h"
#include "ofxNetwork.h"
#include "ofxThread.h"

#define DEFAULT_PORT 11999
/**
	This setting defines the length of the wait in each thread loop.
	Optimize for performance.
 */
#define SENDER_ACTIVITY_SLEEP 100
#define SENDER_FRAME_WIDTH 800
#define SENDER_FRAME_HEIGHT 600

class FrameSender {
public:
	FrameSender();
	FrameSender(int port, int frameWidth, int frameHeight);
	~FrameSender();
	
	void start();
	void stop();
	
	/**
		Writes a new frame into the buffer frame that gets sent via TCP.
	 */
	void updateFrame(ofxCvColorImage* frame) throw (length_error);
	
private:	
	Poco::Activity<FrameSender> activity;
	Poco::RWLock rwlock;
	
	ofxTCPServer server;
	int port;
	
	ofxCvColorImage bufferFrame;
	const char* pixels;
	int frameWidth;
	int frameHeight;
	int frameSize;
	
	void runActivity(); //the main thread function
};

#endif