/**
	Implements a thread that sends frames over TCP. A frame can be an image or a
	frame from a video which results in streaming simulation.
	@author: Jan Christian Hjorth Hansen - rocknrolldk@gmail.com
 */

#include "FrameSender.h"

#include <cstdio>
using std::cout;
using std::endl;

FrameSender::FrameSender() : activity(this, &FrameSender::runActivity) {
	FrameSender(DEFAULT_PORT, SENDER_FRAME_WIDTH, SENDER_FRAME_HEIGHT);
}

FrameSender::FrameSender(int port, int frameWidth, int frameHeight) : activity(this, &FrameSender::runActivity) {
	this->port = port;
	this->frameWidth = frameWidth;
	this->frameHeight = frameHeight;
	frameSize = frameWidth * frameHeight * 3;
	
	bufferFrame.allocate(frameWidth, frameHeight);
	
	server.setup(port, true); //this causes OS X to ask for permission to receive from network
	server.setVerbose(true); //NOTE: Remove when done testing
}

FrameSender::~FrameSender() {
}

void FrameSender::start() {
	activity.start();
}

void FrameSender::stop() {
	activity.stop();
}

/**
	Writes a new frame into the buffer frame that gets sent via TCP.
 */
void FrameSender::updateFrame(ofxCvColorImage* frame) throw (length_error) {
	if (frame->getWidth() > frameWidth || frame->getHeight() > frameHeight) {
		throw length_error("FrameSender::updateFrame: Frame size is bigger than the size of the buffer.");
	}
	
	if (rwlock.tryWriteLock()) {
		bufferFrame.setFromPixels(frame->getPixels(), frame->getWidth(), frame->getHeight());
		rwlock.unlock();
	}
}

void FrameSender::runActivity() {
	while (!activity.isStopped()) {
		
		//only the first connected client will be considered: id=0
		if (server.isClientConnected(0)) {
			//cout << "connected" << endl;
			if (rwlock.tryReadLock()) {
				
				//Send frame by throttling, ie splitting frame up into chunks (rows) and sending them sequentially
				const char* index = (const char*)bufferFrame.getPixels(); //start at beginning of pixel array
				int length = frameWidth * 3; //length of one row of pixels in the image
				int pixelCount = 0;
				while (pixelCount < frameSize) {
					server.sendRawBytes(0, index, length); //send the first row of the image
					index += length; //increase pointer so that it points to the next image row
					pixelCount += length; //increase pixel count by one row
				}
				
				rwlock.unlock();
			}
		}
		else {
			//cout << "NOT" << endl;
		}
		
		ofSleepMillis(SENDER_ACTIVITY_SLEEP);
	}
}
