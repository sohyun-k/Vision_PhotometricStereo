#include "PhotometricStereo.h"

cv::Point3f PhotometricStereo::calCart(double a, double e, double radius)
{
	cv::Point3f cartesian;
	cartesian.x = radius*cos(a)*cos(e);
	cartesian.y = radius*sin(a)*cos(e);
	cartesian.z = radius*sin(e);
	return cartesian;
}

void PhotometricStereo::settingMat()
{
	/*setting Matrix I and V*/
	for (int row = 0; row < this->rowNum; ++row) {
		for (int col = 0; col < this->colNum; ++col) {
			cv::Mat tempI = cv::Mat::zeros(this->imgs.size(), 1, CV_32FC1);
			for (int index = 0; index < this->imgs.size(); ++index) {
				tempI.at<float>(index, 0) = this->blurredImgs[index].at<float>(row, col);
			}
			this->I_Matrices.push_back(tempI);

			cv::Mat tempV = cv::Mat::zeros(this->imgs.size(), 3, CV_32FC1);
			for (int index = 0; index < this->imgs.size(); ++index) {
				tempV.at<float>(index, 0) = this->lightDirections.at(index).x;
				tempV.at<float>(index, 1) = this->lightDirections.at(index).y;
				tempV.at<float>(index, 2) = this->lightDirections.at(index).z;
			}
			this->V_Matrices.push_back(tempV);

			/* g matrix = V^(-1) * I */
			cv::Mat g_matrix = tempV.inv(cv::DECOMP_SVD)*tempI;
			this->g_Matrices.push_back(g_matrix);

			ofVec3f g_vector = ofVec3f(g_matrix.at<float>(0, 0), g_matrix.at<float>(1, 0), g_matrix.at<float>(2, 0));
			this->g_Vectors.push_back(g_vector);
		}
	}
}

void PhotometricStereo::settingAlbedo()
{
	this->albedo_Mat = cv::Mat::zeros(this->rowNum, this->colNum, CV_32FC1);
	for (int row = 0; row < this->rowNum; ++row) {
		for (int col = 0; col < this->colNum; ++col) {
			float length = this->g_Vectors.at(row*this->colNum + col).length();
			this->albedo_Mat.at<float>(row, col) = length;
		}
	}
}

void PhotometricStereo::settingNormals()
{
	this->normal_Mat = cv::Mat::zeros(this->rowNum, this->colNum, CV_32FC3);
	for (int row = 0; row < this->rowNum; ++row) {
		for (int col = 0; col < this->colNum; ++col) {
			ofVec3f norm_vec = this->g_Vectors.at(row*this->colNum + col).getNormalized();
			this->normal_Mat.at<cv::Vec3f>(row, col)[0] = norm_vec.x;
			this->normal_Mat.at<cv::Vec3f>(row, col)[1] = norm_vec.y;
			this->normal_Mat.at<cv::Vec3f>(row, col)[2] = norm_vec.z;
		}
	}
}

void PhotometricStereo::settingPQMat()
{
	this->p_Mat = cv::Mat::zeros(this->rowNum, this->colNum, CV_32FC1);
	this->q_Mat = cv::Mat::zeros(this->rowNum, this->colNum, CV_32FC1);
	for (int row = 0; row < this->rowNum; ++row) {
		for (int col = 0; col < this->colNum; ++col) {
			this->p_Mat.at<float>(row, col) = this->g_Vectors.at(row*this->colNum + col).x / this->g_Vectors.at(row*this->colNum + col).z;
			this->q_Mat.at<float>(row, col) = this->g_Vectors.at(row*this->colNum + col).y / this->g_Vectors.at(row*this->colNum + col).z;
		}
	}
}

