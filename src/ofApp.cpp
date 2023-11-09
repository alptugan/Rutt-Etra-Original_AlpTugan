#include "ofApp.h"


// things to do
// work on fft
// smoothing lines
// fast video switch?

//--------------------------------------------------------------
void ofApp::setup(){
    //ofEnableSmoothing();
    //ofEnableSetupScreen();
    //ofSetVerticalSync(true);
    // toggle fullscreen
   // ofToggleFullscreen();
    //ofSetFrameRate(2);
    ofSetWindowPosition(0,0);
    ofSetWindowShape(1800,1012);

    // Enable or Disable Debug Mode
    isDebug = true;

    
    // Hide mouse cursor, if "isDebug" enabled
    if(!isDebug)
        ofHideCursor();
    
    // Enable or Disable Mesh Gray scale
    isWhiteColor    = false;
    
    // Switch MESH drawing mode to POINTS / LINES
    isPointMode     = false;
    
    // Switch input source mode
    mode = VIDEO;
    folderPath = "ISP_haydarpasa";
    soundFilePath.set("file path", "sounds/ISP_arkaoda_2.wav");

    // GUI OPTIONS
    ofxGuiSetFont( "../../../../../../assets/fonts/DIN.otf", 8 );
    ofxGuiSetDefaultWidth( 300 );
    ofxGuiSetFillColor(ofColor(255,204,0,200));
    
    string xmlSettingsPath = "RuttEtra_Settings.xml";
    gui.setup( "RuttEtra_Options", xmlSettingsPath );
    gui.setPosition(ofGetWidth() - 310,10);
    
    gui.add(isPostGlitchEnabled.set("POST GLITCH FX", true));
    gui.add(yStep.set("Vertical Vertex Distance", 5,1,200));
    gui.add(xStep.set("Horizontal Vertex Distance", 2,1,200));
    gui.add(zMult.set("Z-Depth Vertex",0.3,0,4));
    gui.add(soundThresMult.set("sound Multiplier Fac", 0.5, 0, 1.));
    gui.add(pLineThickness.set("Mesh Line Thickness",1,1,20));
    
    gui.add(isShowFrame.set("Show Mesh Frame", true));
    gui.add(isWhiteColor.set("Set Color",false));
    gui.add(isPointMode.set("POINT Mesh Mode",false));
    gui.add(isVideoSoundEnabled.set("Enable Video Sound", false));
    gui.add(isSoundReactive.set("Sound Reactive Mode", true));

    isVideoSoundEnabled.addListener(this, &ofApp::setVideoSoundEnabled);
    soundLevel.addListener(this, &ofApp::setSoundLevel);
    isSoundPlayPause.addListener(this, &ofApp::setPaused);
    
    // SOUND GUI
    soundGUI.setName("Sound Player");
    soundGUI.add(soundFilePath);
    soundGUI.add(isExtSoundEnabled.set("Load External Sound", false));
    soundGUI.add(isSoundPlayPause.set("PLAY/PAUSE",false));
    soundGUI.add(soundLevel.set("Volume", 0.5, 0.0, 1.0));
    gui.add(soundGUI);
    
    // FX GUI
    fxTypes.setName("FX Types");
    fxTypes.add(glowAmount.set("Glow Effect Amount", 8, 0, 40));
    fxTypes.add(converge.set("Converge",false));
    fxTypes.add(glow.set("Glow",false));
    fxTypes.add(shaker.set("Shaker",false));
    fxTypes.add(cut.set("Cut Slider",false));
    fxTypes.add(twist.set("Twist",false));
    fxTypes.add(outline.set("Outline",false));
    fxTypes.add(noise.set("Noise",false));
    fxTypes.add(slitscan.set("Slitscan",false));
    fxTypes.add(swell.set("Swell",false));
    fxTypes.add(invert.set("Invert",false));
    fxTypes.add(highContrast.set("High Contrast",false));
    fxTypes.add(blueish.set("Blue Raise",false));
    fxTypes.add(greenish.set("Green Raise",false));
    fxTypes.add(redish.set("Red Raise",false));
    fxTypes.add(blueInv.set("Blue Invert",false));
    fxTypes.add(redInv.set("Red Invert",false));
    fxTypes.add(greenInv.set("Green Invert",false));
    gui.add(fxTypes);
    
    
    
#ifdef KORG_ENABLED
    // Korg GUI
    KorgGui.setName("Korg Controller Options");
    KorgGui.add(KorgDebug.set("Korg Debug", false));

    gui.add(KorgGui);
    
    // KORG NANO MIDI CONTROLLER PARAMETERS
    nano.getListMidiDevices();
    nano.setup(true);
    
    ofAddListener(nano.sceneButtonPressed, this, &ofApp::sceneButtonPressed);
    ofAddListener(nano.sliderValChanged, this, &ofApp::korgSliderChanged);
    ofAddListener(nano.pushButtonPressed, this, &ofApp::korgButtonPressed);
    ofAddListener(nano.potValChanged, this, &ofApp::korgPotChanged);
#endif

    // Initialize image
    if(mode == IMAGE)
        setupImages(folderPath);
    
    // Initialize video grabber
    if(mode == CAM) {
        camWidth         =  320;    // try to grab at this size.
        camHeight        =  240;
        vidGrabber.initGrabber(camWidth,camHeight);
    }
    
    
    // VIDEO::Initialize Video Player
    if(mode == VIDEO) {
        setupVideos(folderPath+"/");
    }
    
    
    // SOUND ANALYZE FFT
    //fft.setup();
    //fft.setNumFFTBins(32);
    
    
    // POST EEFECT (FX) SECTION
    // Create fbo to apply shader FX effects
    fbo.allocate(ofGetWidth(),ofGetHeight(),GL_RGBA,4);
    fbo.begin();
    ofClear(0,0);
    fbo.end();
    
    fx.setup(&fbo);

    fxManager.setup(ofGetWidth(),ofGetHeight(),"",10);
    fxManager.setFlip(false);
    fxManager.loadSettings();
    
    // Default parameter values
    defCamPos = cam.getPosition();
    
  
    // SETUP CAMERA SAVE & LOAD to load selected camera properties
    setupCameraSaveLoad();
    

    // GUI group to minimize or maximize
    // LOAD SETUP DEFAULTS
    //gui.getGroup("Sound Player").minimize();
    gui.getGroup("FX Types").minimize();
    gui.loadFromFile(xmlSettingsPath);


    // SOUND::Load Sound
    // When using timeline we don't need external sound.
    // For live performance you can enable it. OR leave it for
    if(isExtSoundEnabled)
    {
        /*mp3.load(soundFilePath.get());
        mp3.play();
        mp3.setPaused(!isSoundPlayPause);*/
    }
    
    // SETUP TOOLKITS
#ifdef ENABLE_SAVE
    isSaveEnabled = false;
    tools.setupRecord(1,"export");
#endif

    // TIMELINE INIT
    initTimeline();


}

