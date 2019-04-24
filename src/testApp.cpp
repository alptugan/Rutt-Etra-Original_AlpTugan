#include "testApp.h"


//things to do
//work on fft
//mode selector on the beginning

//--------------------------------------------------------------
void testApp::setup(){
    ofSetVerticalSync(true);
    // toggle fullscreen
   // ofToggleFullscreen();
    
    // Hide mouse cursor by default
    ofHideCursor();
    
    // Enable or Disable FPS cam
    isFPSCamEnabled = false;
    
    // Enable or Disable Debug Mode
    isDebug = false;
    
    // Enable or Disable Mesh Gray scale
    isWhiteColor    = false;
    
    // Switch MESH drawing mode to POINTS / LINES
    isPointMode     = false;
    
    // Switch input source mode
    mode = "video";
    
    
    // GUI OPTIONS
    ofxGuiSetFont( "../../../../../../assets/DIN.otf", 8 );
    ofxGuiSetDefaultWidth( 300 );
    ofxGuiSetFillColor(ofColor(255,204,0,200));
    
    string xmlSettingsPath = "RuttEtra_Settings.xml";
    gui.setup( "Rutt & Etra Options", xmlSettingsPath );
    gui.setPosition(ofGetWidth() - 310,10);
    
    
    gui.add(yStep.set("Vertical Vertex Distance", 5,1,200));
    gui.add(xStep.set("Horizontal Vertex Distance", 2,1,200));
    gui.add(zMult.set("Z-Depth Vertex",0.3,0,10));
    gui.add(pLineThickness.set("Mesh Line Thickness",1,1,20));
    
    gui.add(isShowFrame.set("Show Mesh Frame", true));
    gui.add(isWhiteColor.set("Set Color",false));
    gui.add(isPointMode.set("Mode: POINT",false));
    
    // SOUND GUI
    soundGUI.setName("Sound Player");
    soundGUI.add(isSoundEnabled.set("Load Sound", false));
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
    
    ofAddListener(nano.sceneButtonPressed, this, &testApp::sceneButtonPressed);
    ofAddListener(nano.sliderValChanged, this, &testApp::korgSliderChanged);
    ofAddListener(nano.pushButtonPressed, this, &testApp::korgButtonPressed);
    ofAddListener(nano.potValChanged, this, &testApp::korgPotChanged);
#endif
    
    // GUI group to minimize or maximize
    gui.getGroup("Sound Player").minimize();
    gui.getGroup("FX Types").minimize();
    
    // Initialize image
    if(mode == "image")
        img.load("afx.png");
    
    // Initialize video grabber
    if(mode == "cam") {
        camWidth         =  320;    // try to grab at this size.
        camHeight         =  240;
        vidGrabber.initGrabber(camWidth,camHeight);
    }
    
    
    // VIDEO::Initialize Video Player
    if(mode == "video") {
        vidPlayer.load("ISP-Salt_video_footage/v1.mp4");
        // vidPlayer.play();

        // Set video sound level to 0
        vidPlayer.setVolume(0);
        
        // Default video parameters to set pivot point to center
        curTransX = -vidPlayer.getWidth()*0.5;
        curTransY = -vidPlayer.getHeight()*0.5;
    }
    
    
    // Load Sound
    if(isSoundEnabled)
    {
        mp3.load("sounds/ISP_arkaoda_2.wav");
        mp3.play();
        mp3.setPaused(true);
    }
    
    
    // Sound Analysis Parameters
    fftSmoothed = new float[8192];
    for (int i = 0; i < 8192; i++){
        fftSmoothed[i] = 0;
    }
    
    nBandsToGet = 1024;
    
    
    
    // Create fbo to apply shader FX effects
    fbo.allocate(ofGetWidth(),ofGetHeight(),GL_RGBA,4);
    fx.setup(&fbo);
   
    // Default parameter values
    defCamPos = cam.getPosition();
    
    // SETUP FPS CAM
    //setupFPSCam();
    
    // SETUP CAMERA SAVE & LOAD to load selected camera properties
    setupCameraSaveLoad();
}



