/*
 *  ChannelMixer.cpp
 *  ofApp
 *
 *  Created by Brian Eschrich on 10/12/14
 *  Copyright 2014 __MyCompanyName__. All rights reserved.
 *
 */

#include "MultiChannelMixer.h"

MultiChannelMixer::MultiChannelMixer(string name_):MixTable(name_){
    psBlend.setup(width, heigth);
    maxInputs = 8;
}
void MultiChannelMixer::inputAdded(ImageOutput* in_){
    renderLayers.push_back(new ofFbo());
    renderLayers[renderLayers.size()-1]->allocate(width, heigth,GL_RGBA32F_ARB);
    renderLayers[renderLayers.size()-1]->begin();
    ofClear(255, 255, 255,0);
    renderLayers[renderLayers.size()-1]->end();
    
    opacity.push_back(12);
    gui.add(opacity[opacity.size()-1].set("channel " + ofToString(opacity.size()), 0, 0, 255));
}


//------------------------------------------------------------------
void MultiChannelMixer::setup() {
	
	
}


//------------------------------------------------------------------
void MultiChannelMixer::update() {
    ofEnableAlphaBlending();

    for (int i=0; i<input.size(); ++i) {
        ofSetColor(255, 255, 255);
        if (opacity[i] != 0) {
            renderLayers[i]->begin();
            input[i]->getTexture()->draw(0, 0);
            renderLayers[i]->end();
        }
    }
    
    fbo.begin();
    ofClear(255,255,255, 0);
    for (int i=0; i<input.size(); ++i) {
        ofSetColor(255, 255, 255);
        if (opacity[i] != 0) {
            //opacity layers
            ofSetColor(255, 255, 255,opacity[i]);
            input[i]->getTexture()->draw(0, 0);
        }
    }
    fbo.end();
}


//------------------------------------------------------------------
void MultiChannelMixer::draw(int x,int y, float scale) {
    ofSetColor(255, 255, 255);
    fbo.draw(x, y,640*scale, 480*scale);
    /*ofDrawBitmapString(name, x + 10, y + 30);
    ofDrawBitmapString("Channel       Blendmode          Opacity", x + 10, y + 480*scale + 20);
    ofDrawBitmapString("---------     ---------          -------", x + 10, y + 480*scale + 30);
    for (int i=0; i<controls.size(); ++i) {
        ofDrawBitmapString("channel " + ofToString(i+1), x + 10, y + 480*scale + 45 + (i*15));
        ofDrawBitmapString(psBlend.getBlendMode(controls[i].blendMode), x + 123, y + 480*scale + 45 + (i*15));
        ofDrawBitmapString(ofToString((int)ofMap(controls[i].opacity,0,255,0,100)), x + 280, y + 480*scale + 45 + (i*15));
    }*/
    int h = (480*scale)/input.size()*2;
    int w = (640*scale)/input.size()*2;
    
    for (int i=0; i<renderLayers.size(); ++i) {
        ofPushMatrix();
        int r = i/4 * (w + 10);
        ofTranslate(x+640*scale + 20 +r, y +(i%4*h));
        renderLayers[i]->draw(0,0,w*0.9 ,h*0.9);
        ofDrawBitmapString("channel " + ofToString(i+1), 5,15);
        ofDrawBitmapString("opacity " + ofToString((int)ofMap(opacity[i],0,255,0,100)), 5,30);
        ofPopMatrix();
    }
}