void ofApp::setPaused(bool & val) {
    //isSoundPlayPause = !val;
    mp3.setPaused(!isSoundPlayPause);
}

void ofApp::setVideoSoundEnabled(bool & val) {
    float vol = 0;
    if(val) {
        vol = soundLevel;
    }else{
        vol = 0;
    }
    for (int i = 0; i < vidPlayer.size(); i++) {
        vidPlayer[i].setVolume(vol);
    }
}

void ofApp::setSoundLevel(float & val) {
    soundLevel = val;
    mp3.setVolume(soundLevel);
}

void ofApp::setupImages(string _folder) {
    images.clear();
    dirVidStr = _folder;
    //dirVidStr = "ISP_haydarpasa";
    dirVid.listDir(dirVidStr);
    dirVid.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order
    
    //allocate the vector to have as many ofImages as files
    if( dirVid.size() ){
        images.assign(dirVid.size(), ofImage());
    }
    
    // you can now iterate through the files and load them into the ofImage vector
    for(int i = 0; i < (int)dirVid.size(); i++){
        images[i].load(dirVid.getPath(i));
    }
    idVid = 0;
    
    // Default video parameters to set pivot point to center
    setTranslationPoints();
}

void ofApp::setupVideos(string _str)
{
    vidPlayer.clear();
    dirVidStr = _str;
    //dirVidStr = "ISP_haydarpasa";
    dirVid.allowExt("mov");
    dirVid.allowExt("mp4");
    dirVid.listDir(dirVidStr + "/");
    dirVid.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order
    
    //allocate the vector to have as many ofImages as files
    if( dirVid.size() ){
        vidPlayer.assign(dirVid.size(), ofVideoPlayer());
    }
    
    // you can now iterate through the files and load them into the ofImage vector
    for(int i = 0; i < (int)dirVid.size(); i++){
        vidPlayer[i].load(dirVid.getPath(i));
        bool enabl = isVideoSoundEnabled;
        setVideoSoundEnabled(enabl);
    }
    idVid = 0;
    
    // Default video parameters to set pivot point to center
    setTranslationPoints();
}

