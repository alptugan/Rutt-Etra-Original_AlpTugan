#include "ofApp.h"


// things to do
// work on fft
// smoothing lines
// fast video switch?

//--------------------------------------------------------------
void ofApp::setup(){
    ofEnableSmoothing();
    //ofEnableSetupScreen();
    ofSetVerticalSync(true);
    // toggle fullscreen
   // ofToggleFullscreen();
   
    // Enable or Disable Debug Mode
    isDebug = true;
    
    // TOOLKIT
    isSaveEnabled = false;
    
    // Hide mouse cursor, if "isDebug" enabled
    if(!isDebug)
        ofHideCursor();
    
    // Enable or Disable Mesh Gray scale
    isWhiteColor    = false;
    
    // Switch MESH drawing mode to POINTS / LINES
    isPointMode     = false;
    
    // Switch input source mode
    mode = VIDEO;
    folderPath = "videos";
    
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
    soundGUI.add(isSoundReactive.set("Sound Reactive Mode", true));
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
    
    ofAddListener(nano.sceneButtonPressed, this, &ofApp::sceneButtonPressed);
    ofAddListener(nano.sliderValChanged, this, &ofApp::korgSliderChanged);
    ofAddListener(nano.pushButtonPressed, this, &ofApp::korgButtonPressed);
    ofAddListener(nano.potValChanged, this, &ofApp::korgPotChanged);
#endif
    
    // GUI group to minimize or maximize
    gui.getGroup("Sound Player").minimize();
    gui.getGroup("FX Types").minimize();
    
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
    fbo.begin();
    ofClear(0,0);
    fbo.end();
    
    fx.setup(&fbo);
   
    // Default parameter values
    defCamPos = cam.getPosition();
    
  
    // SETUP CAMERA SAVE & LOAD to load selected camera properties
   // setupCameraSaveLoad();
    
    
    // SETUP TOOLKITS
    tools.setupRecord(1,"export");
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
    
    ofLog() << "curTransX : " << curTransX;
}
//--------------------------------------------------------------
// UPDATE : MAIN
//--------------------------------------------------------------
void ofApp::update(){
    // Update FFT sound input analyze
    if(isSoundReactive)
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
    
    if(mode == CAM) {
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
                    float fftDepth = getMappedFreqResponse(xy, 0, vidPlayer[idVid].getHeight() * vidPlayer[idVid].getWidth(), 100, 15000);
                    

                   
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
    
    
    
    // Process stroke size
    ofSetLineWidth(pLineThickness);
    glPointSize(pLineThickness);
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
                float fftDepth = getMappedFreqResponse(xy, 0, images[idVid].getHeight() * images[idVid].getWidth(), 100, 15000);
                
               
                
                mesh.addVertex(ofVec3f(x, y, meshColor.getBrightness() * (zMult + fft.getIntensityAtFrequency(fftDepth) * soundThresMult)));

                
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


//--------------------------------------------------------------
// UPDATE : FX SHADER
//--------------------------------------------------------------
void ofApp::updateFXParameters() {
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
void ofApp::draw(){
    
    //cam.setPosition(camPos.x, camPos.y, camPos.z);
    
    // BEGIN RECORD
    if(isSaveEnabled) {
        tools.beginRecord();
    }
    
    // light.enable();
    ofBackground(0);
    
    
    glEnable(GL_DEPTH_TEST);
    fbo.begin();
    ofClear(0,0);
    //ofTranslate(0, ofGetHeight()*0.5);
    cam.begin();
    
    
    
    ofPushMatrix();
    
    int depth = 255*zMult;
    
    // MODE: CAM
    if(mode == CAM) {
        ofTranslate(-vidGrabber.getWidth() * 0.5, -vidGrabber.getHeight() * 0.5);
    }
    
    //translate based on size of video
    int xp,yp;
    if( mode == IMAGE) {
        // Draw Frame Box
        drawFrameBox(curTransX + images[idVid].getWidth() * 0.5, curTransY + images[idVid].getHeight() * 0.5, curTransZ , images[idVid].getWidth(), images[idVid].getHeight(), depth);
        
        ofScale(1, -1, 1);
        ofTranslate(curTransX, curTransY, curTransZ - depth*0.5);
    }
    
    
    
    if(mode == VIDEO) {
        // Draw Frame Box
        drawFrameBox(curTransX + vidPlayer[idVid].getWidth() * 0.5, curTransY + vidPlayer[idVid].getHeight() * 0.5, curTransZ , vidPlayer[idVid].getWidth(), vidPlayer[idVid].getHeight(), depth);
        
        // if easycam y=-1
        ofScale(1, -1, 1);
        
        ofTranslate(curTransX, curTransY, curTransZ - depth*0.5);
    }
    
    
    // DRAW MESH
    ofPushStyle();
    ofNoFill();
    
    mesh.draw();
    ofPopStyle();
    ofPopMatrix();
    
    

    cam.end();
    
    fbo.end();
    
    
    // Generate and apply shader effects
    fx.generateFx();
    
    // DRAW GENERATED FBO TO SCREEN
    fbo.draw(0,0);
    
    glDisable(GL_DEPTH_TEST);
    
    if(isSaveEnabled) {
        tools.endRecord();
        isSaveEnabled = false;
    }
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
        
        
    }
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
// AUDIO BUFFER FROM LOADED FILE EVENT HANDLER
//--------------------------------------------------------------


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
    
    if(key == 's'){
        isSaveEnabled = true;
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
    
    if(key == 'z'){
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
    }
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
    settings.internalformat = GL_RGBA16F;
    settings.numSamples = 4;
    
    fbo.allocate(settings);
    fbo.begin();
    ofClear(0,0);
    fbo.end();
    
    fx.setFbo(&fbo);
    
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
    //saveCam.enableSave(); // by defaul the listion is on you can actival with enableSave;
    saveCam.disableSave(); // or disable key save wtih this
}

void ofApp::exit() {
    for(int i = 0; i < (int)dirVid.size(); i++){
        
        vidPlayer[i].close();
    }
}

