#include "testApp.h"


// things to do
// work on fft
// smoothing lines
// fast video switch?

//--------------------------------------------------------------
void testApp::setup(){
    ofSetVerticalSync(true);
    // toggle fullscreen
   // ofToggleFullscreen();
    
    // Hide mouse cursor by default
    ofHideCursor();
    
   
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
    gui.add(zMult.set("Z-Depth Vertex",0.3,0,4));
    gui.add(soundThresMult.set("sound Multiplier Fac", 0.5, 0, 1.));
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
        camHeight        =  240;
        vidGrabber.initGrabber(camWidth,camHeight);
    }
    
    
    // VIDEO::Initialize Video Player
    if(mode == "video") {
        setupVideos();
    }
    
    
    // SOUND::Load Sound
    if(isSoundEnabled)
    {
        mp3.load("sounds/ISP_arkaoda_2.wav");
        mp3.play();
        mp3.setPaused(true);
    }
    
    // SOUND ANALYZE FFT
    fft.setup();
    fft.setNumFFTBins(32);
    
    
    // Create fbo to apply shader FX effects
    fbo.allocate(ofGetWidth(),ofGetHeight(),GL_RGBA,4);
    fx.setup(&fbo);
   
    // Default parameter values
    defCamPos = cam.getPosition();
    
  
    // SETUP CAMERA SAVE & LOAD to load selected camera properties
    setupCameraSaveLoad();
}

void testApp::setupVideos()
{
    dirVidStr = "ISP-Salt_video_footage";
    //dirVidStr = "ISP_haydarpasa";
    dirVid.listDir(dirVidStr + "/");
    dirVid.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order
    
    //allocate the vector to have as many ofImages as files
    if( dirVid.size() ){
        vidPlayer.assign(dirVid.size(), ofVideoPlayer());
    }
    
    // you can now iterate through the files and load them into the ofImage vector
    for(int i = 0; i < (int)dirVid.size(); i++){
        vidPlayer[i].load(dirVid.getPath(i));
        vidPlayer[i].setVolume(0);
    }
    idVid = 0;
    
    // Default video parameters to set pivot point to center
    curTransX = -vidPlayer[idVid].getWidth()*0.5;
    curTransY = -vidPlayer[idVid].getHeight()*0.5;
}

//--------------------------------------------------------------
// UPDATE : MAIN
//--------------------------------------------------------------
void testApp::update(){
    // Update FFT sound input analyze
    fft.update();
    
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
    /*
    pLineThickness = int(ofMap(nano.getVal(K_SLIDER_3),0,127,2,12));
    */
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
    
    
    //update pixels for mesh
    if(isPointMode) {
        mesh.setMode(OF_PRIMITIVE_POINTS);
    }else{
        //  ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_LINES);
    }
    
    if(mode == "video") {
        if(dirVid.size() > 0) {
            vidPlayer[idVid].update();
            vidPlayerPx = vidPlayer[idVid].getPixels();
            
            int xy=0;
            for (int y = 0; y<vidPlayer[idVid].getHeight(); y+=yStep){
                
                
                for (int x = 0; x < vidPlayer[idVid].getWidth(); x += xStep){
                    
                    
                    if(!isWhiteColor) {
                        meshColor = vidPlayerPx.getColor(x, y);
                        mesh.addColor(ofColor(meshColor, 255));
                    }else{
                        meshColor = vidPlayerPx.getColor(x, y).getBrightness();
                        mesh.addColor(ofColor(meshColor, 255));
                    }
                    
                    float fftDepth = ofMap(xy, 0, vidPlayer[idVid].getHeight() * vidPlayer[idVid].getWidth(), 100, 15000);
                    //cout << fft.getIntensityAtFrequency(fftDepth) << endl;
                    mesh.addVertex(ofVec3f(x, y, meshColor.getBrightness() * (zMult + fft.getIntensityAtFrequency(fftDepth) * soundThresMult)));
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
    if(mode == "image"  ) {
        vidPlayerPx = img.getPixels();
        for (int y = 0; y<img.getHeight(); y+=yStep){
            ofNoFill();
            
            
            for (int x = 0; x < img.getWidth(); x += xStep){
                
                meshColor = img.getColor(x, y);

                
                if(!isWhiteColor) {
                    meshColor = vidPlayerPx.getColor(x, y);
                    mesh.addColor(ofColor(meshColor, 255));
                }else{
                    meshColor = vidPlayerPx.getColor(x, y).getBrightness();
                    mesh.addColor(ofColor(meshColor, 255));
                }
                
                mesh.addVertex(ofVec3f(x, y, meshColor.getBrightness() * (zMult + fft.getMidVal())));

                
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
            
            ofDrawBox(curTransX + vidPlayer[idVid].getWidth() * 0.5, curTransY + vidPlayer[idVid].getHeight() * 0.5, curTransZ , vidPlayer[idVid].getWidth(), vidPlayer[idVid].getHeight(), depth);
            ofPopStyle();
        }
        
        // if easycam y=-1
        ofScale(1, -1, 1);
        //ofRotateX(ofMap(mouseY,0, ofGetHeight(),-180,180));
        
        //ofTranslate(contTransX, contTransY, contTransZ);
        
        //ofLog() << "cam position: " << cam.getPosition();
        ofTranslate(curTransX, curTransY, curTransZ - depth*0.5);
        
        
    }
    
    
    // DRAW MESH
    ofPushStyle();
    ofNoFill();
    ofSetLineWidth(pLineThickness);
    glPointSize(pLineThickness);
    mesh.draw();
    ofPopStyle();
    ofPopMatrix();
    
    glDisable(GL_DEPTH_TEST);
    

    cam.end();
    
    fbo.end();
    
    // Generate and apply shader effects
    fx.generateFx();
    
    // DRAW GENERATED FBO TO SCREEN
    fbo.draw(0,0);
    
    
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
            
            ofDrawBitmapStringHighlight(ofToString(dirVid.size()) + " videos loaded",posXDebug, (posYDebug + 22), bg, fg);
            
            for(int i = 0; i < (int)dirVid.size(); i++){
                string fileInfo = "Video file " + ofToString(i + 1) + " = " + dirVid.getName(i);
                
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
        
        
    }
}



//--------------------------------------------------------------
// AUDIO BUFFER FROM LOADED FILE EVENT HANDLER
//--------------------------------------------------------------


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
        if(vidPlayer[idVid].isPaused()) {
            if(isSoundEnabled) {
                mp3.setPaused(false);
            }
            vidPlayer[idVid].setPaused(false);
        }else{
            if(isSoundEnabled) {
                mp3.setPaused(true);
            }
            vidPlayer[idVid].setPaused(true);
        }
    }
    
    if (dirVid.size() > 0){
        if(key == OF_KEY_DOWN) {
            idVid++;
        }else if(key == OF_KEY_UP) {
            idVid--;
            if(idVid < 0)
                idVid = dirVid.size() - 1;
        }
        mesh.clear();
        
        idVid %= dirVid.size();
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
    pLineThickness = int(ofMap(nano.getVal(K_SLIDER_3),0,127,2,12));
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
    //saveCam.enableSave(); // by defaul the listion is on you can actival with enableSave;
    saveCam.disableSave(); // or disable key save wtih this
}
