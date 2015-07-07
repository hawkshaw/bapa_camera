#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxOpenCv.h"
#include "ofxGUI.h"
#include "ofMotionDetect.h"

#define HOST "localhost"
//#define HOST "10.0.1.24"

#define PORT 12345

#define SEND_METHOD 4

#define MAXSENDSIZE 400

#define _USE_LIVE_VIDEO


class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void drawWaku();
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
    
    //検出エリアパラメータ 百分率
    ofParameter<int>  detectAreaTop;
    ofParameter<int>  detectAreaBottom;
    ofParameter<int>  detectAreaTopR;
    ofParameter<int>  detectAreaTopL;
    ofParameter<int>  detectAreaBottomR;
    ofParameter<int>  detectAreaBottomL;
    int areaTop,areaBottom,areaTopR,areaTopL,areaBottomR,areaBottomL;
    
    
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
    std::vector<signed int> velsid;
    //std::vector<signed int> velsx_1f;
    //std::vector<signed int> velsy_1f;
    std::vector<signed int> velsid_1f;
    //中心座標
    std::vector<signed int> centersx;
    std::vector<signed int> centersy;
    std::vector<signed int> centersx_1f;
    std::vector<signed int> centersy_1f;
    //座標変換後の中心座標
    std::vector<signed int> newx;
    std::vector<signed int> newy;
    std::vector<signed int> newx_1f;
    std::vector<signed int> newy_1f;
    
    bool bHide = false;
    bool bHideGui = false;
    bool bLearnBakground;
    bool bClearLog = false;

};
