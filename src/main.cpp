#include "ofApp.h"

//--------------------------------------------------------------
int main(){
    
   // ofSetupOpenGL(1024,768, OF_WINDOW);
    //ofRunApp(new ofApp());
    
    
    ofGLWindowSettings settings;
    settings.setGLVersion(2,1);    // <--- ofxShadertoy NEEDS the GL Programmable Renderer
    ofCreateWindow(settings);       // <-------- setup the GL context
    
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());
}
