#include "ofApp.h"
#include "ofMotionDetect.h"

using namespace ofxCv;
using namespace cv;


void ofApp::setup() {
    ofSetVerticalSync(true);
    ofBackground(0);
    
#ifdef _USE_LIVE_VIDEO
    //vidGrabber.setVerbose(true);
    vidGrabber.initGrabber(1280,720);
#else
    movie.loadMovie("video2.mov");
    movie.play();
#endif
    
    colorImg.allocate(1280,720);
    grayImage.allocate(1280,720);
    grayBg.allocate(1280,720);
    grayDiff.allocate(1280,720);
    _threshold = 200;
    
    gui.setup("panel");
    gui.add(radMin.set("radMin", 1,1,10));
    gui.add(radMax.set("radMax", 11,11,200));
    gui.add(th.set("th_detection", 200,0,255));             //cv側の検出のthreshold(2値化しないとき)
    gui.add(_th.set("th_binarization", 230,0,255));         //2値化のためのthreshold
    gui.add(histscale.set("histscale", 10,3,50));
    gui.add(detectSpeedMin.set("detectSpeedMin", 4,1,30));
    gui.add(detectSpeedMax.set("detectSpeedMax", 30,1,30));

    contourFinder.setMinAreaRadius(radMin);
    contourFinder.setMaxAreaRadius(radMax);
    contourFinder.setThreshold(th);
    // wait for half a frame before forgetting something
    contourFinder.getTracker().setPersistence(15);          //ここよくわからん
    // an object can move up to 100 pixels per frame
    contourFinder.getTracker().setMaximumDistance(100);     //横振りとるために増やしました
    
    radMin.addListener(this, &ofApp::valChanged);
    radMax.addListener(this, &ofApp::valChanged);
    th.addListener(this, &ofApp::valChanged);
    histscale.addListener(this, &ofApp::valChanged);
    
    showLabels = true;
    
    //osc sender
    sender.setup(HOST, PORT);
    
    bLearnBakground = true;
}

void ofApp::valChanged(int &val){
    contourFinder.setMinAreaRadius(radMin);
    contourFinder.setMaxAreaRadius(radMax);
    contourFinder.setThreshold(th);
    }


void ofApp::update() {
    
#ifdef _USE_LIVE_VIDEO
    vidGrabber.update();
    if (vidGrabber.isFrameNew()){
        //blur(movie, 10);
            colorImg.setFromPixels(vidGrabber.getPixels(), 1280,720);
            grayImage = colorImg;
        if (bLearnBakground == true){
            grayBg = grayImage;     // the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }
        
        //背景差分して2値化
//            grayDiff.absDiff(grayBg, grayImage);
//            grayDiff.threshold(_th);
//            contourFinder.findContours(grayDiff);
        
        //背景差分とらずに2値化
        grayImage.threshold(_th);
        contourFinder.findContours(grayImage);
        
    }
#else
    movie.update();
    if(movie.isFrameNew()) {
        blur(movie, 10);
        contourFinder.findContours(movie);
    }
#endif
}

void ofApp::draw() {
    ofSetBackgroundAuto(showLabels);
    RectTracker& tracker = contourFinder.getTracker();
    if(showLabels) {                                    //camera capture or video play
        ofSetColor(255);
#ifdef _USE_LIVE_VIDEO
        if(!bHide) vidGrabber.draw(0, 0);
        //grayDiff.draw(0,0);
        else grayImage.draw(0, 0);
#endif
        contourFinder.draw();
        if(!bHideGui) gui.draw();
        
        if(SEND_METHOD==4){
            motion_detect_4();
        }
        if(SEND_METHOD==3){
            motion_detect_3();
        }
        if(SEND_METHOD==2){
            motion_detect_2();
        }
        if(SEND_METHOD==1 || SEND_METHOD==0){
            motion_detect_01();
        }
    } else {                                             //トラッキングの軌跡(スペースで切り替え)
        for(int i = 0; i < contourFinder.size(); i++) {
            unsigned int label = contourFinder.getLabel(i);
            // only draw a line if this is not a new label
            if(tracker.existsPrevious(label)) {
                // use the label to pick a random color
                ofSeedRandom(label << 24);
                ofSetColor(ofColor::fromHsb(ofRandom(255), 255, 255));
                // get the tracked object (cv::Rect) at current and previous position
                const cv::Rect& previous = tracker.getPrevious(label);
                const cv::Rect& current = tracker.getCurrent(label);
                // get the centers of the rectangles
                ofVec2f previousPosition(previous.x + previous.width / 2, previous.y + previous.height / 2);
                ofVec2f currentPosition(current.x + current.width / 2, current.y + current.height / 2);
                ofLine(previousPosition, currentPosition);
            }
        }
    }
}

void ofApp::keyPressed(int key) {
    if(key == ' ') showLabels = !showLabels;
    if(key == 'h') bHide = !bHide;
    if(key == 'g') bHideGui = !bHideGui;
    if(key == 'b') bLearnBakground = true;
    
    if(key == 's') {
        gui.saveToFile("settings.xml");
    }
    if(key == 'l') {
        gui.loadFromFile("settings.xml");
    }

//    if(key == '+'){
//        _threshold ++;
//        if (_threshold > 255) _threshold = 255;
//    }
//    if(key == '-'){
//        _threshold --;
//        if (_threshold < 0) _threshold = 0;
//    }
}
