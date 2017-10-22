// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<opencv2/opencv.hpp>
#include<iostream>
#include<math.h>

using namespace std;
using namespace cv;

void print_vector_neatly(vector<int>* vec_pointer) {
	for (int i = 0; i < (*vec_pointer).size(); i++) {
		cout << (*vec_pointer)[i] << ", ";
	}
	cout << endl;
}

double distance(double x1, double y1, double x2, double y2) {
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

double map_value(double value, double min_from, double max_from, double min_to, double max_to) {
	return min_to + (max_to - min_to) * ((value - min_from) / (max_from - min_from));
}

void spacial_negative(Mat* source_image, Mat* target_image) {
	cout << "Negative" << endl;
	for (int i = 0; i < (*source_image).rows; i++) {
		for (int j = 0; j < (*source_image).cols; j++) {
			Vec3b current_colour = (*source_image).at<Vec3b>(Point(j, i));
			(*target_image).at<Vec3b>(Point(j, i)) = Vec3b(255 - current_colour[0], 255 - current_colour[1], 255 - current_colour[2]);
		}
	}
}

void spacial_box_blur(Mat* source_image, Mat* target_image, int neighbourhood_size) {
	cout << "Box blur" << endl;
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
	cout << "Gaussian blur with neighbourhood [" << neighbourhood_size << "]" << endl;
	double min = 0.0;
	double max = distance(0, 0, neighbourhood_size / 2 + 1, neighbourhood_size / 2 + 1);

	for (int i = 0; i < (*source_image).rows; i++) {
		for (int j = 0; j < (*source_image).cols; j++) {
			double totalb = 0;
			double totalg = 0;
			double totalr = 0;
			int sample_count = 0;
			double total_distance_weight = 0;
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					if (j + l > -1 && j + l < (*source_image).cols && i + k > -1 && i + k < (*source_image).rows) {
						sample_count += 1;

						double distance_weight = max - distance(j, i, j + l, i + k);
						total_distance_weight += distance_weight;

						Vec3b current_colour = (*source_image).at<Vec3b>(Point(j + l, i + k));
						totalb += (double)current_colour[0] * distance_weight;
						totalg += (double)current_colour[1] * distance_weight;
						totalr += (double)current_colour[2] * distance_weight;
					}
				}
			}

			totalb /= total_distance_weight;
			totalg /= total_distance_weight;
			totalr /= total_distance_weight;

			(*target_image).at<Vec3b>(Point(j, i)) = Vec3b(totalb, totalg, totalr);
		}
	}
}

void binary_threshold(Mat* source_image, Mat* target_image, int threshold) {
	for (int i = 0; i < (*source_image).rows; i++) {
		for (int j = 0; j < (*source_image).cols; j++) {
			Vec3b current_colour = (*source_image).at<Vec3b>(Point(j, i));
			int avg = (current_colour[0] + current_colour[1] + current_colour[2]) / 3;
			if (avg > threshold) {
				(*target_image).at<Vec3b>(Point(j, i)) = Vec3b(255, 255, 255);
			}
			else {
				(*target_image).at<Vec3b>(Point(j, i)) = Vec3b(0, 0, 0);
			}
		}
	}
}

int get_image_grey_avg(Mat* source_image) {
	int avg = 0;
	for (int i = 0; i < (*source_image).rows; i++) {
		for (int j = 0; j < (*source_image).cols; j++) {
			Vec3b current_colour = (*source_image).at<Vec3b>(Point(j, i));
			avg += (current_colour[0] + current_colour[1] + current_colour[2]) / 3;
		}
	}
	return avg / ((*source_image).rows * (*source_image).cols);
}

void binary_threshold_auto(Mat* source_image, Mat* target_image) {
	int avg = get_image_grey_avg(source_image);
	cout << avg << endl;
	binary_threshold(source_image, target_image, avg);
}

void spacial_edge_preserving(Mat* source_image, Mat* target_image, int neighbourhood_size ) {
	cout << "Edge preserved smoothing" << endl;
	for (int i = 0; i < (*source_image).rows; i++) {
		for (int j = 0; j < (*source_image).cols; j++) {
			vector<int> blue_vec;
			vector<int> green_vec;
			vector<int> red_vec;
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					if (j + l > -1 && j + l < (*source_image).cols && i + k > -1 && i + k < (*source_image).rows) {
						Vec3b current_colour = (*source_image).at<Vec3b>(Point(j + l, i + k));
						blue_vec.emplace_back(current_colour[0]);
						green_vec.emplace_back(current_colour[1]);
						red_vec.emplace_back(current_colour[2]);
					}
				}
			}

			sort(blue_vec.begin(), blue_vec.end());
			sort(green_vec.begin(), green_vec.end());
			sort(red_vec.begin(), red_vec.end());

			(*target_image).at<Vec3b>(Point(j, i)) = Vec3b(blue_vec[blue_vec.size() / 2], green_vec[green_vec.size() / 2], red_vec[red_vec.size() / 2]);
		}
	}
}

int main() {
	Mat source_img = imread("C:/PandaNoise.bmp");
	Mat target_img = imread("C:/PandaNoise.bmp");
	Mat target_img2 = imread("C:/PandaNoise.bmp");
	int size = 3;

	//spacial_gaussian_blur(&source_img, &target_img, 20);
	//binary_threshold_auto(&target_img, &target_img);
	//edge_highlight(&target_img, &target_img, 5, 127);

	spacial_gaussian_blur(&source_img, &target_img, 10);
	spacial_edge_preserving(&target_img, &target_img2, 20);

	imshow("SOURCE IMAGE", source_img);
	imshow("TARGET IMAGE", target_img);
	imshow("TARGET IMAGE2", target_img2);
	waitKey(0);

	delete &source_img;
	delete &target_img;

	return 0;
}

