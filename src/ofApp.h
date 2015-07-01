#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxOpenCv.h"
#include "ofxGUI.h"
#include "ofMotionDetect.h"

#define HOST "localhost"
#define PORT 12345

#define SEND_METHOD 3

#define MAXSENDSIZE 400

//#define _USE_LIVE_VIDEO

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    float threshold;
    ofxCv::ContourFinder contourFinder;
    bool showLabels;
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
    
    void radChanged(int &radMin_);
    ofParameter<int>  radMin;
    ofParameter<int>  radMax;
    ofParameter<int>  th;
    ofxPanel gui;
    
    //ofxCvContourFinder 	contourFinder;
    
    
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
    bool				bLearnBakground;

};