void ofApp::setTranslationPoints() {
    if(mode == VIDEO) {
        curTransX = -vidPlayer[idVid].getWidth()*0.5;
        curTransY = -vidPlayer[idVid].getHeight()*0.5;
    }else if( mode == IMAGE) {
        curTransX = -images[idVid].getWidth()*0.5;
        curTransY = -images[idVid].getHeight()*0.5;
    }else if( mode == CAM) {
        
    }
}

//--------------------------------------------------------------
// UPDATE : MAIN
//--------------------------------------------------------------
void ofApp::update(){
    // Update FFT sound input analyze
    if(isSoundReactive) {
        //fft.update();
    }
    
    // Sound Player
    if(isExtSoundEnabled) {
        //ofSoundUpdate();
    }
    
    // Clear mesh
    mesh.clear();
    
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));

    // update camera Move
    saveCam.update();
#ifdef KORG_ENABLED
    contTrans.x = ofMap(nano.getVal(K_SLIDER_7),0,127,-3000,3000,true);
    contTrans.y = ofMap(nano.getVal(K_SLIDER_8),0,127,-3000,3000,true);
    contTrans.z = ofMap(nano.getVal(K_SLIDER_9),0,127,1000,-10000,true);
    
    contRot.x = ofMap(nano.getVal(K_POT_7,K_TYPE_POT,K_SCENE_1),0,127,-180,180);
    contRot.y = ofMap(nano.getVal(K_POT_8,K_TYPE_POT,K_SCENE_1),0,127,0,360);
    contRot.z = ofMap(nano.getVal(K_POT_9,K_TYPE_POT,K_SCENE_1),0,127,0,360);

    yStep = int(ofMap(nano.getVal(K_SLIDER_2),0,127,2,150));
    xStep = int(ofMap(nano.getVal(K_SLIDER_1),0,127,2,150));
    /*
    pLineThickness = int(ofMap(nano.getVal(K_SLIDER_3),0,127,2,12));
    */
    isPointMode = nano.getVal(K_BUTTON_1,K_TYPE_BUTTON);
    isWhiteColor = nano.getVal(K_BUTTON_3,K_TYPE_BUTTON);
