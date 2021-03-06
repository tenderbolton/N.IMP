/*
 *  MixMask.cpp
 *  ofApp
 *
 *  Created by Brian Eschrich on 08/12/14
 *  Copyright 2014 __MyCompanyName__. All rights reserved.
 *
 */

#include "MixMask.h"

MixMask::MixMask(string name_):MixTable(name_){
    gui.add(spin.setup("spin", 90, 0, 360));
    
    maxInputs = 2;
    maskShader.load("Shaders/composite");
    maskShader.begin();
       maskShader.end();
    //fbo.setDefaultTextureIndex(1);
}
//------------------------------------------------------------------
void MixMask::setup() {
	
	
}


//------------------------------------------------------------------
void MixMask::draw(int x,int y, float scale) {
    ofSetColor(255, 255, 255);
    float ratio = (float)heigth/(float)width;
    int w = 640*scale;
    int h = w*ratio;
    fbo.draw(x, y,w,h);
    ofDrawBitmapString(name, x + 10, y + 30);
	
}

void MixMask::update(){
    
    fbo.begin();
    ofClear(255,255,255, 0);
    input[0]->getTexture()->draw(0, 0);
    ofEnableAlphaBlending();
    ofPushMatrix();
    ofTranslate(width/2, heigth/2);
    ofPushMatrix();
    ofRotate(spin);
    ofPushMatrix();
    ofTranslate(-width/2, -heigth/2);
    drawShader();
    ofPopMatrix();
    ofPopMatrix();
    ofPopMatrix();
    ofDisableBlendMode();
    fbo.end();
    tex = fbo.getTextureReference();
}

void MixMask::drawShader(){
    maskShader.begin();
    maskShader.setUniformTexture("Tex0", *input[0]->getTexture(), 0);
    maskShader.setUniformTexture("Tex1", *input[1]->getTexture(), 1);

    //our shader uses two textures, the top layer and the alpha
    //we can load two textures into a shader using the multi texture coordinate extensions
    glActiveTexture(GL_TEXTURE0_ARB);
    input[0]->getTexture()->bind();
    
    glActiveTexture(GL_TEXTURE1_ARB);
    input[1]->getTexture()->bind();
    
    //draw a quad the size of the frame
    glBegin(GL_QUADS);
    
    //move the mask around with the mouse by modifying the texture coordinates
    float maskOffset = 0;//15 - mouseY;
    glMultiTexCoord2d(GL_TEXTURE0_ARB, 0, 0);
    glMultiTexCoord2d(GL_TEXTURE1_ARB, 0, maskOffset);
    glVertex2f( 0, 0);
    
    glMultiTexCoord2d(GL_TEXTURE0_ARB, width, 0);
    glMultiTexCoord2d(GL_TEXTURE1_ARB, width, maskOffset);
    glVertex2f( width, 0);
    
    glMultiTexCoord2d(GL_TEXTURE0_ARB, width, heigth);
    glMultiTexCoord2d(GL_TEXTURE1_ARB, width, heigth + maskOffset);
    glVertex2f( width,heigth);
    
    glMultiTexCoord2d(GL_TEXTURE0_ARB, 0, heigth);
    glMultiTexCoord2d(GL_TEXTURE1_ARB, 0, heigth + maskOffset);
    glVertex2f( 0, heigth );
    
    glEnd();
    
    //deactive and clean up
    glActiveTexture(GL_TEXTURE1_ARB);
    input[0]->getTexture()->unbind();
    
    glActiveTexture(GL_TEXTURE0_ARB);
    input[1]->getTexture()->unbind();
    
    maskShader.end();
}

void MixMask::updateParameter(Param* inputParam){
    
}