#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

void ofApp::setup() {
    ofSetWindowTitle("n.imp");
    ofSetFrameRate(30);
    loadingOK = false;
    isFullScreen = false;
    
    ofSetLogLevel(OF_LOG_ERROR);
    
    //populating string dictionaries for simple comparison used in LoadFromXML
    inputTypes.insert(std::pair<string,InputType>("VIDEO",VIDEO));
    inputTypes.insert(std::pair<string,InputType>("CAM",CAM));
    inputTypes.insert(std::pair<string,InputType>("IMAGE",IMAGE));
    inputTypes.insert(std::pair<string,InputType>("PARTICLE",PARTICLE));
    visualLayerTypes.insert(std::pair<string,VisualLayerType>("IKEDA", IKEDA));
    visualLayerTypes.insert(std::pair<string,VisualLayerType>("GLITCH_1", GLITCH_1));
    visualLayerTypes.insert(std::pair<string,VisualLayerType>("GLITCH_2", GLITCH_2));
    visualLayerTypes.insert(std::pair<string,VisualLayerType>("IMAGE_PROCESSOR", IMAGE_PROCESSOR));
    mixerTypes.insert(std::pair<string,MixerType>("MASK", MASK));
    mixerTypes.insert(std::pair<string,MixerType>("SIMPLE_BLEND", SIMPLE_BLEND));
    mixerTypes.insert(std::pair<string,MixerType>("MULTI_CHANNEL", MULTI_CHANNEL));
    inputGenTypes.insert(std::pair<string,InputGeneratorsType>("MIDI", MIDI));
    inputGenTypes.insert(std::pair<string,InputGeneratorsType>("FFT", FFT));
    inputGenTypes.insert(std::pair<string,InputGeneratorsType>("OSC", OSC));
    

    ofDrawBitmapString("LOADING XML ...", 50, 50);
    loadingOK = loadFromXML();
    
    if(loadingOK){
        //TODO: change mixtable assignment.
      //  ofAddListener(mixtables[0]->textureEvent, this, &ofApp::updateSyphon);
        
        //invoking input generators setup functions
        
        for(int i=0; i<inputGenerators.size(); i++){
            inputGenerators[i]->setup();
        }
        
        //starting input generators theads (the not threadeds will not start)
        
        for(int i=0; i<inputGenerators.size(); i++){
            inputGenerators[i]->start();
        }
        
        for(int i=0; i<syphonServers.size();i++){
            syphonServers[i]->setup();
        }
        
        setCurrentViewer(0);
        
        setupAudio();
    }
    
  }

void ofApp::setupAudio(){
    
    bufferCounter	= 0;
   
    // 0 output channels,
    // 2 input channels
    // 44100 samples per second
    // BUFFER_SIZE samples per buffer
    // 4 num buffers (latency)
    
    ofSoundStreamSetup(0,2,this, 44100,BUFFER_SIZE, 4);
    left = new float[BUFFER_SIZE];
    right = new float[BUFFER_SIZE];
    
    //soundStream.setup(this, 0, 2, 44100, bufferSize, 4);
}

void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    // samples are "interleaved"
    for (int i = 0; i < bufferSize; i++){
        left[i] = input[i*2];
        right[i] = input[i*2+1];
    }
    
    for (int i=0; i<audioListeners.size(); i++){
        // for each audio listener, i copy the audiobuffer and send it to it.
        
        float * leftToGo = new float[BUFFER_SIZE];
        float * rightToGo = new float[BUFFER_SIZE];
        
        memcpy ( leftToGo, left, sizeof (float)*BUFFER_SIZE );
        memcpy ( rightToGo, right, sizeof (float)*BUFFER_SIZE );
        
        if(!audioListeners[i]->fillNewData(leftToGo, rightToGo, BUFFER_SIZE)){
            // if data copy was not successful, then we delete the memory (because the input gen wont delete it)
            delete[] leftToGo;
            delete[] rightToGo;
        }
    }
    
    bufferCounter++;
}