//--------------------------------------------------------------
// UPDATE : MAIN
//--------------------------------------------------------------
void testApp::update(){
    
    // Sound Player
    if(isSoundEnabled) {
        mp3.setVolume(soundLevel);
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
    
    pLineThickness = int(ofMap(nano.getVal(K_SLIDER_3),0,127,2,12));
    
    isPointMode = nano.getVal(K_BUTTON_1,K_TYPE_BUTTON);
    isWhiteColor = nano.getVal(K_BUTTON_3,K_TYPE_BUTTON);
#endif
    
    // Update Method for shader effects
    updateFXParameters();
    
    if(mode == "grabber") {
        //vidGrabber.update();
    }
    
    // update the sound playing system:
    ofSoundUpdate();
    
    float * val = ofSoundGetSpectrum(nBandsToGet);		// request 128 values for fft
    for (int i = 0;i < nBandsToGet; i++){
        
        // let the smoothed calue sink to zero:
        fftSmoothed[i] *= 0.96f;
        
        // take the max, either the smoothed or the incoming:
        if (fftSmoothed[i] < val[i]) fftSmoothed[i] = val[i];
        
    }
    
    
    //update pixels for mesh
    
    
    //lets scale the vol up to a 0-1 range
    scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
    
    //lets record the volume into an array
    volHistory.push_back( scaledVol );
    
    //if we are bigger the the size we want to record - lets drop the oldest value
    if( volHistory.size() >= 400 ){
        volHistory.erase(volHistory.begin(), volHistory.begin()+1);
    }
    
    
    
    if(isPointMode) {
        mesh.setMode(OF_PRIMITIVE_POINTS);
    }else{
        //  ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_LINES);
    }
    
    if(mode == "video") {
        
        vidPlayer.update();
        vidPlayerPx = vidPlayer.getPixels();
        
        int xy=0;
        for (int y = 0; y<vidPlayer.getHeight(); y+=yStep){

            
            for (int x = 0; x < vidPlayer.getWidth(); x += xStep){
                

                if(!isWhiteColor) {
                    meshColor = vidPlayerPx.getColor(x, y);
                    mesh.addColor(ofColor(meshColor, 255));
                }else{
                    meshColor = vidPlayerPx.getColor(x, y).getBrightness();
                    mesh.addColor(ofColor(meshColor, 255));
                }
                
                
                mesh.addVertex(ofVec3f(x, y, meshColor.getBrightness() * zMult  /*+ fftSmoothed[y]*300.f*/));
                //curColor.getBrightness() * .3 + scaledVol));
                
                if(x == 0) {
                    mesh.addIndex(xy);
                }
                
                if(x != 0 && x != vidPlayer.getWidth()-xStep) {
                    mesh.addIndex(xy);
                    mesh.addIndex(xy);
                }
                
                if(x >= vidPlayer.getWidth()-xStep) {
                    mesh.addIndex(xy);
                }
                
                xy = xy + 1;
                
            }
        }
    }
    
    
    
    
    int xy=0;
    if(mode == "image"  ) {
        vidPlayerPx = img.getPixels();
        for (int y = 0; y<img.getHeight(); y+=yStep){
            ofNoFill();
            ofSetLineWidth(2);
            
            for (int x = 0; x < img.getWidth(); x += xStep){
                
                meshColor = img.getColor(x, y);

                
                if(!isWhiteColor) {
                    meshColor = vidPlayerPx.getColor(x, y);
                    mesh.addColor(ofColor(meshColor, 255));
                }else{
                    meshColor = vidPlayerPx.getColor(x, y).getBrightness();
                    mesh.addColor(ofColor(meshColor, 255));
                }
                
                mesh.addVertex(ofVec3f(x, y, meshColor.getBrightness() * zMult + fftSmoothed[y]*300.f));

                
                if(x == 0 || x == img.getWidth()-1) {
                    mesh.addIndex(xy);
                }
                
                if(x != 0 && x != img.getWidth()-1) {
                    mesh.addIndex(xy);
                    mesh.addIndex(xy);
                }
                
                xy++;
                
            }
        }
    }
}


//--------------------------------------------------------------
// UPDATE : FX SHADER
//--------------------------------------------------------------
void testApp::updateFXParameters() {
    // FX
    if(fx.getFx(OFXPOSTGLITCH_GLOW))
        fx.setGlowAmount(glowAmount);
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
void testApp::draw(){
    
    //cam.setPosition(camPos.x, camPos.y, camPos.z);
    
    // light.enable();
    ofBackground(0);
    
    
    
    fbo.begin();
    
    //ofTranslate(0, ofGetHeight()*0.5);
    cam.begin();
    // FPS CAM
    if(isFPSCamEnabled) camera.begin();
    ofClear(0,0,0,0);
    
    glEnable(GL_DEPTH_TEST);
    
    //uncomment for video grabber
    if(mode == "cam") {
        ofTranslate(-vidGrabber.getWidth() * 0.5, -vidGrabber.getHeight() * 0.5);
    }
    
    //translate based on size of video
    int xp,yp;
    if( mode == "image") {
        xp = -img.getWidth()*0.5 - 22;
        yp = -img.getHeight()*0.5;
        ofTranslate(xp, yp);//
    }
    
    int depth = 255*zMult;
    ofPushMatrix();
    if(mode == "video") {
        //ofTranslate(contTrans);
        if(isShowFrame) {
            ofPushStyle();
            ofNoFill();
            ofSetColor(200,0,0);
        //ofDrawRectangle(curTransX, curTransY, curTransZ, vidPlayer.getWidth(),vidPlayer.getHeight());
            
            ofDrawBox(curTransX + vidPlayer.getWidth() * 0.5, curTransY + vidPlayer.getHeight() * 0.5, curTransZ , vidPlayer.getWidth(), vidPlayer.getHeight(), depth);
        ofPopStyle();
        }
        
        // if easycam y=-1
        ofScale(1, -1, 1);
        //ofRotateX(ofMap(mouseY,0, ofGetHeight(),-180,180));
        
        //ofTranslate(contTransX, contTransY, contTransZ);
        
        //ofLog() << "cam position: " << cam.getPosition();
        ofTranslate(curTransX, curTransY, curTransZ- depth*0.5);
        
    }
    
    ofPushStyle();
    ofNoFill();
    ofSetLineWidth(pLineThickness);
    glPointSize(pLineThickness);
    mesh.draw();
    ofPopStyle();
    ofPopMatrix();
    
    glDisable(GL_DEPTH_TEST);
    

    cam.end();
    
    // FPS CAM
    if(isFPSCamEnabled) camera.end();
    
    fbo.end();
    
    // Generate and apply shader effects
    fx.generateFx();
    
    // DRAW GENERATED FBO TO SCREEN
    fbo.draw(0,0);
    
    
    // GUI
    if(isDebug) {
        //draw framerate
        ofSetColor(255);
        string msg = "FPS: " + ofToString(ofGetFrameRate(), 2);
#ifdef KORG_ENABLED
        // Show/hide Korg gui
        nano.showGui(KorgDebug);
#endif
        
        ofDrawBitmapString(msg, 10, 20);
        
        gui.setPosition(ofGetWidth() - 310,10);
        gui.draw();
    }
}



//--------------------------------------------------------------
// AUDIO BUFFER FROM LOADED FILE EVENT HANDLER
//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels){
    
    float curVol = 0.0;
    
    // samples are "interleaved"
    int numCounted = 0;
    
    //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
    for (int i = 0; i < bufferSize; i++){
        left[i]		= input[i*2]*0.5;
        right[i]	= input[i*2+1]*0.5;
        
        curVol += left[i] * left[i];
        curVol += right[i] * right[i];
        numCounted+=2;
    }
    
    //this is how we get the mean of rms :)
    curVol /= (float)numCounted;
    
    // this is how we get the root of rms :)
    curVol = sqrt( curVol );
    
    smoothedVol *= 0.93;
    smoothedVol += 0.07 * curVol;
    
    bufferCounter++;
    
    
    
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    if(key=='y'){
        if(yStep<150)
            yStep++;
        
    }
    
    if(key=='u'){
        if(yStep>3)
            yStep--;
    }
    
    
    
    if(key==' ') {
        if(isFPSCamEnabled) {
            camera.setPosition(0, 0, 0);
            camera.target(ofVec3f(1,0,0));
        }
        
        if(vidPlayer.isPaused()) {
            if(isSoundEnabled) {
                mp3.setPaused(false);
            }
            vidPlayer.setPaused(false);
        }else{
            if(isSoundEnabled) {
                mp3.setPaused(true);
            }
            vidPlayer.setPaused(true);
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
        if(!isDebug)
            ofHideCursor();
        else
            ofShowCursor();
    }
    
    if(key == 'f')
        ofToggleFullscreen();
    
    if(key == 'x'){
        saveCam.tweenNow(0, 3); // first int is what camera to tween to , secound int is time
    }
    if(key == 'c'){
        saveCam.tweenNow(1, 2.5);
    }
    if(key == 'v'){
        saveCam.tweenNow(2, 3.5);
    }
    
    if(key == 'b'){
        saveCam.tweenNow(3, 3); // first int is what camera to tween to , secound int is time
    }
    if(key == 'n'){
        saveCam.tweenNow(4, 3);
    }
    if(key == 'm'){
        saveCam.tweenNow(5, 3);
    }
    
    
    if(key == 'b'){
        saveCam.cutNow((int)ofRandom(2));
    }
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    fbo.clear();
    fbo.allocate(ofGetWidth(),ofGetHeight(),GL_RGBA,4);
    fx.setFbo(&fbo);
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
    
}

//--------------------------------------------------------------
// KORG NANO CONTROLLER LISTENER METHODS
//--------------------------------------------------------------
#ifdef KORG_ENABLED
void testApp::korgButtonPressed(int &_val) {
    
}


void testApp::korgPotChanged(int &_val) {
    
    
}


void testApp::korgSliderChanged(int &_val) {
    zMult = float(ofMap(nano.getVal(K_SLIDER_4),0,127,0.01,4.0));
}


void testApp::sceneButtonPressed(int &e) {
    cout <<  "Scene button pressed" << endl;
}
#endif


//--------------------------------------------------------------
// CAMERA SAVE & LOAD
//--------------------------------------------------------------
void testApp::setupCameraSaveLoad() {
    saveCam.setup(&cam,"xml"); // add you ofeasycam and the folder where the xmls are
    saveCam.enableSave(); // by defaul the listion is on you can actival with enableSave;
    //saveCam.disableSave(); // or disable key save wtih this
}

//--------------------------------------------------------------
// FPS GAME CAMERA SETUP
//--------------------------------------------------------------
void testApp::setupFPSCam() {
    // FPS CAMERA
    camera.setup();
    /*****************************************************
     // ofxFPSCamera example
     
     // by Ivaylo Getov, 2014
     // Derived from ofxGameCamera, created by James George and FlightPhase
     
     // Methods: ////////////////////////
     
     camera.disableMove()                                // Disables both mouse and keyboard control
     camera.enableMove()                                 // Enables both mouse and keyboard control
     
     camera.disableStrafe()                              // Disables left/right strafe (A/D or left/right arrow)
     camera.enableStrafe()                               // Enables left/right strafe (on by default)
     
     camera.setCamHeight(float ch);                      // Sets camera y-height
     
     camera.target(ofVec3f v);                           // Sets look target as a vector from current camera position
     camera.getTarget();                                 // Returns current target as an ofVec3f
     
     camera.getPosition();                               // Returns ofVec3f of (X,Y,Z) position
     camera.setPosition(float px, float py, float pz);   // Sets camera position to (x,y,z)
     camera.setPosition(const ofVec3f &p);               // Sets camera position to ofVec3f
     camera.movedManually();                             // Call this whenever you update orientation or position manually (eg using camera.setPosition())
     
     camera.setMinMaxY(float angleDown, float angleUp);  // Set new limits for looking down and up
     
     camera.reset();                                     // Sets camera position to (0,0,0) and camHeight = 0.0
     camera.reset(float h);                              // Sets camera position to (0,0,0) and camHeight = h
     camera.reset(float x, float y, float z);            // Sets camera position to (x,y,z) and camHeight = 0.0
     camera.reset(float x, float y, float z, float h);   // Sets camera position to (x,y,z) and camHeight = h
     camera.reset(ofVec3f v);                            // Sets camera position to ofVec3f v and camHeight = 0.0
     camera.reset(ofVec3f v, float h);                   // Sets camera position to ofVec3f v and camHeight = h
     
     
     // Defaults: ///////////////////////
     
     camera.sensitivityX = 0.10f;            // Mouse Sensitivity
     camera.sensitivityY = 0.10f;
     
     
     
     
     camera.usemouse = true;
     camera.useArrowKeys = false;            // arrow keys instead of W A S D
     camera.autosavePosition = false;
     
     camera.camHeight = 0.0;                 // Camera will always move along a horizontal plane at this Y-value
     camera.upAngle = -30.0;                 // must be negative - limits angle looking UP
     camera.downAngle = 30.0;                // must be positive - limits angle looking DOWN
     
     camera.keepTurning = false;             // When mouse is at far left or right of screen, setting this to TRUE will
     // keep the camera rotating in that direction. The default setting of FALSE will
     // allow for an "infinite" mouse, repositioning the mouse to the middle when it reaches
     // the edge. This is tested on a Mac and SHOUD work on windows, but I haven't had
     // a chance to test it. Currently does not worl on Linux.
     
     camera.easeIn = true;                   // Camera movements speed up gradually until they reach value set as camera.speed.
     camera.accel = 0.3;                     // if easeIn is set to TRUE, this is the rate of acceleration.
     
     *****************************************************/
    camera.speed = 28.0f;                    // Movement speed
    camera.target(ofVec3f(0,0,1));
    
    isFPSCamEnabled = true;
}
