/*
 * linelocalization.cpp
 *
 *  Created on: Apr 21, 2016
 *      Author: saverio
 */


#include "opencv2/opencv.hpp"
#include "persistence1d.hpp"
#include <algorithm>

using namespace cv;
using namespace std;
using namespace p1d;

inline void invert (Mat& im, Mat& output) {
	output = abs(255 - im) / 255;
}

inline void enhance (Mat& im, Mat& output) {
	Mat element = getStructuringElement( MORPH_RECT, Size(5, 5), Point(2, 2));
	erode(im, output, element);

	element = getStructuringElement( MORPH_RECT, Size(15, 15), Point(7, 7));
	dilate(output, output, element);
}

inline vector<int> detect_peaks (Mat& hist, double delta) {

	vector<float> data;
	for (unsigned int i = 0; i < hist.total(); i++) {
		data.push_back((float) hist.at<double>(i, 0));
	}

	Persistence1D detector;
	detector.RunPersistence(data);

	vector<TPairedExtrema> Extrema;
	detector.GetPairedExtrema(Extrema, delta);

	vector<int> lines;
	for(vector< TPairedExtrema >::iterator it = Extrema.begin(); it != Extrema.end(); it++) {
		lines.push_back((*it).MaxIndex);
	}

	return lines;

}

inline vector<int> projection_analysis (Mat& im, double delta) {

	im.convertTo(im, CV_64F);
	Mat hist = Mat(im.rows, 1, CV_64F);
	reduce(im, hist, 1, CV_REDUCE_SUM, CV_64F);

	Scalar _mean, _std;
	meanStdDev(hist, _mean, _std);
	double hist_mean = _mean.val[0];
	double hist_std = _std.val[0];

	double min, max;
	minMaxLoc(hist, &min, &max);

	hist = hist / max;
	delta = hist_mean / max + hist_std / max;

	return detect_peaks (hist, delta);

}

inline vector<int> localize (Mat& input, double delta) {

	Mat im;
	invert(input, im);
	vector<int> peaks = projection_analysis(im, delta);
	sort(peaks.begin(), peaks.end());

	vector<int> lines;
	int dist, valley;
	for (unsigned int i = 0; i < peaks.size() - 1; i++) {
		dist = (peaks[i + 1] - peaks[i]) / 2;
		valley = peaks[i] + dist;
		lines.push_back(valley);
	}

	return lines;

}