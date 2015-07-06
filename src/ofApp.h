#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxOpenCv.h"
#include "ofxGUI.h"
#include "ofMotionDetect.h"

#define HOST "localhost"
//#define HOST "192.168.13.3"

#define PORT 12345

#define SEND_METHOD 4

#define MAXSENDSIZE 400

//#define _USE_LIVE_VIDEO


///////TEST///////////


class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    float threshold;
    ofxCv::ContourFinder contourFinder;
    bool showLabels;
    void motion_detect_4();
    void motion_detect_3();
    void motion_detect_2();
    void motion_detect_01();
    
#ifdef _USE_LIVE_VIDEO
		  ofVideoGrabber 		vidGrabber;
#else
		  ofVideoPlayer 		movie;
#endif
    
    //osc sender
    ofTrueTypeFont font;
    ofxOscSender sender;
    int sendhistory[MAXSENDSIZE];
    
    ofxCvColorImage			colorImg;
    ofxCvGrayscaleImage 	grayImage;
    ofxCvGrayscaleImage 	grayBg;
    ofxCvGrayscaleImage 	grayDiff;
    int _threshold;
    
    void valChanged(int &val);
    ofParameter<int>  radMin;
    ofParameter<int>  radMax;
    ofParameter<int>  th;
    ofParameter<int>  _th;
    ofParameter<int>  histscale;
    ofParameter<int>  detectSpeedMin;
    ofParameter<int>  detectSpeedMax;
    ofxPanel gui;
    
    //ofxCvContourFinder 	contourFinder;
    //SEND_METHOD 3
    
    //SEND_METHOD 2
    class clap{
    public:
        int clapx,clapy;
        int clapid;
        int clapsend;
    };
    std::vector<clap> claps;
    std::vector<signed int> velsx;
    std::vector<signed int> velsy;
    

    bool bHide = false;
    bool bHideGui = false;
    bool bLearnBakground;

};