void ofApp::update() {
    if(loadingOK){
        //resetting processed flags
        for (int i=0; i<nodesVector.size(); ++i) {
            nodesVector[i]->resetProcessedFlag();
        }
        
        //getting new paraminputs
        for (int i=0; i<inputGenerators.size(); ++i) {
            ParamInputGenerator* p = inputGenerators[i];
            
            int maxParams = 0;
            bool continueProcessing = true;
            while(maxParams<=5 && continueProcessing){
                Param* param = p->getNextInputMessage();
                
                //sending param info to the destination node
                if(param!=NULL){
                    ImageOutput* destinationNode=NULL;
                    std::map<string,ImageOutput*>::iterator it;

                    it=nodes.find(param->imageInputName);
                    
                    if(it!=nodes.end()){
                        it->second->updateParameter(param);
                        delete param;
                    }
                    maxParams += 1;
                }
                else{
                    continueProcessing = false;
                }
            }
        }

        for(int i=0; i<syphonServers.size();i++){
            syphonServers[i]->publishTexture();
        }
    }
}

void ofApp::draw() {
    
    //create bg
    ofClear(35);
    
    // draw nodes
    if(loadingOK){
        nodeViewers[currentViewer]->draw();
        
        string info = "";
        info += "  (";
        info += ofToString(currentViewer+1);
        info += "/";
        info += ofToString(nodeViewers.size());
        info += ")";
        info += "    switch layers <- ->";
        
        ofDrawBitmapString(info, 20, ofGetHeight()-20);
        string info2 = nodeViewers[currentViewer]->getName();
        
        ofDrawBitmapString(ofToString(info2,0), ofGetWidth() - 300, ofGetHeight()-20);
        ofDrawBitmapString(ofToString(ofGetFrameRate(),0), ofGetWidth() - 50, ofGetHeight()-35);
    }
    else{
        ofDrawBitmapString("ERROR LOADING XML", 50, 50);
    }
}

void ofApp::updateSyphon(ofFbo & img){
    if(loadingOK){
        
    }
}