#endif
    
    // Update Method for shader effects
    if(isPostGlitchEnabled)
        updateFXParameters();
    else
        fxManager.updateValues();
    
    if(mode == CAM) {
        //vidGrabber.update();
    }
    
    // update the sound playing system:
    //ofSoundUpdate();
    
    
    //update pixels for mesh
    if(isPointMode) {
        mesh.setMode(OF_PRIMITIVE_POINTS);
    }else{
        //  ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_LINES);
    }
    
    if(mode == VIDEO) {
        if(dirVid.size() > 0) {
            vidPlayer[idVid].update();
            vidPlayerPx = vidPlayer[idVid].getPixels();
            
            int xy=0;
            for (int y = 0; y<vidPlayer[idVid].getHeight(); y+=yStep){
                
                
                for (int x = 0; x < vidPlayer[idVid].getWidth(); x += xStep){
                    
                    
                    if(!isWhiteColor) {
                        meshColor = vidPlayerPx.getColor(x, y);
                        mesh.addColor(ofColor(meshColor, 255)); // meshColor.getBrightness()
                    }else{
                        meshColor = vidPlayerPx.getColor(x, y).getBrightness();
                        mesh.addColor(ofColor(meshColor, 255));
                    }
                    
                    // Get mapped Frequency Response
                    float fftDepth = 0; //getMappedFreqResponse(xy, 0, vidPlayer[idVid].getHeight() * vidPlayer[idVid].getWidth(), 100, 15000);
                    float fftIntensity = 1;//fft.getIntensityAtFrequency(fftDepth);

                   
                    //cout << fft.getIntensityAtFrequency(fftDepth) << endl;
                    mesh.addVertex(glm::vec3(x, y, meshColor.getBrightness() * (zMult + fftIntensity * soundThresMult)));
                    //curColor.getBrightness() * .3 + scaledVol));
 
                    
                    if(x == 0) {
                        mesh.addIndex(xy);
                    }
                    
                    if(x != 0 && x != vidPlayer[idVid].getWidth()-xStep) {
                        mesh.addIndex(xy);
                        mesh.addIndex(xy);
                    }
                    
                    if(x >= vidPlayer[idVid].getWidth()-xStep) {
                        mesh.addIndex(xy);
                    }
                    xy = xy + 1;
                }
            }
        }else{
            cout << "There is no video file in the " << dirVidStr << " folder" << endl;
        }
    }
    
    
    
    
    int xy=0;
    if(mode == IMAGE  ) {
        vidPlayerPx = images[idVid].getPixels();
        for (int y = 0; y < images[idVid].getHeight(); y+=yStep){
            ofNoFill();
            
            
            for (int x = 0; x < images[idVid].getWidth(); x += xStep){
                
                meshColor = images[idVid].getColor(x, y);

                
                if(!isWhiteColor) {
                    meshColor = vidPlayerPx.getColor(x, y);
                    mesh.addColor(ofColor(meshColor, 255));
                }else{
                    meshColor = vidPlayerPx.getColor(x, y).getBrightness();
                    mesh.addColor(ofColor(meshColor, 255));
                }
                
                // Get mapped Frequency Response
                //float fftDepth = getMappedFreqResponse(xy, 0, images[idVid].getHeight() * images[idVid].getWidth(), 100, 15000);
                float fftDepth = 0; //getMappedFreqResponse(xy, 0, vidPlayer[idVid].getHeight() * vidPlayer[idVid].getWidth(), 100, 15000);
                float fftIntensity = 1;//fft.getIntensityAtFrequency(fftDepth);

               
                
                mesh.addVertex(glm::vec3(x, y, meshColor.getBrightness() * (zMult + fftIntensity * soundThresMult)));

                
                if(x == 0) {
                    mesh.addIndex(xy);
                }
                
                if(x != 0 && x != images[idVid].getWidth()-xStep) {
                    mesh.addIndex(xy);
                    mesh.addIndex(xy);
                }
                
                if(x >= images[idVid].getWidth()-xStep) {
                    mesh.addIndex(xy);
                }
                xy = xy + 1;
                
            }
        }
    }
}

void ofApp::initTimeline() {
    timeline.setup();
    if(isExtSoundEnabled) {
        timeline.addAudioTrack("audio", soundFilePath.get());
        timeline.setDurationInSeconds(timeline.getAudioTrack("audio")->getDuration());
    }

    // FX Switches
    timeline.addSwitches("fx");

    // FX Swtiches 2
    timeline.addSwitches("fx II");

    // Camera Track
    cameraTrack = new ofxTLCameraTrack();
    cameraTrack->setCamera(cam);
    timeline.addTrack("Camera", cameraTrack);
    cameraTrack->lockCameraToTrack = true;

    //ofAddListener(timeline.events().bangFired, this, &ofApp::bangFired);
    ofAddListener(timeline.events().switched, this, &ofApp::switchFired);
    ofAddListener(timeline.events().switched, this, &ofApp::switchFired2);
}

void ofApp::bangFired(ofxTLBangEventArgs& args) {
    cout << "bang fired!" << args.flag << endl;
}

//--------------------------------------------------------------
// OFXTIMELINE - Switch between post fx during the playback
//--------------------------------------------------------------
void ofApp::switchFired(ofxTLSwitchEventArgs& args) {
    //cout << "switch fired!" << args.switchName << "::" << args.on << endl;
    if(ofToInt(args.switchName) != 0)
        fxManager.setFX(ofToInt(args.switchName), args.on);
}

