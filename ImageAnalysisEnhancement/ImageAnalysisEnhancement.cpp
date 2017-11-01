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


// Spatial filtering functions

void spatial_box_blur(Mat& source_image, Mat& target_image, int neighbourhood_size) {
	cout << "Box blur with neighbourhood [" << neighbourhood_size << "]" << endl;
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			int total = 0;
			int sample_count = 0;
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					if (j + l > -1 && j + l < source_image.cols && i + k > -1 && i + k < source_image.rows) {
						sample_count += 1;
						total += (int)source_image.at<uchar>(Point(j + l, i + k));
					}
				}
			}
			total /= sample_count;

			target_image.at<uchar>(Point(j, i)) = total;
		}
	}
}

void spatial_gaussian_blur(Mat& source_image, Mat& target_image, int neighbourhood_size) {
	cout << "Gaussian blur with neighbourhood [" << neighbourhood_size << "]" << endl;
	double min = 0.0;
	double max = distance(0, 0, neighbourhood_size / 2 + 1, neighbourhood_size / 2 + 1);

	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			double total = 0;
			int sample_count = 0;
			double total_distance_weight = 0;
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					if (j + l > -1 && j + l < source_image.cols && i + k > -1 && i + k < source_image.rows) {
						sample_count += 1;

						double distance_weight = max - distance(j, i, j + l, i + k);
						total_distance_weight += distance_weight;
						total += (int)source_image.at<uchar>(Point(j + l, i + k)) * distance_weight;

					}
				}
			}

			total /= total_distance_weight;

			target_image.at<uchar>(Point(j, i)) = total;
		}
	}
}

void spatial_negative(Mat& source_image, Mat& target_image) {
	cout << "Negative" << endl;
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			target_image.at<uchar>(Point(j, i)) = 255 - source_image.at<uchar>(Point(j, i));
		}
	}
}

void binary_threshold(Mat& source_image, Mat& target_image, int threshold) {
	cout << "\nBinary threshold" << endl;
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			int avg = source_image.at<uchar>(Point(j, i));
			if (avg > threshold) {
				target_image.at<uchar>(Point(j, i)) = 255;
			}
			else {
				target_image.at<uchar>(Point(j, i)) = 0;
			}
		}
	}
}

int get_image_grey_avg(Mat& source_image) {
	int avg = 0;
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			avg += source_image.at<uchar>(Point(j, i));
		}
	}
	return avg / (source_image.rows * source_image.cols);
}

void binary_threshold_auto(Mat& source_image, Mat& target_image) {
	int avg = get_image_grey_avg(source_image);
	binary_threshold(source_image, target_image, avg);
}

void spatial_median(Mat& source_image, Mat& target_image, int neighbourhood_size ) {
	cout << "Median smoothing" << endl;
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			vector<int> shade;
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					if (j + l > -1 && j + l < source_image.cols && i + k > -1 && i + k < source_image.rows) {
						shade.emplace_back(source_image.at<uchar>(Point(j + l, i + k)));
					}
				}
			}

			sort(shade.begin(), shade.end());

			target_image.at<uchar>(Point(j, i)) = shade[size(shade) / 2];
		}
	}
}


// Frequency filtering functions

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

void ellipse(Mat& image, int x, int y, int size_x, int size_y, int stroke) {
	ellipse(image, Point(x, y), Size(size_x, size_y), 0, 0, 360, Scalar(255, 0, 0), stroke, 8);
}

void init_filter(Mat& mask, Size size) {
	mask = Mat::zeros(size.height, size.width, CV_32F);
	normalize(mask, mask, 0, 1, CV_MINMAX);
}

