#pragma once

#include "ofxControlPanel.h"
#include "ofEvents.h"
#include "ofMain.h"
class ofxAutoControlPanel : public ofxControlPanel {
public:
	ofxAutoControlPanel() {
		ofAddListener(ofEvents.update, this, &ofxAutoControlPanel::update);
		ofAddListener(ofEvents.draw, this, &ofxAutoControlPanel::draw);
		ofAddListener(ofEvents.mousePressed, this, &ofxAutoControlPanel::mousePressed);
		ofAddListener(ofEvents.mouseReleased, this, &ofxAutoControlPanel::mouseReleased);
		ofAddListener(ofEvents.mouseDragged, this, &ofxAutoControlPanel::mouseDragged);
	}
    void setDraw(bool boolean)
    {
        if(boolean)
        {
            ofAddListener(ofEvents.draw, this, &ofxAutoControlPanel::draw);
        }
        else
        {
            ofRemoveListener(ofEvents.draw, this, &ofxAutoControlPanel::draw);
        }
    }
	void update(ofEventArgs& event) {
		ofxControlPanel::update();
	}
	void draw(ofEventArgs& event) {
        ofPushStyle();
        ofSetLineWidth(1);
		ofxControlPanel::draw();
        ofPopStyle();
	}
	void mousePressed(ofMouseEventArgs& event) {
		ofxControlPanel::mousePressed(event.x, event.y, event.button);
	}
	void mouseReleased(ofMouseEventArgs& event) {
		ofxControlPanel::mouseReleased();
	}
	void mouseDragged(ofMouseEventArgs& event) {
		ofxControlPanel::mouseDragged(event.x, event.y, event.button);
	}
};