void ofApp::switchFired2(ofxTLSwitchEventArgs& args) {
    //cout << "switch fired!" << args.switchName << "::" << args.on << endl;
    if(args.switchName == "points") {
        isPointMode = args.on;
    }else if(args.switchName == "bw") {
        isWhiteColor = args.on;
    }else{
        if(ofToInt(args.switchName) != 0)
                fxManager.setFX(ofToInt(args.switchName), args.on);
    }
}


//--------------------------------------------------------------
// UPDATE : FX SHADER
//--------------------------------------------------------------
void ofApp::updateFXParameters() {
    // FX
    if(fx.getFx(OFXPOSTGLITCH_GLOW))
        fx.setBlurAmt(glowAmount);

#ifdef KORG_ENABLED
    (nano.getVal(K_BUTTON_2,K_TYPE_BUTTON)) ? converge = true : converge = false;
    (nano.getVal(K_BUTTON_4,K_TYPE_BUTTON)) ? cut = true : cut = false;
    (nano.getVal(K_BUTTON_6,K_TYPE_BUTTON)) ? glow = true : glow = false;
    (nano.getVal(K_BUTTON_8,K_TYPE_BUTTON)) ? invert = true : invert = false;
    (nano.getVal(K_BUTTON_10,K_TYPE_BUTTON)) ? shaker = true : shaker = false;
    (nano.getVal(K_BUTTON_12,K_TYPE_BUTTON)) ? slitscan = true : slitscan = false;
    (nano.getVal(K_BUTTON_14,K_TYPE_BUTTON)) ? twist = true : twist = false;
    (nano.getVal(K_BUTTON_16,K_TYPE_BUTTON)) ? swell = true : swell = false;
#endif
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_CONVERGENCE, converge);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_CR_BLUEINVERT, blueInv);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_CR_BLUERAISE, blueish);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_CR_GREENINVERT, greenInv);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_CR_GREENRAISE, greenish);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_CR_HIGHCONTRAST, highContrast);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_CR_REDINVERT, redInv);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_CR_REDRAISE, redish);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_CUTSLIDER, cut);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_GLOW, glow);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_INVERT, invert);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_NOISE, noise);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_OUTLINE, outline);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_SHAKER, shaker);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_SLITSCAN, slitscan);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_SWELL, swell);
    fx.setFx(ofxPostGlitchType::OFXPOSTGLITCH_TWIST, twist);
    
}


//--------------------------------------------------------------
// DRAW METHOD
//--------------------------------------------------------------
void ofApp::draw(){
    
    //cam.setPosition(camPos.x, camPos.y, camPos.z);
    
    // BEGIN RECORD
#ifdef ENABLE_SAVE
    if(isSaveEnabled) {
        tools.beginRecord();
    }
#endif

    // light.enable();
    ofBackground(0);

    if(isPostGlitchEnabled)
        processPostGlitch();
    else
        processPostProcessing();

#ifdef ENABLE_SAVE
    if(isSaveEnabled) {
        tools.endRecord();
        isSaveEnabled = false;
    }
#endif

    // GUI
    if(isDebug) {
        //draw framerate
        ofSetColor(255);
        
#ifdef KORG_ENABLED
        // Show/hide Korg gui
        nano.showGui(KorgDebug);
#endif
        int posYDebug = 20;
        int posXDebug = 10;
        
        string msg = "FPS: " + ofToString(ofGetFrameRate(), 2);
        ofDrawBitmapString(msg, posXDebug, posYDebug);
        
        // Draw video file names
        if(dirVid.size() > 0) {
            ofColor bg(0,160);
            ofColor fg(255);
            
            ofDrawBitmapStringHighlight(ofToString(dirVid.size()) + " files loaded",posXDebug, (posYDebug + 22), bg, fg);
            
            for(int i = 0; i < (int)dirVid.size(); i++){
                string fileInfo = "File " + ofToString(i + 1) + " = " + dirVid.getName(i);
                
                if(i == idVid) {
                    fg = ofColor(255,0,0);
                }    else {
                    fg = ofColor(255);
                }
                
                ofDrawBitmapStringHighlight(fileInfo, posXDebug, (posYDebug + 22*2) + (i * 21), bg, fg);
            }
        }
        
        
        gui.setPosition(ofGetWidth() - 310,10);
        gui.draw();
        
        // DRAW FX
        fxManager.drawGui(gui.getPosition().x - fxManager.getGUIWidth() + 1, gui.getPosition().y);
    }

    // FADEOUT SCREEN
    // todo: Fade out time to GUI
    if(isExtSoundEnabled) {
        float diff = timeline.getAudioTrack("audio")->getDuration() - timeline.getCurrentTime();
        if(diff < 5) {
            ofPushStyle();
            ofPushMatrix();
            ofSetColor(0,ofMap(diff, 0, 5, 255 , 0, true));
            ofDrawRectangle(0,0,ofGetWidth(), ofGetHeight());
            ofPopMatrix();
            ofPopStyle();
        }
    }

    timeline.draw();

}

