//
//  AudioListener.h
//  ofApp
//
//  Created by Christian Clark on 12/15/14.
//
//

#ifndef __ofApp__AudioListener__
#define __ofApp__AudioListener__

#include "ofMain.h"
#include "ParamInputGenerator.h"

// this class implements the behavior of a audio listener.
// it runs on its own thread, and receives audio data fro the main thread.
// the main thread copies data to the thread, and lets the thread use and dispose it.

// this class is used to control the data exchange behavior between audio specific paramGenerators as FFT, etc.

class AudioListenerInput: public ParamInputGenerator {
    
public:
    
    AudioListenerInput(string name_);
    void processInput();
    bool setupFromXML();
    bool fillNewData(float* left_, float* right_, int bufferLen_);
    void disposeData();

protected:
    
    void setDataProcessed();
    bool hasNewData;
    bool isDataProcessed;
    int bufferLen;
    float* left;
    float* right;
    
};


#endif /* defined(__ofApp__AudioListener__) */
