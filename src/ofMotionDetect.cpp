#include "ofApp.h"
#include "ofMotionDetect.h"

using namespace ofxCv;
using namespace cv;


//motion0
#define MINSPEED 3
#define AREASIZEX 20 //近傍の点を検索する範囲
#define AREASIZEY 5

//motion3
#define MOTION_VOTE_BIN_NUM2 40 //検出する最大スピード*2
#define MOTION_VOTE_BIN_NUM 20 //検出する最大スピード
#define MOTION_VOTE_GAUSS 5
#define MOTION_VOTE_GAUSS_HALF 2

//motion3
int MotionVoteX[MOTION_VOTE_BIN_NUM2];
int MotionVoteY[MOTION_VOTE_BIN_NUM2];
int MotionVoteGauss[MOTION_VOTE_GAUSS]={1,2,3,2,1}; //平均スピード算出のためのガウスパラメータ

int MotionVoteX2[MOTION_VOTE_BIN_NUM2];//重み付け後の投票結果（投票＝最も多い手のスピードを投票式で計算）
int MotionVoteY2[MOTION_VOTE_BIN_NUM2];

int MotionVoteXIir[MOTION_VOTE_BIN_NUM2];//Lowpassかけたヒスト
int MotionVoteYIir[MOTION_VOTE_BIN_NUM2];


void ofApp::motion_detect_3()
{
    ofxOscMessage m;
    m.setAddress("/mouse/position2");
    for(int i = 0; i < contourFinder.size(); i++) {
        ofPoint center = toOf(contourFinder.getCenter(i));
        ofPushMatrix();
        ofTranslate(center.x, center.y);
        int label = contourFinder.getLabel(i);
        ofVec2f velocity = toOf(contourFinder.getVelocity(i));
        velsx.push_back(velocity.x);
        velsy.push_back(velocity.y);
        if(((-MOTION_VOTE_BIN_NUM) <= velocity.x) && (velocity.x <= MOTION_VOTE_BIN_NUM)){
            if(((-MOTION_VOTE_BIN_NUM) <= velocity.y) && (velocity.y <= MOTION_VOTE_BIN_NUM)){
                MotionVoteX[int(velocity.x+MOTION_VOTE_BIN_NUM)]+= 1;
                MotionVoteY[int(velocity.y+MOTION_VOTE_BIN_NUM)]+= 1;
            }
        }
        string msg = ofToString(velocity.x);
        m.addIntArg(center.x);
        m.addIntArg(center.y);
        m.addIntArg(label);
        ofDrawBitmapString(msg, 0, 0);
        ofPopMatrix();
    }
    //最も多い動きを計算
    for(int i=0; i<MOTION_VOTE_BIN_NUM2;i++){
        if(i==MOTION_VOTE_BIN_NUM){
            continue;
        }
        for(int j=0; j<MOTION_VOTE_GAUSS; j++){
            if(((i+j-MOTION_VOTE_GAUSS_HALF)<0) || ((i+j-MOTION_VOTE_GAUSS_HALF)>=MOTION_VOTE_BIN_NUM2)){
                continue;
            }
            MotionVoteX2[i+j-MOTION_VOTE_GAUSS_HALF] += (MotionVoteGauss[j] * MotionVoteX[i]);
            MotionVoteY2[i+j-MOTION_VOTE_GAUSS_HALF] += (MotionVoteGauss[j] * MotionVoteY[i]);
        }
    }
    //こっから一番多い手の動きを投票式で決定
    int max_votex=0;
    int max_votey=0;
    int max_votex_idx=0;
    int max_votey_idx=0;
    for(int i=0; i<MOTION_VOTE_BIN_NUM2;i++){
        if(MotionVoteX2[i] > max_votex){
            max_votex=MotionVoteX2[i];
            max_votex_idx=i;
        }
        if(MotionVoteY2[i] > max_votey){
            max_votey=MotionVoteY2[i];
            max_votey_idx=i;
        }
        MotionVoteXIir[i]=(MotionVoteX2[i]+MotionVoteXIir[i]*3)>>2;//Iir Lowpass
        MotionVoteYIir[i]=(MotionVoteY2[i]+MotionVoteYIir[i]*3)>>2;
        MotionVoteX[i]=0;
        MotionVoteY[i]=0;
        MotionVoteX2[i]=0;
        MotionVoteY2[i]=0;
        if(i==MOTION_VOTE_BIN_NUM){
            continue;
        }
        ofSetColor(255, 255, 255, 127);
        ofRect(i*30,0, 30, MotionVoteXIir[i]*histscale);
        ofSetColor(255, 255, 255, 127);
        ofRect(i*30,300, 30, MotionVoteYIir[i]*histscale);
    }
    ofSetColor(255, 0, 0,255);
    ofRect(max_votex_idx*30,0, 30, MotionVoteXIir[max_votex_idx]*10);
    ofRect(max_votey_idx*30,300, 30, MotionVoteYIir[max_votey_idx]*10);
    int Vx=0;
    int Vy=0;
    if(max_votex>0){
        Vx=max_votex_idx-MOTION_VOTE_BIN_NUM;
    }
    if(max_votey>0){
        Vy=max_votey_idx-MOTION_VOTE_BIN_NUM;
    }
    string msg2 = "Vx:"+ofToString(Vx);
    if(Vx>0){
        ofSetColor(255);
    }else{
        ofSetColor(255, 0, 0);
    }
    ofDrawBitmapString(msg2, 0, 30);
    msg2 = "Vx:"+ofToString(Vy);
    if(Vy>0){
        ofSetColor(255);
    }else{
        ofSetColor(255, 0, 0);
    }
    ofDrawBitmapString(msg2, 0, 60);
    for(int i = 0; i < velsx.size() ;i++){
        //平均からどれだけずれてるのか
        m.addIntArg( (velsx[i]-Vx)*(velsx[i]-Vx) + (velsy[i]-Vy)*(velsy[i]-Vy) );
    }
    velsx.clear();
    velsy.clear();
    sender.sendMessage(m);
    //平均速度を送る
    ofxOscMessage m2;
    m2.setAddress("/mouse/position22");
    m2.addIntArg(Vx);
    m2.addIntArg(Vy);
    sender.sendMessage(m2);
}




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


void ofApp::motion_detect_01(){
    int clapcount = 0;
    int sendlimit = 0;
    int detect_flag;
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
    string msg2 = "count :" + ofToString(clapcount);
    ofDrawBitmapString(msg2, 0, 20);
}
