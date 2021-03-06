/*
 *  ImageTypePictureSequence.h
 *  ofApp
 *
 *  Created by Brian Eschrich on 12/12/14
 *  Copyright 2014 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _ImageTypePictureSequence
#define _ImageTypePictureSequence

#include "ofMain.h"
#include "ImageType.h"
#include "ofxImageSequencePlayer.h"

class ImageTypePictureSequence : public ImageType{
	
  public:
	
	ImageTypePictureSequence(string name_ ,string path_);
	
    void activate(ofImage& _img, ofTexture& _tex);
    void update(ofImage& _img, ofTexture& _tex);
    int getFrameRate();
    float getPosition();
    void calculateFPS();
    void setPosition(float p, ofImage& _img, ofTexture& _tex);
    void setLoopState(ofLoopType l);
    
private:
    ofxImageSequencePlayer player;
    ofImage img;
    ofTexture tex;
};

#endif
