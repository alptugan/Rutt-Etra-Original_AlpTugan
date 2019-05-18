# Rutt-Etra-Analog Video Synth v2

This repository is a newer version of [Rutt-Etra-analog-video-synth-implementation](https://github.com/alptugan/Rutt-Etra-analog-video-synth-implementation) uploaded in 2015. 

RE-Viser is a software implementation of the amazing Rutt-Etra analog video animation synthesizer from 70s. The Rutt-Etra analog video animation synthesizer was co-invented by Steve Rutt & Bill Etra as an analog computer for video raster manipulation. RE-Viser is written on C++ using open-source library openFrameworks. The application has built-in GUI (Graphical User Interface) to create Rutt & Etra Analog Video Synthesizer style visual effects on imported image/video files. This is an ongoing open-source project. The source files and former beta releases of the application can be downloaded from my [github.com/alptugan] account. I’ve been using Re-Viser on my personal audiovisual performances and updating the source codes time to time. Currently RE-Viser is only available for developers, not suitable for end-users. 


The code is still a little bit dirty and undocumented. Basically, the app has three modes; sitll image, moving image & video grabber modes. You can switch between the visual input modes setting the ```mode``` variable in ```testApp::setup()``` method. 

---------
## Features
- Control over Korg Nano MIDI device v1. To enable the remote control uncomment the following line in the header file; 
  
  ```// Uncomment the foloowing line to enbale midi - Input``` 
  
  ```//#define KORG_ENABLED```
- Enable load sound and play
- Load still images like .jpg, .png, etc.
- Load video files .mp4, .mov, etc.
- Enable built-in/external webcam input
- MIDI device input (Currently only Korg Nano Device is supported. However, if you are a developer, you can easly map your physical device via ofxMidi addon.)
- Export the generated visual as .jpg file
- Shader effects with ofxPostGlitch
- Record and load 3D space camera actions
- Convert the imported media to 3D mesh and view the generated image with virtual cam in 3D space.
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