void ofApp::drawScene() {
    int depth = 255*zMult;

    // MODE: CAM
    if(mode == CAM) {
        ofTranslate(-vidGrabber.getWidth() * 0.5, -vidGrabber.getHeight() * 0.5);
    } else if( mode == IMAGE) {
        // Draw Frame Box
        ofSetLineWidth(1);
        drawFrameBox(curTransX + images[idVid].getWidth() * 0.5, curTransY + images[idVid].getHeight() * 0.5, curTransZ , images[idVid].getWidth(), images[idVid].getHeight(), depth);

        ofScale(1, -1, 1);

        ofTranslate(curTransX, curTransY, curTransZ - depth*0.5);
    } else if(mode == VIDEO) {
        // Draw Frame Box
        ofSetLineWidth(1);
        drawFrameBox(curTransX + vidPlayer[idVid].getWidth() * 0.5, curTransY + vidPlayer[idVid].getHeight() * 0.5, curTransZ , vidPlayer[idVid].getWidth(), vidPlayer[idVid].getHeight(), depth);

        // if easycam y=-1
        ofScale(1, -1, 1);
        ofTranslate(curTransX, curTransY, curTransZ - depth*0.5);
    }


    // DRAW MESH
    ofPushStyle();
    ofNoFill();

    // Process stroke size
    ofSetLineWidth(pLineThickness);
    glPointSize(pLineThickness);
    glEnable(GL_POINT_SMOOTH);

    mesh.draw();
    ofPopStyle();
}

void ofApp::drawFrameBox(int _x, int _y, int _z, int _w, int _h, int _d) {
    if(isShowFrame) {
        ofPushStyle();
        ofNoFill();
        ofSetColor(200,0,0);
        
        ofDrawBox(_x, _y, _z, _w, _h, _d);
        ofPopStyle();
    }
}

float ofApp::getMappedFreqResponse(int _f, int _inMin, int _inMax, int _outMin, int _outMax) {
    return (isSoundReactive) ? ofMap(_f, _inMin, _inMax, _outMin, _outMax) : 0;
}



//--------------------------------------------------------------
// PROCESS POST GLITCH
//--------------------------------------------------------------
void ofApp::processPostGlitch() {
    glEnable(GL_DEPTH_TEST);
    fbo.begin();
    ofClear(0,0);
    ofClearAlpha();
    //ofTranslate(0, ofGetHeight()*0.5);
    cam.begin();

    ofPushMatrix();
    //translate based on size of video

    drawScene();

    ofPopMatrix();

    cam.end();

    fbo.end();


    // Generate and apply shader effects
    fx.generateFx();

    // DRAW GENERATED FBO TO SCREEN
    fbo.draw(0,0);

    glDisable(GL_DEPTH_TEST);
}