bool ofApp::loadFromXML(){
    
    bool result = true;
    string message = "";
    
    if( XML.loadFile("appSettings.xml") ){
        
        int numMainSettingsTag = XML.getNumTags("MAIN_SETTINGS");
        
        if(numMainSettingsTag==1){
            XML.pushTag("MAIN_SETTINGS");
            int numSettingsTag = XML.getNumTags("SETTINGS");
            
            if(numSettingsTag==1){
                
                XML.pushTag("SETTINGS");
                
                int numInputTag = XML.getNumTags("INPUTS");
                
                // LOADING INPUTS
                
                if(numInputTag==1){
                    XML.pushTag("INPUTS");
                    int numInputTag = XML.getNumTags("INPUT");
                    for(int i=0; i<numInputTag; i++){
                        string inputName = XML.getAttribute("INPUT","name","default",i);
                        string inputType = XML.getAttribute("INPUT","type","CAM",i);
                        
                        switch(inputTypes[inputType]){
                            case VIDEO:
                            {
                                VideoPlayerMac* vP = new VideoPlayerMac(inputName);
                                
                                XML.pushTag("INPUT",i);
                                
                                int numVideoTag = XML.getNumTags("VIDEO");
                                
                                if(numVideoTag>0){
                                    for (int v=0; v<numVideoTag; v++){
                                        string path = XML.getAttribute("VIDEO","path","default",v);
                                        vP->loadVideo(path);
                                    }
                                    inputs.push_back(vP);
                                    nodes.insert(std::pair<string,ImageOutput*>(inputName,vP));
                                }
                                else{
                                    result = false;
                                    message = "no videos to be loaded!";
                                }
                                
                                XML.popTag();
                                
                                break;
                            };
                            case CAM:
                            {
                                InputCamera* iC = new InputCamera(inputName);
                                
                                //not used yet
                                string cameraId = XML.getAttribute("INPUT", "id","default", i);
                                
                                inputs.push_back(iC);
                                nodes.insert(std::pair<string,ImageOutput*>(inputName,iC));
                                
                                break;
                            };
                            case IMAGE:
                            {
                                //ImageInput* iI = new ImageInput(inputName);
                                
                                ImageInputList* iI = new ImageInputList(inputName);
                                string  path = XML.getAttribute("INPUT", "path","none", i);
                                
                                float bpm         = ofToFloat(XML.getAttribute("INPUT", "bpm","120", i));
                                int bpmMultiplier = ofToInt(XML.getAttribute("INPUT", "multiplier_divider","32", i));
                                bool isPlaying    = ofToBool(XML.getAttribute("INPUT", "isPlaying","true", i));
                                bool isPalindromLoop    = ofToBool(XML.getAttribute("INPUT", "palindrom","true", i));
                                bool isMatchBpmToSequenceLength    = ofToBool(XML.getAttribute("INPUT", "matchBPMtoSequence","false", i));
                                
                                if (path == "none") {
                                    XML.pushTag("INPUT",i);
                                    int numVideoTag = XML.getNumTags("ASSET");
                                    if(numVideoTag>0){
                                        for (int v=0; v<numVideoTag; v++){
                                            string name = XML.getAttribute("ASSET","name","default",v);
                                            string path_ = XML.getAttribute("ASSET","path","default",v);
                                            iI->loadImage(name,path_);
                                        }
                                        
                                    }
                                    else{
                                        result = false;
                                        message = "no videos to be loaded!";
                                    }
                                    XML.popTag();
                                }
                                else{
                                    iI->loadImage(inputName, path);
                                }
                                
                                iI->bpm = bpm;
                                iI->bpmMultiplier = bpmMultiplier;
                                iI->isPlaying = isPlaying;
                                iI->isPalindromLoop = isPalindromLoop;
                                iI->isMatchBpmToSequenceLength = isMatchBpmToSequenceLength;
                                //iI->setParameters(bpm);
                                
                                inputs.push_back(iI);
                                nodes.insert(std::pair<string,ImageOutput*>(inputName,iI));
                                
                                break;
                            };
                            case PARTICLE:
                            {
                                ParticleGenerator* iI = new ParticleGenerator(inputName);
                                
                                bool isClearBg          = ofToBool(XML.getAttribute("INPUT", "isClearBg","true", i));
                                float alphaParticles    = ofToFloat(XML.getAttribute("INPUT", "alphaParticles","0.4", i));
                                bool autoGenParticle    = ofToBool(XML.getAttribute("INPUT", "autoGenParticle","false", i));
                                float autoGenAmount     = ofToFloat(XML.getAttribute("INPUT", "autoGenAmount","0.0", i));
                                bool unityScale         = ofToBool(XML.getAttribute("INPUT", "unityScale","false", i));
                                float minRadius         = ofToFloat(XML.getAttribute("INPUT", "minRadius","4", i));
                                float maxRadius         = ofToFloat(XML.getAttribute("INPUT", "maxRadius","10", i));
                                float minLifetime       = ofToFloat(XML.getAttribute("INPUT", "minLifetime","0", i));
                                float maxLifetime       = ofToFloat(XML.getAttribute("INPUT", "maxLifetime","0", i));
                                
                                iI->isClearBg = isClearBg;
                                iI->alphaParticles = alphaParticles;
                                iI->autoGenParticle = autoGenParticle;
                                iI->autoGenAmount = autoGenAmount;
                                iI->unityScale = unityScale;
                                iI->minRadius = minRadius;
                                iI->maxRadius = maxRadius;
                                iI->minLifetime = minLifetime;
                                iI->maxLifetime = maxLifetime;
                                
                                
                                inputs.push_back(iI);
                                nodes.insert(std::pair<string,ImageOutput*>(inputName,iI));
                                
                                break;
                            };
                            default:
                            {
                                result = false;
                                message = "unknown input type!";
                                break;
                            };
                        }
                        
                        if(!result){
                            //there has been an error
                            //exit the loop
                            break;
                        }
                        
                    }
                    XML.popTag();
                    
                }
                else{
                    message = "inputs tag missing";
                    result = false;
                }
                
                //LOADING VISUAL LAYERS
                
                if(result){
                    int numVLsTag = XML.getNumTags("VISUAL_LAYERS");
                    if(numVLsTag==1){
                        
                        XML.pushTag("VISUAL_LAYERS");
                        
                        int numVLTag = XML.getNumTags("VISUAL_LAYER");
                        
                        for(int i=0; i < numVLTag; i++){
                            string layerName = XML.getAttribute("VISUAL_LAYER","name","default",i);
                            string layerType = XML.getAttribute("VISUAL_LAYER","type","IKEDA",i);
                            string inputSourceName = XML.getAttribute("VISUAL_LAYER","inputSource","default",i);
                            
                            switch(visualLayerTypes[layerType]){
                                case IKEDA:
                                {
                                    bool isCanny = ofToBool(XML.getAttribute("VISUAL_LAYER","isCanny","true",i));
                                    bool isThreshold = ofToBool(XML.getAttribute("VISUAL_LAYER","isThreshold","true",i));
                                    bool isColumns = ofToBool(XML.getAttribute("VISUAL_LAYER","isColumns","true",i));
                                    bool isInvert = ofToBool(XML.getAttribute("VISUAL_LAYER","isInvert","true",i));
                                    int pNColumns = ofToInt(XML.getAttribute("VISUAL_LAYER","pNColumns","4",i));
                                    int pCannyX = ofToInt(XML.getAttribute("VISUAL_LAYER","pCannyX","12",i));
                                    int pCannyY = ofToInt(XML.getAttribute("VISUAL_LAYER","pCannyY","12",i));
                                    int pThreshold = ofToInt(XML.getAttribute("VISUAL_LAYER","pThreshold","12",i));
                                    
                                    IkedaLayer* iL = new IkedaLayer(layerName);
                                    
                                    iL->isCanny = isCanny;
                                    iL->isThreshold = isThreshold;
                                    iL->isColumns = isColumns;
                                    iL->isInvert = isInvert;
                                    iL->pNColumns = pNColumns;
                                    iL->pCannyX = pCannyX;
                                    iL->pCannyY = pCannyY;
                                    iL->pThreshold = pThreshold;
                                    
                                    
                                    iL->addInputIdentifier(inputSourceName);
                                    
                                    
                                    visualLayers.push_back(iL);
                                    nodes.insert(std::pair<string,ImageOutput*>(layerName,iL));
                                    
                                    break;
                                };
                                case GLITCH_1:
                                {
                                    bool do_CONVERGENCE = ofToBool(XML.getAttribute("VISUAL_LAYER","do_CONVERGENCE","false",i));
                                    bool do_GLOW = ofToBool(XML.getAttribute("VISUAL_LAYER","do_GLOW","false",i));
                                    bool do_SHAKER = ofToBool(XML.getAttribute("VISUAL_LAYER","do_SHAKER","false",i));
                                    bool do_CUTSLIDER = ofToBool(XML.getAttribute("VISUAL_LAYER","do_CUTSLIDER","false",i));
                                    bool do_TWIST = ofToBool(XML.getAttribute("VISUAL_LAYER","do_TWIST","false",i));
                                    bool do_OUTLINE = ofToBool(XML.getAttribute("VISUAL_LAYER","do_OUTLINE","false",i));
                                    bool do_NOISE = ofToBool(XML.getAttribute("VISUAL_LAYER","do_NOISE","false",i));
                                    bool do_SLITSCAN = ofToBool(XML.getAttribute("VISUAL_LAYER","do_SLITSCAN","false",i));
                                    bool do_SWELL = ofToBool(XML.getAttribute("VISUAL_LAYER","do_SWELL","false",i));
                                    bool do_INVERT = ofToBool(XML.getAttribute("VISUAL_LAYER","do_INVERT","false",i));
                                    
                                    bool do_CR_HIGHCONTRAST = ofToBool(XML.getAttribute("VISUAL_LAYER","do_CR_HIGHCONTRAST","false",i));
                                    bool do_CR_BLUERAISE = ofToBool(XML.getAttribute("VISUAL_LAYER","do_CR_BLUERAISE","false",i));
                                    bool do_CR_REDRAISE = ofToBool(XML.getAttribute("VISUAL_LAYER","do_CR_REDRAISE","false",i));
                                    bool do_CR_GREENRAISE = ofToBool(XML.getAttribute("VISUAL_LAYER","do_CR_GREENRAISE","false",i));
                                    bool do_CR_BLUEINVERT = ofToBool(XML.getAttribute("VISUAL_LAYER","do_CR_BLUEINVERT","false",i));
                                    bool do_CR_REDINVERT = ofToBool(XML.getAttribute("VISUAL_LAYER","do_CR_REDINVERT","false",i));
                                    bool do_CR_GREENINVERT = ofToBool(XML.getAttribute("VISUAL_LAYER","do_CR_GREENINVERT","false",i));
                                    
                                    GlitchLayer* gL = new GlitchLayer(layerName);
                                    
                                    gL->do_CONVERGENCE = do_CONVERGENCE;
                                    gL->do_GLOW = do_GLOW;
                                    gL->do_SHAKER = do_SHAKER;
                                    gL->do_CUTSLIDER = do_CUTSLIDER;
                                    gL->do_TWIST = do_TWIST;
                                    gL->do_OUTLINE = do_OUTLINE;
                                    gL->do_NOISE = do_NOISE;
                                    gL->do_SLITSCAN = do_SLITSCAN;
                                    gL->do_SWELL = do_SWELL;
                                    gL->do_INVERT = do_INVERT;
                                    
                                    gL->do_CR_HIGHCONTRAST = do_CR_HIGHCONTRAST;
                                    gL->do_CR_BLUERAISE = do_CR_BLUERAISE;
                                    gL->do_CR_REDRAISE = do_CR_REDRAISE;
                                    gL->do_CR_GREENRAISE = do_CR_GREENRAISE;
                                    gL->do_CR_BLUEINVERT = do_CR_BLUEINVERT;
                                    gL->do_CR_REDINVERT = do_CR_REDINVERT;
                                    gL->do_CR_GREENINVERT = do_CR_GREENINVERT;
                                    
                                    gL->addInputIdentifier(inputSourceName);

                                    visualLayers.push_back(gL);
                                    nodes.insert(std::pair<string,ImageOutput*>(layerName,gL));
                                    
                                    break;
                                };
                                case GLITCH_2:
                                {
                                    int dq = ofToInt(XML.getAttribute("VISUAL_LAYER","dq","20",i));
                                    int qn = ofToInt(XML.getAttribute("VISUAL_LAYER","qn","40",i));
                                    int dht = ofToInt(XML.getAttribute("VISUAL_LAYER","dht","80",i));
                                    
                                    GlitchLayerAlt* gLA = new GlitchLayerAlt(layerName);
                                    
                                    
                                    gLA->addInputIdentifier(inputSourceName);
                                    
                                    visualLayers.push_back(gLA);
                                    nodes.insert(std::pair<string,ImageOutput*>(layerName,gLA));
                                    
                                    break;
                                };
                                case IMAGE_PROCESSOR:
                                {
                                    
                                    ImageProcessor* gLA = new ImageProcessor(layerName);
                                    
                                    
                                    gLA->addInputIdentifier(inputSourceName);

                                    
                                    visualLayers.push_back(gLA);
                                    nodes.insert(std::pair<string,ImageOutput*>(layerName,gLA));
                                    
                                    break;
                                };
                                default:
                                {
                                    result = false;
                                    message = "unknown visual layer type!";
                                    break;
                                };
                            }
                            
                            if(!result){
                                //there has been an error
                                //exit the loop
                                break;
                            }

                        }
                        
                        XML.popTag();
                    }
                    else{
                        message = "visual layers tag missing";
                        result = false;
                    }
                }
                
                //LOADING MIXERS
                if(result){
                    int numMXsTag = XML.getNumTags("MIXERS");
                    if(numMXsTag==1){
                        XML.pushTag("MIXERS");
                        
                        int numMXTag = XML.getNumTags("MIXER");
                        
                        for(int i = 0; i < numMXTag; i++){
                            string name = XML.getAttribute("MIXER","name","default",i);
                            string type = XML.getAttribute("MIXER","type","SIMPLE_BLEND", i);
                            
                            switch(mixerTypes[type]){
                                case SIMPLE_BLEND:
                                {
                                    int selectorL = ofToInt(XML.getAttribute("MIXER","selectorLeft","0", i));
                                    int selectorR = ofToInt(XML.getAttribute("MIXER","selectorRight","0", i));
                                    int blendmode = ofToInt(XML.getAttribute("MIXER","blendmode","0", i));
                                    float opacity = ofToFloat(XML.getAttribute("MIXER","opacity","0", i));
                                    

                                    MixSimpleBlend* mSB = new MixSimpleBlend(name);
                                    XML.pushTag("MIXER",i);
                                    
                                    int numINPUTTag = XML.getNumTags("INPUT_SOURCE");
                                    
                                    for(int j=0; j<numINPUTTag; j++){
                                        
                                        string inputName = XML.getAttribute("INPUT_SOURCE","name","default",j);
                                        
                                        mSB->addInputIdentifier(inputName);
                                        
                                    }
                                    mSB->selector1 = selectorL;
                                    mSB->selector2 = selectorR;
                                    mSB->opacity = opacity;
                                    mSB->blendMode = blendmode;
                                    
                                    mixtables.push_back(mSB);
                                    nodes.insert(std::pair<string, ImageOutput*>(name, mSB));
                                    //MIXER POP
                                    XML.popTag();
                                    
                                    break;
                                };
                                case MASK:
                                {
                                    MixMask* mMM = new MixMask(name);
                                    XML.pushTag("MIXER",i);
                                    
                                    int numINPUTTag = XML.getNumTags("INPUT_SOURCE");
                                    std::map<string,ImageOutput*>::iterator it;
                                    
                                    for(int j=0; j<numINPUTTag; j++){
                                        string inputName = XML.getAttribute("INPUT_SOURCE","name","default",j);
                                        
                                        mMM->addInputIdentifier(inputName);
                                        
                                    }
                                    
                                    mixtables.push_back(mMM);
                                    nodes.insert(std::pair<string, ImageOutput*>(name, mMM));
                                    //MIXER POP
                                    XML.popTag();
                                    
                                    break;
                                };
                                case MULTI_CHANNEL:
                                {
                                    MultiChannelSwitch* mMM = new MultiChannelSwitch(name);
                                    XML.pushTag("MIXER",i);
                                    
                                    int numINPUTTag = XML.getNumTags("INPUT_SOURCE");
                                    std::map<string,ImageOutput*>::iterator it;
                                    int selChannel =  ofToInt(XML.getAttribute("INPUT_SOURCE","selChannel","0",i));
                                    for(int j=0; j<numINPUTTag; j++){
                                        string inputName = XML.getAttribute("INPUT_SOURCE","name","default",j);
                            
                                        it = nodes.find(inputName);
                                        
                                        if(it!=nodes.end()){
                                            /*ImageOutput * iO = it->second;
                                            
                                            mMM->addInput(iO);*/
                                            string inputName = XML.getAttribute("INPUT_SOURCE","name","default",j);
                                            
                                            mMM->addInputIdentifier(inputName);
                                        }
                                        else{
                                            result = false;
                                            message = "node not foud!";
                                        }
                                        
                                    }
                                    mMM->selChannel = selChannel;
                                    mixtables.push_back(mMM);
                                    nodes.insert(std::pair<string, ImageOutput*>(name, mMM));
                                    //MIXER POP
                                    XML.popTag();
                                    
                                    break;
                                };
                                default:
                                {
                                    result = false;
                                    message = "unknown mixer type!";
                                    break;
                                };
                                    
                            }
                            
                            if(!result){
                                //there has been an error
                                //exit the loop
                                break;
                            }
                            
                        }
                        //MIXERS POP
                        XML.popTag();

                    }

                }
                else{
                    message = "mixers tag missing";
                    result = false;
                }
                //SETTINGS POP
                XML.popTag();
            }
            else{
                result = false;
                message = "missing SETTINGS tag!";
            }
            
            if(result){
                //PROCESSING NODE_VIEWS
                
                int numNodeViews = XML.getNumTags("NODE_VIEWS");
                
                if(numNodeViews==1){
                    
                    XML.pushTag("NODE_VIEWS");
                    
                    int numNodeView = XML.getNumTags("NODE_VIEW");
                    
                    for(int i = 0; i < numNodeView; i++){
                        string nodeViewName = XML.getAttribute("NODE_VIEW","name","default",i);
                        
                        NodeViewer* nV = new NodeViewer(nodeViewName);
                        
                        XML.pushTag("NODE_VIEW",i);
                        int numNODETag = XML.getNumTags("NODE");
                        
                        for(int j=0; j<numNODETag; j++){
                            string nodeName = XML.getAttribute("NODE","name","default",j);
                            
                            int x = ofToInt(XML.getAttribute("NODE","x","20",j));
                            int y = ofToInt(XML.getAttribute("NODE","y","20",j));
                            int guiX = ofToInt(XML.getAttribute("NODE","guiX","20",j));
                            int guiY = ofToInt(XML.getAttribute("NODE","guiY","20",j));
                            int guiWidth = ofToInt(XML.getAttribute("NODE","guiWidth","120",j));
                            float imageScale = ofToFloat(XML.getAttribute("NODE","imageScale","1",j));
                            
                            std::map<string,ImageOutput*>::iterator it;
                            
                            it=nodes.find(nodeName);
                            
                            if(it!=nodes.end()){
                                ImageOutput* iO = it->second;
                                NodeElement* nE = new NodeElement(iO, x, y, guiX, guiY, guiWidth, imageScale);
                                
                                nV->addElement(nE);
                            }
                            else{
                                result = false;
                                message = "node not foud!";
                            }
                            
                        }
                        
                        nodeViewers.push_back(nV);
                        
                        //NODE_VIEW
                        XML.popTag();
                        
                    }
                    
                    //NODE_VIEWS POP
                    XML.popTag();
                    
                }
                else{
                    result = false;
                    message = "missing NODE_VIEWS tag!";
                }
                
                // PROCESSING PARAM INPUT GENERATORS
                
                if (result){
                    
                    int numParamInputs = XML.getNumTags("PARAM_INPUT_GENERATORS");
                    
                    
                    if(numParamInputs==1){
                        
                        XML.pushTag("PARAM_INPUT_GENERATORS");

                        int numInputGen = XML.getNumTags("INPUT_GEN");
                        
                        for(int j=0; j<numInputGen; j++){
                            string inputName = XML.getAttribute("INPUT_GEN","name","default",j);
                            string inputType = XML.getAttribute("INPUT_GEN","type","MIDI",j);
                            string midiDeviceName = XML.getAttribute("INPUT_GEN","midiDeviceName","Oxygen 25",j);
                            switch(inputGenTypes[inputType]){
                                case MIDI:
                                {
                                    MidiInputGenerator* mI = new MidiInputGenerator(inputName,midiDeviceName);
                                    inputGenerators.push_back(mI);
                                    
                                    break;
                                }
                                case FFT:
                                {
                                    AudioInputGenerator* aI = new AudioInputGenerator(inputName);
                                    inputGenerators.push_back(aI);
                                    audioListeners.push_back(aI);
                                    
                                    break;
                                }
                                case OSC:
                                {
                                    OscInputGenerator* oI = new OscInputGenerator(inputName);
                                    inputGenerators.push_back(oI);
                                    
                                    break;
                                }
                                default:
                                {
                                    result = false;
                                    message = "unknown mixer type!";
                                    break;
                                };
                                    
                            }
                            
                        }
                        
                        //Popping PARAM_INPUT_GENERATORS tags
                        XML.popTag();

                    }
                    else{
                        result = false;
                        message = "missing PARAM_INPUT_GENERATORS tag!";
                    }
                    
                }
                
                //PROCESSING SYPHON SERVERS
                if (result){
                    int numServers = XML.getNumTags("SYPHON_SERVERS");
                    
                    
                    if(numServers==1){
                        XML.pushTag("SYPHON_SERVERS");
                        
                        int numSyphonServer = XML.getNumTags("SERVER");
                        
                        for(int j=0; j<numSyphonServer; j++){
                            string inputName = XML.getAttribute("SERVER","inputName","mainMix",j);
                            string exportName = XML.getAttribute("SERVER","exportName","syphon1",j);
                            
                            std::map<string,ImageOutput*>::iterator it;
                            
                            it=nodes.find(inputName);
                            
                            if(it!=nodes.end()){
                                ImageOutput* iO = it->second;
                                CustomSyphonServer* cSS = new CustomSyphonServer(exportName,iO);
                                
                                syphonServers.push_back(cSS);
                                
                            }
                            else{
                                result = false;
                                message = "node not foud!";
                                break;
                            }
                            
                        }
                        
                        //Popping SYPHON_SERVERS tags
                        XML.popTag();
                        
                    }
                    else{
                        result = false;
                        message = "missing SYPHON_SERVERS tag!";
                    }
                    
                }
            }
        }
        else{
            result = false;
            message = "missing MAIN_SETTINGS tag!";
        }
        
    }else{
        
        //file not loaded
        message = "file not loaded!";
        result = false;
        
    }
    
    if(!result){
        ofLogNotice() << message;
    }
    else{
        //populating the nodes vector for fast iteration in the main loop.
        for(int i=0; i<inputs.size();i++){
            nodesVector.push_back(inputs[i]);
        }
        for(int i=0; i<visualLayers.size();i++){
            nodesVector.push_back(visualLayers[i]);
        }
        for(int i=0; i<mixtables.size();i++){
            nodesVector.push_back(mixtables[i]);
        }
        
        //setting inputs to every node
        bool error = false;
        for(int i=0; i<nodesVector.size(); i++){
            error = nodesVector[i]->findAndAssignInputs(nodes);
            if(error){
                message = "node not found";
                result = false;
                break;
            }
        }
        
        //create connections in nodeView
        for (int i=0; i<nodeViewers.size(); ++i) {
            nodeViewers[i]->createConnections();
        }
        
    }
    
    return result;

}

