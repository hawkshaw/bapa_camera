#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

#define MINSPEED 3
#define AREASIZEX 20 //近傍の点を検索する範囲
#define AREASIZEY 5

#define SEND_METHOD 2

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
    int clapcount = 0;
    int sendlimit = 0;
    int detect_flag;
    if(showLabels) {
        ofSetColor(255);
        //vidGrabber.draw(0, 0);
        //grayDiff.draw(0,0);
        contourFinder.draw();
        
        if(!bHide) gui.draw();
        
        if(SEND_METHOD==2){
            ofxOscMessage m;
            m.setAddress("/mouse/position2");
            //int MAX_SIZE = 1;
            for(int i = 0; i < contourFinder.size(); i++) {
                //if(i > MAX_SIZE){
                //    break;
                //}
                ofPoint center = toOf(contourFinder.getCenter(i));
                ofPushMatrix();
                ofTranslate(center.x, center.y);
                int label = contourFinder.getLabel(i);
                ofVec2f velocity = toOf(contourFinder.getVelocity(i));
                velsx.push_back(velocity.x);
                velsy.push_back(velocity.y);
                //string msg = ofToString(velocity.x);
                string msg = ofToString(velocity.x);
                //ofLine(0, 0, velocity.x, velocity.y);
                m.addIntArg(center.x);
                m.addIntArg(center.y);
                m.addIntArg(label);
                ofDrawBitmapString(msg, 0, 0);
                ofPopMatrix();
            }
            //平均動きを計算
            
            int velsx_ave = 0;
            int velsy_ave = 0;
            if(velsx.size()>0){
                velsx_ave = std::accumulate(velsx.begin(), velsx.end(), 0.0)/velsx.size();
                velsy_ave = std::accumulate(velsy.begin(), velsy.end(), 0.0)/velsy.size();
            }
            string msg2 = "velsx_ave"+ofToString(velsx_ave);
            if(velsx_ave>0){
                ofSetColor(255);
            }else{
                ofSetColor(255, 0, 0);
            }
            ofDrawBitmapString(msg2, 0, 30);
            msg2 = "velsy_ave"+ofToString(velsy_ave);
            if(velsy_ave>0){
                ofSetColor(255);
            }else{
                ofSetColor(255, 0, 0);
            }
            ofDrawBitmapString(msg2, 0, 60);
            for(int i = 0; i < velsx.size() ;i++){
                //平均からどれだけずれてるのか
                m.addIntArg( (velsx[i]-velsx_ave)*(velsx[i]-velsx_ave) + (velsy[i]-velsy_ave)*(velsy[i]-velsy_ave) );
            }
            velsx.clear();
            velsy.clear();
            sender.sendMessage(m);
            //平均速度を送る
            ofxOscMessage m2;
            m2.setAddress("/mouse/position22");
            m2.addIntArg(velsx_ave);
            m2.addIntArg(velsy_ave);
            sender.sendMessage(m2);
        }
        if(SEND_METHOD==1 || SEND_METHOD==0){
            for(int i = 0; i < contourFinder.size(); i++) {
                ofPoint center = toOf(contourFinder.getCenter(i));
                ofPushMatrix();
                ofTranslate(center.x, center.y);
                int label = contourFinder.getLabel(i);
                //string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
                ofVec2f velocity = toOf(contourFinder.getVelocity(i));
                string msg = ofToString(velocity.x);
                //ofScale(5, 5);
                //ofLine(0, 0, velocity.x, velocity.y);
                detect_flag = 0;
                if(velocity.x > MINSPEED){
                    for(int j = 0; j < contourFinder.size(); j++){
                        ofPoint center2 = toOf(contourFinder.getCenter(j));
                        if(center2.x < center.x){
                            continue;
                        }
                        if(center2.x > (center.x + AREASIZEX)){
                            continue;
                        }
                        if (center2.y < (center.y - AREASIZEY/2)) {
                            continue;
                        }
                        if (center2.y > (center.y + AREASIZEY/2)) {
                            continue;
                        }
                        ofVec2f velocity2 = toOf(contourFinder.getVelocity(j));
                        if(velocity2.x < (- MINSPEED)){
                            clapcount++;
                            detect_flag = 1;
                            //if((sendlimit < 10) && (sendhistory[i%MAXSENDSIZE]==0)){
                            if(sendhistory[i%MAXSENDSIZE]==0){
                                sendlimit++;
                                if(SEND_METHOD==0){//古い送信方法
                                    ofxOscMessage m;
                                    m.setAddress("/mouse/position");
                                    m.addIntArg(int((center.x+center2.x)/2));
                                    m.addIntArg(int((center.y+center2.y)/2));
                                    m.addIntArg(i);
                                    sender.sendMessage(m);
                                }
                                clap clap_buf;
                                clap_buf.clapx = int((center.x+center2.x)/2);
                                clap_buf.clapy = int((center.y+center2.y)/2);
                                clap_buf.clapid = i;
                                claps.push_back(clap_buf);
                                sendhistory[i%MAXSENDSIZE]=1;
                            }
                            ofLine(0, 0, center2.x-center.x, center2.y-center.y);
                            ofDrawBitmapString(msg, 0, 0);
                        }
                    }
                }
                if(detect_flag==0){
                    sendhistory[i%MAXSENDSIZE]=0;
                }
                ofPopMatrix();
            }
            if(SEND_METHOD==1){
                ofxOscMessage m;
                m.setAddress("/mouse/position1");
                for (int i=0; i<claps.size() ;i++){
                    m.addIntArg(claps[i].clapx);
                    m.addIntArg(claps[i].clapy);
                    m.addIntArg(claps[i].clapid);
                }
                sender.sendMessage(m);
            }
        }
        string msg2 = "count :" + ofToString(clapcount);
        ofDrawBitmapString(msg2, 0, 20);
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