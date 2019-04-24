#pragma once

#include "ofMain.h"
#include "ofxFPSCamera.h"
#include "ofxGui.h"
#include "ofxPostGlitch.h"
#include "ofxCameraMove.h"
#include "ofxMotionBlurCamera.h"

#ifdef KORG_ENABLED
#include "ofxKorgNanoKontrol.h"
#endif

// Uncomment the foloowing line to enbale midi - Input
//#define KORG_ENABLED

class testApp : public ofBaseApp{
public:
    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    

    
    // GUI
    ofxPanel gui;
    ofParameter<int> pLineThickness;
    ofParameterGroup fxTypes;
    ofParameter<bool> converge;
    ofParameter<bool> glow;
    ofParameter<bool> shaker;
    ofParameter<bool> cut;
    ofParameter<bool> twist;
    ofParameter<bool> outline;
    ofParameter<bool> noise;
    ofParameter<bool> slitscan;
    ofParameter<bool> swell;
    ofParameter<bool> invert;
    ofParameter<bool> highContrast;
    ofParameter<bool> blueish;
    ofParameter<bool> greenish;
    ofParameter<bool> redish;
    ofParameter<bool> redInv;
    ofParameter<bool> blueInv;
    ofParameter<bool> greenInv;
    
    // Sound Player Parameters
    ofParameterGroup soundGUI;
    ofParameter<bool> isSoundEnabled;
    ofParameter<float> soundLevel;
    
    // Mesh Parameters
    ofParameter<bool> isShowFrame;
    ofParameter<bool> isPointMode;
    ofParameter<bool> isWhiteColor;
    
    ofParameter<int> yStep;
    ofParameter<int> xStep;
    ofParameter<float> zMult;
    
    // Glow FX GUI
    ofParameter<int> glowAmount;
    
    // FX
    ofxPostGlitch fx;
    ofFbo fbo;
    void updateFXParameters();

#ifdef KORG_ENABLED
    // Korg GUI
    ofParameterGroup KorgGui;
    ofParameter<bool> KorgDebug;
    
    
    // Korg midi Controller
    ofxKorgNanoKontrol nano;
    void korgSliderChanged(int & _val);
    void korgButtonPressed(int & _val);
    void korgPotChanged(int & _val);
    void sceneButtonPressed(int & _val);
#endif
    
    // Video Grabber
    ofVideoGrabber 		vidGrabber;
    int 				camWidth;
    int 				camHeight;
    
    // Global Light Options
    ofLight             light;
    
    // Scene Camera
    ofEasyCam cam;
    
    // Sound Input Analysis Parameters
    ofSoundStream soundStream;
    void audioIn(float * input, int bufferSize, int nChannels); 
    
    vector <float> left;
    vector <float> right;
    vector <float> volHistory;
    
    int 	bufferCounter;
    int 	drawCounter;
    
    float smoothedVol;
    float scaledVol;
    
    
    // Sound Player Analysis Parameters
    ofSoundPlayer 		mp3;
    float 				* fftSmoothed;
    
    int nBandsToGet;
    float prevx, prevy;
    
    
    
    
    bool isDebug;
    
    ofImage img;
    string mode;
    ofVec3f camPos;
    
    // Cameras
    ofxCameraMove saveCam;
    ofxFPSCamera camera;
    bool isFPSCamEnabled;
    
    // CUSTOM METHODS
    void setupFPSCam();
    void setupCameraSaveLoad();
    
    // Random rotation variables
    ofColor color;
    float velocity;
    ofVec3f pos;
    ofQuaternion rot;
    ofMatrix4x4 mat;
    ofVec3f rotationAxis;
    float size;
    

    
    
    //playing video + rutt etra stuff
    ofVideoPlayer vidPlayer;
    ofPixels vidPlayerPx;
    
    // Main Scene Mesh
    ofMesh mesh;
    ofColor meshColor;
    
    int curTransX;
    int curTransY;
    int curTransZ;
    
    ofVec3f defCamPos;
    ofVec3f contTrans;
    ofVec3f contRot;
};
