#pragma once
#include "opencv.hpp"
#include <string>
#include <math.h>
#include "ofMain.h"

class PhotometricStereo {
private:
	const std::string dataDir = "yaleB01/";
	const std::string allowExt = "pgm";
	ofDirectory directory;

	int rowNum, colNum;

	std::vector<std::string> imgFileNames;
	std::string ambientFileName;
	std::vector<cv::Mat> imgs;
	cv::Mat ambientImg;
	std::vector<cv::Point3f> lightDirections;
	std::vector<cv::Mat> scaledImgs; 
	std::vector<cv::Mat> blurredImgs;

	std::vector<cv::Mat> I_Matrices;
	std::vector<cv::Mat> V_Matrices;
	std::vector<cv::Mat> g_Matrices;
	std::vector<ofVec3f> g_Vectors;
	
	cv::Mat albedo_Mat;
	cv::Mat normal_Mat;
	cv::Mat p_Mat; /* df/dx = N1/N3 */
	cv::Mat q_Mat; /* df/dy = N2/N3 */

	cv::Mat p_MatIntegral, q_MatIntegral;
	cv::Mat surface;
	cv::Point3f calCart(double a, double e, double radius);

	void settingMat();
	void settingAlbedo();
	void settingNormals();
	void settingPQMat();
	void computeIntegral(); 

public:
	void loadImgs(); 
	void preprocessing();
	void computeImgs();
	void computeSurface();
	cv::Mat getSurface();
	cv::Mat getAlbedo();
};