void invert_filter(Mat& mask) {
	mask = Mat::ones(mask.size(), CV_32F) - mask;
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

void really_specific_mask(Mat& mask) {
	int stroke = 5;

	// TOP STUFF
	rectangle(mask, Point(39, 23), Point(39, 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(115, 23), Point(115, 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 115, 23), Point(mask.cols - 115, 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 39, 23), Point(mask.cols - 39, 23), Scalar(0, 0, 255), stroke);

	rectangle(mask, Point(39, 64), Point(39, 64), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(115, 64), Point(115, 64), Scalar(0, 0, 255), stroke / 2);
	rectangle(mask, Point(mask.cols - 39, 64), Point(mask.cols - 39, 64), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 115, 64), Point(mask.cols - 115, 64), Scalar(0, 0, 255), stroke / 2);

	// BOTTOM STUFF
	rectangle(mask, Point(39, mask.rows - 23), Point(39, mask.rows - 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(115, mask.rows - 23), Point(115, mask.rows - 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 115, mask.rows - 23), Point(mask.cols - 115, mask.rows - 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 39, mask.rows - 23), Point(mask.cols - 39, mask.rows - 23), Scalar(0, 0, 255), stroke);

	rectangle(mask, Point(39, mask.rows - 64), Point(39, mask.rows - 64), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(115, mask.rows - 64), Point(115, mask.rows - 64), Scalar(0, 0, 255), stroke / 2);
	rectangle(mask, Point(mask.cols - 39, mask.rows - 64), Point(mask.cols - 39, mask.rows - 64), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 115, mask.rows - 64), Point(mask.cols - 115, mask.rows - 64), Scalar(0, 0, 255), stroke / 2);

	// MIDDLE TOP STUFF
	rectangle(mask, Point(mask.cols / 2, 23), Point(mask.cols / 2, 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols / 2, 58), Point(mask.cols / 2, 70), Scalar(0, 0, 255), stroke / 2);

	// MIDDLE BOTTOM STUFF
	rectangle(mask, Point(mask.cols / 2, mask.rows - 23), Point(mask.cols / 2, mask.rows - 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols / 2, mask.rows - 58), Point(mask.cols / 2, mask.rows - 70), Scalar(0, 0, 255), stroke / 2);

	// MIDDLE OUTER STUFF
	rectangle(mask, Point(39, mask.rows / 2), Point(39, mask.rows / 2), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 39, mask.rows / 2), Point(mask.cols - 39, mask.rows / 2), Scalar(0, 0, 255), stroke);

	// MIDDLE INNER STUFF
	rectangle(mask, Point(105, mask.rows / 2), Point(120, mask.rows / 2), Scalar(0, 0, 255), stroke / 2);
	rectangle(mask, Point(mask.cols - 105, mask.rows / 2), Point(mask.cols - 120, mask.rows / 2), Scalar(0, 0, 255), stroke / 2);
}

void create_filter_mask(Size size, Mat& mask, int x_size, int y_size, int stroke, bool invert) {
	mask = Mat::zeros(size.height, size.width, CV_32F);
	ellipse(mask, mask.cols / 2, mask.rows / 2, x_size, y_size, stroke);

	normalize(mask, mask, 0, 1, CV_MINMAX);

	if (invert) {
		mask = Mat::ones(mask.size(), CV_32F) - mask;
	}
}

