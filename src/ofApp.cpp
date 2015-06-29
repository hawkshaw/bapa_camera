#include "ofApp.h"
#include "ofMotionDetect.h"

using namespace ofxCv;
using namespace cv;



void ofApp::setup() {
    ofSetVerticalSync(true);
    ofBackground(0);
    
#ifdef _USE_LIVE_VIDEO
    vidGrabber.setVerbose(true);
    vidGrabber.initGrabber(1280,720);
#else
    movie.loadMovie("video4.mov");
    movie.play();
#endif
    
    colorImg.allocate(1280,720);
    grayImage.allocate(1280,720);
    grayBg.allocate(1280,720);
    grayDiff.allocate(1280,720);
    _threshold = 100;
    
    contourFinder.setMinAreaRadius(50);
    contourFinder.setMaxAreaRadius(100);
    contourFinder.setThreshold(10);
    // wait for half a frame before forgetting something
    contourFinder.getTracker().setPersistence(15);
    // an object can move up to 32 pixels per frame
    contourFinder.getTracker().setMaximumDistance(32);
    
    gui.setup("panel");
    gui.add(radMin.set("radMin", 1,1,10));
    gui.add(radMax.set("radMax", 100,11,300));
    gui.add(th.set("threshold", 10,0,255));
    
    radMin.addListener(this, &ofApp::radChanged);
    radMax.addListener(this, &ofApp::radChanged);
    th.addListener(this, &ofApp::radChanged);
    
    showLabels = true;
    
    //osc sender
    sender.setup(HOST, PORT);
    
    bLearnBakground = true;
}

void ofApp::radChanged(int &radMin_){
    contourFinder.setMinAreaRadius(radMin);
    contourFinder.setMaxAreaRadius(radMax);
    contourFinder.setThreshold(th);
    }


void ofApp::update() {
    
    bool bNewFrame = false;
    
#ifdef _USE_LIVE_VIDEO
    vidGrabber.update();
	   bNewFrame = vidGrabber.isFrameNew();
    if (bNewFrame){
        //blur(movie, 10);
            colorImg.setFromPixels(vidGrabber.getPixels(), 1280,720);
            grayImage = colorImg;
        if (bLearnBakground == true){
            grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }
            grayDiff.absDiff(grayBg, grayImage);
            grayDiff.threshold(_threshold);
        contourFinder.findContours(grayDiff);
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
    if(showLabels) {
        ofSetColor(255);
        //vidGrabber.draw(0, 0);
        //grayDiff.draw(0,0);
        contourFinder.draw();
        
        if(!bHide) gui.draw();
        
        if(SEND_METHOD==2){
            motion_detect_2();
        }
        if(SEND_METHOD==1 || SEND_METHOD==0){
            motion_detect_01();
        }
    } else {
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
                //ofLine(previousPosition, currentPosition);
            }
        }
    }
    
//    // this chunk of code visualizes the creation and destruction of labels
//    const vector<unsigned int>& currentLabels = tracker.getCurrentLabels();
//    const vector<unsigned int>& previousLabels = tracker.getPreviousLabels();
//    const vector<unsigned int>& newLabels = tracker.getNewLabels();
//    const vector<unsigned int>& deadLabels = tracker.getDeadLabels();
//    ofSetColor(cyanPrint);
//    for(int i = 0; i < currentLabels.size(); i++) {
//        int j = currentLabels[i];
//        ofLine(j, 0, j, 4);
//    }
//    ofSetColor(magentaPrint);
//    for(int i = 0; i < previousLabels.size(); i++) {
//        int j = previousLabels[i];
//        ofLine(j, 4, j, 8);
//    }
//    ofSetColor(yellowPrint);
//    for(int i = 0; i < newLabels.size(); i++) {
//        int j = newLabels[i];
//        ofLine(j, 8, j, 12);
//    }
//    ofSetColor(ofColor::white);
//    for(int i = 0; i < deadLabels.size(); i++) {
//        int j = deadLabels[i];
//        ofLine(j, 12, j, 16);
//    }
}

void ofApp::keyPressed(int key) {
//    if(key == ' ') {
//        showLabels = !showLabels;
//    }
//    
//    if(key == 'h') bHide = !bHide;
    switch(key){
        case ' ':
            bLearnBakground = true;
            break;
case '+':
    _threshold ++;
    if (_threshold > 255) _threshold = 255;
    break;
case '-':
    _threshold --;
    if (_threshold < 0) _threshold = 0;
    break;
    }

}