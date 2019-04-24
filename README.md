# Rutt-Etra-Analog Video Synth v2

This repository is a newer version of [Rutt-Etra-analog-video-synth-implementation](https://github.com/alptugan/Rutt-Etra-analog-video-synth-implementation) uploaded in 2015. 


The code is still a little bit dirty and undocumented. Basically, the app has three modes; sitll image, moving image & video grabber modes. You can switch between the visual input modes setting the ```mode``` variable in ```testApp::setup()``` method. 

---------
## Features
- Control over Korg Nano MIDI device v1. To enable the remote control uncomment the following line in the header file; 
  
  ```// Uncomment the foloowing line to enbale midi - Input``` 
  
  ```//#define KORG_ENABLED```
- Enable load sound and play
- Load still images like .jpg, .png, etc.
- Load video files .mp4, .mov, etc.
- Enable video grabber input
- Shader effects with ofxPostGlitch
- Record and load 3D space camera
- Keyboard interaction
- GUI interaction
------------
## Dependecies
- ofxCameraMove
- ofxFPSCamera
- ofxGui
- ofxKorgNanoKontrol
- ofxMidi
- ofxMotionBlurCamera
- ofxOsc
- ofxPostGlitch
- ofxTweener
- ofxXmlSettings

---------

## Todo
  - Execute main alogrithm in the GPU via shaders. Any suggestion or help are highly apprecieted.
  - Improve user interaction. Camera movement can be adapted to ofxTimeline.
  - Finish documentation. 
  - Enable sound reactive mode for loaded sound samples or/and microphone input data  
  - Create Enum structure for mode selection.