void normalise(Mat& mask) {
	normalize(mask, mask, 0, 1, CV_MINMAX);
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


// Error checking

void mse_float_input(Mat& source, Mat& target) {
	
	float total = 0;
	for (int i = 0; i < source.cols; i++) {
		for (int j = 0; j < source.rows; j++) {
			total += pow(map_value(source.at<float>(j, i), 0.0, 1.0, 0.0, 255.0) - map_value(target.at<float>(j, i), 0.0, 1.0, 0.0, 255.0), 2);
		}
	}

	cout << "Total error [" << total << "]" << endl;
	total /= (source.cols * source.rows);
	cout << "Mean square error of filtered compared to the no noise image [" << total << "]\n" << endl;
}

void mse_int_input(Mat& source, Mat& target) {

	float total = 0;
	for (int i = 0; i < source.cols; i++) {
		for (int j = 0; j < source.rows; j++) {
			total += pow(source.at<uchar>(j, i) - target.at<uchar>(j, i), 2);
		}
	}

	cout << "Total error [" << total << "]" << endl;
	total /= (source.cols * source.rows);
	cout << "Mean square error of filtered compared to the no noise image [" << total << "]\n" << endl;
}



int main() {

	// load the image with no noise, converting values from 0-255 to 0-1 for frequency use
	Mat no_noise = imread("C:/PandaOriginal.bmp", IMREAD_GRAYSCALE);
	Mat no_noise_float;
	no_noise.convertTo(no_noise_float, CV_32FC1, 1.0 / 255.0);

	// reading in the source image, converting values from 0-255 to 0-1
	Mat original = imread("C:/PandaNoise.bmp", IMREAD_GRAYSCALE);
	Mat original_float;
	original.convertTo(original_float, CV_32FC1, 1.0 / 255.0);

	// comparing the clean image to the noise image
	cout << "No filter" << endl;
	mse_float_input(original_float, no_noise_float);

	float image_aspect_ratio = ((float)original.cols / (float)original.rows);

	// SPATIAL FILTERING
	Mat box_output = original.clone();
	spatial_box_blur(original, box_output, 10);
	mse_int_input(original, box_output);
	imshow("box blur", box_output);
	waitKey();

	Mat gaussian_output = original.clone();
	spatial_gaussian_blur(original, gaussian_output, 10);
	mse_int_input(original, gaussian_output);
	imshow("gaussian blur", gaussian_output);
	waitKey();

	Mat negative_output = original.clone();
	spatial_negative(original, negative_output);
	mse_int_input(original, negative_output);
	imshow("negative", negative_output);
	waitKey();

	Mat binary_output = original.clone();
	binary_threshold(original, binary_output, 50);
	mse_int_input(original, binary_output);
	imshow("binary threshold", binary_output);
	waitKey();

	Mat auto_binary_output = original.clone();
	binary_threshold_auto(original, auto_binary_output);
	mse_int_input(original, auto_binary_output);
	imshow("auto binary threshold", auto_binary_output);
	waitKey();

	Mat median_output = original.clone();
	spatial_median(original, median_output, 5);
	mse_int_input(original, median_output);
	imshow("median", median_output);
	waitKey();


	// FREQUENCY FILTERING

	// create a list of materials, this will hold the masks that will be applied to the source image
	vector<Mat> filter_list;

	// create a list of the filter names to display with the debug details
	vector<string> filter_name_list;

	// create a low pass filter and add it to the list
	Mat low_pass;
	int low_pass_size = 20;
	init_filter(low_pass, original.size());
	ellipse(low_pass, low_pass.cols / 2, low_pass.rows / 2, low_pass_size * image_aspect_ratio, low_pass_size, -1);
	normalise(low_pass);
	filter_list.push_back(low_pass);
	filter_name_list.push_back("low pass");

	// create a high pass filter and add it to the list
	Mat high_pass;
	int high_pass_size = 20;
	init_filter(high_pass, original.size());
	ellipse(high_pass, high_pass.cols / 2, high_pass.rows / 2, high_pass_size * image_aspect_ratio, high_pass_size, -1);
	normalise(high_pass);
	invert_filter(high_pass);
	filter_list.push_back(high_pass);
	filter_name_list.push_back("high pass");

	// create a band stop filter and add it to the list
	Mat band_stop_0;
	int band_stop_0_size = 45;
	int band_stroke_0 = 8;
	init_filter(band_stop_0, original.size());
	ellipse(band_stop_0, band_stop_0.cols / 2, band_stop_0.rows / 2, band_stop_0_size * image_aspect_ratio, band_stop_0_size, band_stroke_0);
	normalise(band_stop_0);
	invert_filter(band_stop_0);
	filter_list.push_back(band_stop_0);
	filter_name_list.push_back("band stop 0");

	// create a band stop filter and add it to the list
	Mat multi_band_stop_1;
	int multi_band_stop_1_size_1 = 45;
	int multi_band_stop_1_size_2 = 95;
	int band_stroke_1 = 8;
	init_filter(multi_band_stop_1, original.size());
	ellipse(multi_band_stop_1, multi_band_stop_1.cols / 2, multi_band_stop_1.rows / 2, multi_band_stop_1_size_1 * image_aspect_ratio, multi_band_stop_1_size_1, band_stroke_1);
	ellipse(multi_band_stop_1, multi_band_stop_1.cols / 2, multi_band_stop_1.rows / 2, multi_band_stop_1_size_2 * image_aspect_ratio, multi_band_stop_1_size_2, band_stroke_1);
	normalise(multi_band_stop_1);
	invert_filter(multi_band_stop_1);
	filter_list.push_back(multi_band_stop_1);
	filter_name_list.push_back("band stop 1");

	// create a band stop filter and add it to the list
	Mat multi_band_stop_2;
	int multi_band_stop_2_size_1 = 45;
	int multi_band_stop_2_size_2 = 95;
	int multi_band_stop_2_size_3 = 122;
	int band_stroke_2 = 8;
	init_filter(multi_band_stop_2, original.size());
	ellipse(multi_band_stop_2, multi_band_stop_2.cols / 2, multi_band_stop_2.rows / 2, multi_band_stop_2_size_1 * image_aspect_ratio, multi_band_stop_2_size_1, band_stroke_2);
	ellipse(multi_band_stop_2, multi_band_stop_2.cols / 2, multi_band_stop_2.rows / 2, multi_band_stop_2_size_2 * image_aspect_ratio, multi_band_stop_2_size_2, band_stroke_2);
	ellipse(multi_band_stop_2, multi_band_stop_2.cols / 2, multi_band_stop_2.rows / 2, multi_band_stop_2_size_3 * image_aspect_ratio, multi_band_stop_2_size_3, band_stroke_2);
	normalise(multi_band_stop_2);
	invert_filter(multi_band_stop_2);
	filter_list.push_back(multi_band_stop_2);
	filter_name_list.push_back("band stop 2");

	// create a specific pass filter and add it to the list
	Mat specific_pass = Mat::zeros(original.size().height, original.size().width, CV_32F);
	init_filter(specific_pass, original.size());
	invert_filter(specific_pass);
	really_specific_mask(specific_pass);
	filter_list.push_back(specific_pass);
	filter_name_list.push_back("specific pass");

	// make sure that there are filter names for all filters applied
	assert(size(filter_list) == size(filter_name_list));

	for (int i = 0; i < size(filter_list); i++) {
		cout << "Filter [" << filter_name_list[i] << "]" << endl;
		Mat source_but_filtered;
		apply_frequency_mask(original_float, filter_list[i], source_but_filtered, true);
		mse_float_input(source_but_filtered, no_noise_float);
		imshow("inverse of dft * mask", source_but_filtered);
		waitKey();
	}

	return 0;
}