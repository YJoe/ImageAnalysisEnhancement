#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <typeinfo>
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

void take_dft(Mat& source_image, Mat& destination) {
	// create a mat object that can hold real and complex values
	Mat original_complex[2] = { source_image, Mat::zeros(source_image.size(), CV_32F) };

	// merge the two channels into the one dft_ready mat object
	Mat dft_ready;
	merge(original_complex, 2, dft_ready);

	dft(dft_ready, destination, DFT_COMPLEX_OUTPUT);
}

void take_inverse_dft(Mat& source_image, Mat& destination){

	dft(source_image, destination, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);	
}

void get_visual_of_dft(Mat& source_dft, Mat& destination_dft) {
	// split the two channels up into a new split array
	Mat split_arr[2] = { Mat::zeros(source_dft.size(), CV_32F), Mat::zeros(source_dft.size(), CV_32F) };
	split(source_dft, split_arr);

	// take the magnitude
	magnitude(split_arr[0], split_arr[1], destination_dft); 

	// add one to all elements
	destination_dft += Scalar::all(1);

	// take log of magnitude to scale things down
	log(destination_dft, destination_dft);

	// normalise this thing
	normalize(destination_dft, destination_dft, 0, 1, CV_MINMAX);
}

void fill_elipse(Mat& image, int x, int y, int size_x, int size_y, int stroke) {
	ellipse(image, Point(x, y), Size(size_x, size_y), 0, 0, 360, Scalar(255, 0, 0), stroke, 8);
}

void create_filter_mask(Size size, Mat& mask, int x_size, int y_size, int stroke, bool invert) {
	mask = Mat::zeros(size.height, size.width, CV_32F);
	fill_elipse(mask, mask.cols / 2, mask.rows / 2, x_size, y_size, stroke);

	normalize(mask, mask, 0, 1, CV_MINMAX);

	if (invert) {
		mask = Mat::ones(mask.size(), CV_32F) - mask;
	}
}

void create_blank_filter(Size size, Mat& mask, bool ones) {
	if (ones) {
		mask = Mat::ones(size.height, size.width, CV_32F);
	} else {
		mask = Mat::zeros(size.height, size.width, CV_32F);
	}
}

void shift(Mat& magI) {

	// crop if it has an odd number of rows or columns
	magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

	int cx = magI.cols / 2;
	int cy = magI.rows / 2;

	Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
	Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

	Mat tmp;                            // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);                     // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);
}

void apply_frequency_mask(Mat& source_image, Mat& mask, Mat& target_image, bool verbose) {
	// ----------------------------------------------------------------------
	// take the dft of the source image
	Mat source_dft;
	take_dft(source_image, source_dft);

	// get a visual representation of dft
	Mat source_dft_visual;
	get_visual_of_dft(source_dft, source_dft_visual);

	// display the source image
	imshow("source", source_image);

	if (verbose) {
		// display the dft of the source image
		imshow("dft", source_dft_visual);

		// swap quadrants of the dft so that the low frequencies are in the middle of the image and display it
		shift(source_dft_visual);
		imshow("dft shifted", source_dft_visual);
	}
	
	// ----------------------------------------------------------------------
	// display the mask
	imshow("mask", mask);

	// shift the mask to match the dft of the source image ready for multiplying them later
	shift(mask);

	if (verbose) {
		// display the shifted mask
		imshow("mask shifted", mask);
	}

	// ----------------------------------------------------------------------
	// create two channels, one for real and the other for imaginary, set both channels to have all zeros as pixels
	Mat planes[] = { Mat::zeros(source_dft.size(), CV_32F), Mat::zeros(source_dft.size(), CV_32F) };

	// create a full mask that will hold both real and imaginary parts of the filter
	Mat full_mask;

	// set the real part of the filter equal to the filter we created earlier, leaving the imaginary part as all 0s
	planes[0] = mask;

	// merge the two planes into the single image full_mask
	merge(planes, 2, full_mask);

	// multiply the source dft with the full mask, after this, source_dft will hold the new dft image
	mulSpectrums(source_dft, full_mask, source_dft, 0);


	// ----------------------------------------------------------------------
	// get a visual representation of the new dft and display it
	
	if (verbose) {
		get_visual_of_dft(source_dft, source_dft_visual);
		imshow("dft * mask shifted", source_dft_visual);

		// swap quadrants of the dft so that the low frequencies are in the middle of the image and display it
		shift(source_dft_visual);
		imshow("dft * mask", source_dft_visual);
	}

	// get the inverse dft of the newly created dft, this will get us back the image after the filter has been applied, then display it
	take_inverse_dft(source_dft, target_image);
}

int main() {
	
	// reading in the source image, converting values from 0-255 to 0-1
	Mat original = imread("C:/PandaNoise.bmp", IMREAD_GRAYSCALE);
	Mat original_float;
	original.convertTo(original_float, CV_32FC1, 1.0 / 255.0);

	// create a list of materials, this will hold the masks that will be applied to the source image
	vector<Mat> filter_list;

	// create a low pass filter and add it to the list
	Mat low_pass;
	int low_pass_size_y = 40;
	int low_pass_size_x = low_pass_size_y * ((float)original.cols / (float)original.rows);
	create_filter_mask(original.size(), low_pass, low_pass_size_x, low_pass_size_y, -1, false);
	filter_list.push_back(low_pass);

	// create a high pass filter and add it to the list
	Mat high_pass;
	int high_pass_size_y = 30;
	int high_pass_size_x = high_pass_size_y * ((float)original.cols / (float)original.rows);
	create_filter_mask(original.size(), high_pass, high_pass_size_x, high_pass_size_y, -1, true);
	filter_list.push_back(high_pass);

	// create a high pass filter and add it to the list
	Mat band_pass;
	int band_pass_size_y = 45;
	int band_pass_size_x = band_pass_size_y * ((float)original.cols / (float)original.rows);
	create_filter_mask(original.size(), band_pass, band_pass_size_x, band_pass_size_y, 30, true);
	filter_list.push_back(band_pass);

	for (Mat& filter: filter_list) {
		Mat source_but_filtered;
		apply_frequency_mask(original_float, filter, source_but_filtered, true);
		imshow("inverse of dft * mask", source_but_filtered);
		waitKey();
	}

	return 0;
}