void ofApp::keyPressed  (int key){
    bool notAvailable = false;
            switch(key){
                //switch Node Views
                case OF_KEY_LEFT:
                    previousViewer();
                    break;
                case OF_KEY_RIGHT:
                    nextViewer();
                    break;
                case '1':
                    setCurrentViewer(0);
                    break;
                case '2':
                    setCurrentViewer(1);
                    break;
                case '3':
                    setCurrentViewer(2);
                    break;
                case '4':
                    setCurrentViewer(3);
                    break;
                case '5':
                    setCurrentViewer(4);
                    break;
                case '6':
                    setCurrentViewer(5);
                    break;
                case '7':
                    setCurrentViewer(6);
                    break;
                case '8':
                    setCurrentViewer(7);
                    break;
                case '9':
                    setCurrentViewer(8);
                    break;
                case '0':
                    setCurrentViewer(9);
                    break;
                case 'f':
                    isFullScreen = !isFullScreen;
                    ofSetFullscreen(isFullScreen);
                    break;
                default:
                    notAvailable = true;
                    break;
            }
    if (notAvailable) ofLogNotice() << "key function not available";
    
    // sending keyboard input to all input generators
    for (int i = 0; i<inputGenerators.size();i++){
        inputGenerators[i]->keyPressed(key);
    }
    
}

void ofApp::nextViewer(){
    int tViewer = currentViewer;
    ++tViewer;
    tViewer %= nodeViewers.size();
    setCurrentViewer(tViewer);
}
void ofApp::previousViewer(){
    int tViewer = currentViewer;
    tViewer = (tViewer == 0) ? nodeViewers.size()-1 : --tViewer;
    setCurrentViewer(tViewer);
}

void ofApp::setCurrentViewer(int currentViewer_){
    if (currentViewer_ >= 0 && currentViewer_ < nodeViewers.size()) {
        currentViewer = currentViewer_;
        //reset the gui positions
        nodeViewers[currentViewer]->setupGuiPositions();
    }else{
        ofLogNotice() << "choosen viewer not available";
    }
}
