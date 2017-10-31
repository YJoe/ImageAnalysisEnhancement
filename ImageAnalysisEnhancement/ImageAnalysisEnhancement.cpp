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

void create_gaussian_filter(Size& size, Mat& output, int ux, int uy, float sigma_x, float sigma_y, float amplitude = 1.0f) {
	
	Mat temp = Mat(size, CV_32F);

	for (int r = 0; r < size.height; r++) {
		for (int c = 0; c < size.width; c++) {
			float x = (((float)c - ux) * ((float)c - ux)) / (2.0f * sigma_x * sigma_x);
			float y = (((float)r - uy) * ((float)r - uy)) / (2.0f * sigma_y * sigma_y);

			float value = amplitude * exp(-(x + y));
			temp.at<float>(r, c) = value;
		}
	}

	normalize(temp, temp, -1.0f, 1.0f, CV_MINMAX);
	output = temp;
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

Mat createGausFilterMask(Size mask_size, int x, int y, int ksize, bool normalization, bool invert) {
	// Some corrections if out of bounds
	if (x < (ksize / 2)) {
		ksize = x * 2;
	}
	if (y < (ksize / 2)) {
		ksize = y * 2;
	}
	if (mask_size.width - x < ksize / 2) {
		ksize = (mask_size.width - x) * 2;
	}
	if (mask_size.height - y < ksize / 2) {
		ksize = (mask_size.height - y) * 2;
	}

	// call openCV gaussian kernel generator
	double sigma = -1;
	Mat kernelX = getGaussianKernel(ksize, sigma, CV_32F);
	Mat kernelY = getGaussianKernel(ksize, sigma, CV_32F);
	// create 2d gaus
	Mat kernel = kernelX * kernelY.t();
	// create empty mask
	Mat mask = Mat::zeros(mask_size, CV_32F);
	Mat maski = Mat::zeros(mask_size, CV_32F);

	// copy kernel to mask on x,y
	Mat pos(mask, Rect(x - ksize / 2, y - ksize / 2, ksize, ksize));
	kernel.copyTo(pos);

	// create mirrored mask
	Mat posi(maski, Rect((mask_size.width - x) - ksize / 2, (mask_size.height - y) - ksize / 2, ksize, ksize));
	kernel.copyTo(posi);
	// add mirrored to mask
	add(mask, maski, mask);

	// transform mask to range 0..1
	if (normalization) {
		normalize(mask, mask, 0, 1, NORM_MINMAX);
	}

	// invert mask
	if (invert) {
		mask = Mat::ones(mask.size(), CV_32F) - mask;
	}

	return mask;
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

float get_gray_avg(Mat& source) {
	
	float total = 0;
	for (int i = 0; i < source.cols; i++) {
		for (int j = 0; j < source.rows; j++) {
			total += source.at<float>(j, i);
		}
	}
	total /= source.cols * source.rows;
	return total;
}

int main() {

	// GETTING SOURCE IMAGE
	Mat original = imread("C:/PandaNoise.bmp", IMREAD_GRAYSCALE);
	//flip(originalx, original, 1);
	Mat original_float;
	original.convertTo(original_float, CV_32FC1, 1.0 / 255.0);

	// GETTING DFT OF SOURCE
	Mat source_dft; 
	Mat source_dft_visual;
	take_dft(original_float, source_dft);
	get_visual_of_dft(source_dft, source_dft_visual);
	cout << "dft avg " << get_gray_avg(source_dft_visual) << endl;
	imshow("source", original_float);
	shift(source_dft_visual);
	imshow("dft shifted", source_dft_visual);
	shift(source_dft_visual);
	imshow("dft", source_dft_visual);

	// CREATING MASK
	Mat mask;
	int b_size_y = 5;
	int b_size_x = b_size_y * ((float)original.cols / (float)original.rows);
	//int b_size_x = 70;
	//create_blank_filter(original.size(), mask, 1);
	create_filter_mask(original.size(), mask, b_size_x, b_size_y, -1, false);
	//create_gaussian_filter(original.size(), mask, original.size().width / 2, original.size().height / 2, 65, 35, 900);
	//mask = createGausFilterMask(original_float.size(), original_float.cols / 2, original_float.rows / 2, 50, true, false);
	//Mat mask2 = imread("C:/mask2.bmp", 0);
	//mask2.convertTo(mask, CV_32FC1, 1.0 / 255.0);

	imshow("mask", mask);
	shift(mask);
	imshow("mask shifted", mask);

	// COMBINING MASK AND SOURCE
	Mat planes[] = {
		Mat::zeros(source_dft.size(), CV_32F),
		Mat::zeros(source_dft.size(), CV_32F)
	};
	Mat kernel_spec;
	planes[0] = mask; // real
	//planes[1] = mask; // imaginary
	merge(planes, 2, kernel_spec);

	mulSpectrums(source_dft, kernel_spec, source_dft, 0);

	// READING STUFF
	get_visual_of_dft(source_dft, source_dft_visual);
	imshow("dft * mask", source_dft_visual);
	shift(source_dft_visual);
	imshow("dft * mask shifted", source_dft_visual);
	cout << "dft avg " << get_gray_avg(source_dft_visual) << endl;

	Mat idft;
	take_inverse_dft(source_dft, idft);

	imshow("idft of dft * mask", idft);

	

	waitKey(); 
}