void PhotometricStereo::computeIntegral()
{
	this->p_MatIntegral = cv::Mat::zeros(this->rowNum, this->colNum, CV_32FC1);
	this->q_MatIntegral = cv::Mat::zeros(this->rowNum, this->colNum, CV_32FC1);

	for (int row = 0; row < this->rowNum; ++row) {
		float compIntegral = 0;
		for (int col = 0; col < this->colNum; ++col) {
			compIntegral += this->p_Mat.at<float>(row, col);
			this->p_MatIntegral.at<float>(row, col) = compIntegral;
		}
	}

	for (int col = 0; col < this->colNum; ++col) {
		float compIntegral = 0;
		for (int row = 0; row < this->rowNum; ++row) {
			compIntegral += this->q_Mat.at<float>(row, col);
			this->q_MatIntegral.at<float>(row, col) = compIntegral;
		}
	}
}

void PhotometricStereo::loadImgs()
{
	this->directory.allowExt(this->allowExt);
	this->directory.listDir(this->dataDir);

	std::vector<ofFile> listFile = this->directory.getFiles();
	for (int index = 0; index < listFile.size(); ++index) {
		std::string fileName = listFile.at(index).getFileName();
		if (fileName.find("Ambient") != std::string::npos) {
			this->ambientFileName = fileName;
			this->ambientImg = cv::imread("data/" + this->dataDir + this->ambientFileName, 0);
			this->ambientImg.convertTo(this->ambientImg, CV_32FC1);
			this->rowNum = this->ambientImg.rows;
			this->colNum = this->ambientImg.cols;
		}
		else {
			this->imgFileNames.push_back(fileName);
			cv::Mat loadImg = cv::imread("data/"+this->dataDir + fileName, 0);
			loadImg.convertTo(loadImg, CV_32FC1);
			this->imgs.push_back(loadImg);
			int idx_A = fileName.find('A');
			int idx_E = fileName.find('E');

			int val_A = std::stoi(fileName.substr(idx_A + 1, 4));
			int val_E = std::stoi(fileName.substr(idx_E + 1, 3));
			cv::Point3f lightDir = this->calCart(ofDegToRad(val_A), ofDegToRad(val_E), 1);
			this->lightDirections.push_back(cv::Point3f(lightDir.y, lightDir.z, lightDir.x));
		}
	}
}

void PhotometricStereo::preprocessing()
{
	/*
	1. Subtract the ambient image of the other images (pixel >= 0)
	2. divide the image intensities by 255 to scale them from [0,1]
	3. reduce noise with a small Gaussian kernel(blurring)
	*/
	for (int index = 0; index < this->imgs.size(); ++index) {
		cv::Mat subtract_scaled = cv::Mat::zeros(this->ambientImg.rows, this->ambientImg.cols, CV_32FC1);
		for (int row = 0; row < this->rowNum; ++row){
			for (int col = 0; col < this->colNum; ++col){
				float subtract = this->imgs.at(index).at<float>(row, col) - this->ambientImg.at<float>(row, col);
				if (subtract < 0){
					subtract_scaled.at<float>(row, col) = 0;
				}
				else {
					subtract_scaled.at<float>(row, col) = subtract / 255;
				}
			}
		}

		this->scaledImgs.push_back(subtract_scaled);
		cv::Mat blur = subtract_scaled.clone();
		cv::GaussianBlur(blur, blur, cv::Size(3, 3), 0, 0);
		this->blurredImgs.push_back(blur);
	}
}

void PhotometricStereo::computeImgs()
{
	this->settingMat();
	this->settingAlbedo();
	this->settingNormals();
	this->settingPQMat();
	this->computeIntegral();
}

void PhotometricStereo::computeSurface()
{
	this->surface = cv::Mat::zeros(this->rowNum, this->colNum, CV_32FC1);
	for (int row = 0; row < this->rowNum; ++row) {
		for (int col = 0; col < this->colNum; ++col) {
			// 표면높이
			this->surface.at<float>(row, col) = this->p_MatIntegral.at<float>(row, col) + this->q_MatIntegral.at<float>(row, col);
		}
	}
}

cv::Mat PhotometricStereo::getSurface()
{
	return this->surface;
}

cv::Mat PhotometricStereo::getAlbedo()
{
	return this->albedo_Mat;
}
