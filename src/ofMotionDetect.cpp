#include "ofApp.h"
#include "ofMotionDetect.h"

using namespace ofxCv;
using namespace cv;

void ofApp::motion_detect_2()
{
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