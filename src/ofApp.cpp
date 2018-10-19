#include "ofApp.h"

void ofApp::visualizeSurface()
{

}

//--------------------------------------------------------------
void ofApp::setup(){
	this->photometricStereo.loadImgs();
	this->photometricStereo.preprocessing();
	this->photometricStereo.computeImgs();
	this->photometricStereo.computeSurface();
	this->albedoImg = this->photometricStereo.getAlbedo();
	this->surfaceMat = this->photometricStereo.getSurface();
//	cv::imshow("Result", this->photometricStereo.getAlbedo());	
	flip(this->albedoImg, this->albedoImg, 0);
	flip(this->surfaceMat, this->surfaceMat, 0);

	cam.setPosition(-this->albedoImg.cols/2, -this->albedoImg.rows/2, 1000);
//	cam.setTarget(ofVec3f(this->albedoImg.cols/2, this->albedoImg.rows/2, 0));
	cam.setDistance(500);
////	cam.setFarClip(10000);

	for (int row = 0; row < this->albedoImg.rows; ++row) {
		for (int col = 0; col < this->albedoImg.cols; ++col) {
			this->surfaceMesh.addVertex(ofVec3f());
			this->surfaceMesh.addColor(ofFloatColor());
		}
	}

	for (int row = 0; row < this->albedoImg.rows-1; ++row) {
		for (int col = 0; col < this->albedoImg.cols-1; ++col) {
			this->surfaceMesh.addIndex(row*this->albedoImg.cols + col);
			this->surfaceMesh.addIndex(row*this->albedoImg.cols + col + 1);
			this->surfaceMesh.addIndex((row + 1)*this->albedoImg.cols + col);
			this->surfaceMesh.addIndex((row + 1)*this->albedoImg.cols + col);
			this->surfaceMesh.addIndex(row*this->albedoImg.cols + col);
			this->surfaceMesh.addIndex((row + 1)*this->albedoImg.cols + col + 1);
		}
	}
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(ofColor(0, 0, 0));

	for (int row = 0; row < this->albedoImg.rows; ++row) {
		for (int col = 0; col < this->albedoImg.cols; ++col) {
			int index = row*this->albedoImg.cols + col;
			float surfaceHeight = this->surfaceMat.at<float>(row, col);
			float albedoColor = this->albedoImg.at<float>(row, col);

			this->surfaceMesh.setVertex(index, ofVec3f(row,col,surfaceHeight));
			this->surfaceMesh.setColor(index, ofFloatColor(albedoColor));
		}
	}

	cam.begin();


//	ofDrawAxis(2000);
	ofPushStyle();
	ofSetColor(ofColor::white);
	ofPushMatrix();
	ofRotate(90, 0, 0, 1);
	ofTranslate(-this->albedoImg.rows / 2, -this->albedoImg.cols / 2, 0);
//	ofDrawGridPlane(2, 400, true);
	this->surfaceMesh.draw();
	ofPopMatrix();
	ofPopStyle();
	cam.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