//--------------------------------------------------------------
// PROCESS POST PROCESSING
//--------------------------------------------------------------
void ofApp::processPostProcessing() {
    fxManager.begin(cam);
    drawScene();
    fxManager.end();
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key=='y'){
        if(yStep<150)
            yStep++;
    }
    
    if(key=='u'){
        if(yStep>3)
            yStep--;
    }
    
    if(key==' ') {
        if(vidPlayer[idVid].isPaused()) {
            if(isExtSoundEnabled) {
                //mp3.setPaused(isSoundPlayPause);
            }
            vidPlayer[idVid].setPaused(false);
        }else{
            if(isExtSoundEnabled) {
                //mp3.setPaused(!isSoundPlayPause);
            }
            vidPlayer[idVid].setPaused(true);
        }
    }
    
    if(mode == VIDEO)
    {
        if (dirVid.size() > 0){
            if(key == OF_KEY_DOWN) {
                vidPlayer[idVid].setPaused(true);
                mesh.clear();
                idVid++;
            }else if(key == OF_KEY_UP) {
                vidPlayer[idVid].setPaused(true);
                idVid--;
                mesh.clear();
                if(idVid < 0)
                    idVid = dirVid.size() - 1;
            }
            idVid %= dirVid.size();
        }
    }else if(mode == IMAGE){
        if (images.size() > 0){
            if(key == OF_KEY_DOWN) {
                idVid++;
                mesh.clear();
            }else if(key == OF_KEY_UP) {
                idVid--;
                mesh.clear();
                if(idVid < 0)
                    idVid = images.size() - 1;
            }
            idVid %= images.size();
        }
    }
    
    
    if( key == 'p') {
        isPointMode = !isPointMode;
    }
    
    
    if(key == 'w') {
        isWhiteColor = !isWhiteColor;
    }
    
    if(key == 'r') {
        zMult += 0.05;
    }
    
    
    if(key == 'R') {
        zMult -=0.05;
        
        if(zMult < 0)
            zMult = 0.05;
    }
    
    if(key == 'd') { 
        isDebug = !isDebug;
    }

    if(key == 'h') {
        if(!timeline.getIsShowing() && !isDebug)
            ofHideCursor();
        else
            ofShowCursor();
    }
    
    if(key == 'f')
        ofToggleFullscreen();

    if(key == 't') {
        if(timeline.getIsShowing())
            timeline.hide();
        else
            timeline.show();
    }

    if(timeline.getIsShowing()) {
        if(key == 'a') {
            cameraTrack->addKeyframe();
        }else if(key == 'l') {
            cameraTrack->lockCameraToTrack = !cameraTrack->lockCameraToTrack;
        }
    }
    
    /*if(key == 'z'){
        saveCam.tweenNow(0, 3); // first int is what camera to tween to , secound int is time
    }
    if(key == 'x'){
        saveCam.tweenNow(1, 2.5);
    }
    if(key == 'c'){
        saveCam.tweenNow(2, 3.5);
    }
    
    if(key == 'v'){
        saveCam.tweenNow(3, 3); // first int is what camera to tween to , secound int is time
    }
    if(key == 'b'){
        saveCam.tweenNow(4, 3);
    }
    if(key == 'n'){
        saveCam.tweenNow(5, 3);
    }*/
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    fbo.clear();
    
    //setTranslationPoints();

    ofFbo::Settings settings;
    settings.useStencil = false;
    settings.maxFilter = GL_NEAREST;
    settings.minFilter = GL_NEAREST;
    settings.height = h;
    settings.width = w;
    settings.internalformat = GL_RGBA32F;
    settings.numSamples = 4;
    
    fbo.allocate(settings);
    fbo.begin();
    ofClear(0,0);
    fbo.end();
    
    fx.setFbo(&fbo);

    ofLogNotice() << "Window Resized: " << w << "*"<< h;
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

//--------------------------------------------------------------
// KORG NANO CONTROLLER LISTENER METHODS
//--------------------------------------------------------------
#ifdef KORG_ENABLED
void ofApp::korgButtonPressed(int &_val) {
    
}


void ofApp::korgPotChanged(int &_val) {
    
    
}


void ofApp::korgSliderChanged(int &_val) {
    zMult = float(ofMap(nano.getVal(K_SLIDER_4),0,127,0.01,4.0));
    pLineThickness = int(ofMap(nano.getVal(K_SLIDER_3),0,127,2,12));
}


void ofApp::sceneButtonPressed(int &e) {
    cout <<  "Scene button pressed" << endl;
}
#endif


//--------------------------------------------------------------
// CAMERA SAVE & LOAD
//--------------------------------------------------------------
void ofApp::setupCameraSaveLoad() {
    saveCam.setup(&cam,"xml"); // add you ofeasycam and the folder where the xmls are
    saveCam.enableSave(); // by defaul the listion is on you can actival with enableSave;
    //saveCam.disableSave(); // or disable key save wtih this
}

void ofApp::exit() {
    if(mode == VIDEO)
    {
        for(int i = 0; i < (int)dirVid.size(); i++){
            
            vidPlayer[i].close();
        }
    }
    
}

