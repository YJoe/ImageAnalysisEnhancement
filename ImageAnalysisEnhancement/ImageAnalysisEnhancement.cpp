// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<opencv2/opencv.hpp>
#include<iostream>
#include<math.h>

using namespace std;
using namespace cv;

double distance(double x1, double y1, double x2, double y2) {
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

double map_value(double value, double min_from, double max_from, double min_to, double max_to) {
	return min_to + (max_to - min_to) * ((value - min_from) / (max_from - min_from));
}

void spacial_box_blur(Mat* source_image, Mat* target_image, int neighbourhood_size) {
	for (int i = 0; i < (*source_image).rows; i++) {
		for (int j = 0; j < (*source_image).cols; j++) {
			int totalb = 0;
			int totalg = 0;
			int totalr = 0;
			int sample_count = 0;
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					if (j + l > -1 && j + l < (*source_image).cols && i + k > -1 && i + k < (*source_image).rows) {
						sample_count += 1;
						Vec3b current_colour = (*source_image).at<Vec3b>(Point(j + l, i + k));
						totalb += current_colour[0];
						totalg += current_colour[1];
						totalr += current_colour[2];
					}
				}
			}
			totalb /= sample_count;
			totalg /= sample_count;
			totalr /= sample_count;

			(*target_image).at<Vec3b>(Point(j, i)) = Vec3b(totalb, totalg, totalr);
		}
	}
}

void spacial_gaussian_blur(Mat* source_image, Mat* target_image, int neighbourhood_size) {
	double min = 0.0;
	double max = distance(0, 0, neighbourhood_size / 2 + 1, neighbourhood_size / 2 + 1);

	cout << "min distance [" << min << "]" << endl;
	cout << "max distance [" << max << "]" << endl;

	for (int i = 0; i < (*source_image).rows; i++) {
		for (int j = 0; j < (*source_image).cols; j++) {
			int totalb = 0;
			int totalg = 0;
			int totalr = 0;
			int sample_count = 0;
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					if (j + l > -1 && j + l < (*source_image).cols && i + k > -1 && i + k < (*source_image).rows) {
						sample_count += 1;

						double distance_weight = 1.0 - map_value(distance(j, i, j + l, i + k), min, max, 0.0, 1.0);

						Vec3b current_colour = (*source_image).at<Vec3b>(Point(j + l, i + k));
						totalb += (double)current_colour[0] * distance_weight;
						totalg += (double)current_colour[1] * distance_weight;
						totalr += (double)current_colour[2] * distance_weight;
					}
				}
			}
			totalb /= sample_count;
			totalg /= sample_count;
			totalr /= sample_count;

			(*target_image).at<Vec3b>(Point(j, i)) = Vec3b(totalb, totalg, totalr);
		}
	}
}

int main() {
	Mat source_img = imread("C:/PandaNoise.bmp");
	Mat target_img = imread("C:/PandaNoise.bmp");
	namedWindow("image", WINDOW_NORMAL);

	int size = 20;
	spacial_gaussian_blur(&source_img, &target_img, size);

	imshow("image", target_img);
	waitKey(0);

	return 0;
}

