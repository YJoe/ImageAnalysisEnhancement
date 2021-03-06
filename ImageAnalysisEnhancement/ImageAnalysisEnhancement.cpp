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
	
	// for all pixels in the image
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			int total = 0;
			int sample_count = 0;
			
			// for all pixels within the neighbourhood 
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					
					// check that the pixel is within the correct range
					if (j + l > -1 && j + l < source_image.cols && i + k > -1 && i + k < source_image.rows) {
						
						// note that the pixel is valid and that we are using it within the total
						sample_count += 1;

						// add the pixel's value to the total
						total += (int)source_image.at<uchar>(Point(j + l, i + k));
					}
				}
			}

			// get the average of the neighbourhood by dividing by the sample count
			total /= sample_count;

			// set the pixel we are operating on to the average of the neighbourhood
			target_image.at<uchar>(Point(j, i)) = total;
		}
	}
}

void spatial_gaussian_blur(Mat& source_image, Mat& target_image, int neighbourhood_size) {
	cout << "Gaussian blur with neighbourhood [" << neighbourhood_size << "]" << endl;

	// define the minimum distance that a pixel can be from another
	double min = 0.0;

	// define the maximum distance a pixel can be from another by using the neighbourhood size
	double max = distance(0, 0, neighbourhood_size / 2 + 1, neighbourhood_size / 2 + 1);

	// for all pixels in the image
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			double total = 0;
			int sample_count = 0;
			double total_distance_weight = 0;

			// for all pixels within the neighbourhood
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					
					// check that the pixel is within the correct range
					if (j + l > -1 && j + l < source_image.cols && i + k > -1 && i + k < source_image.rows) {
						
						// note that the pixel is valid and that we are using it within the total
						sample_count += 1;

						// calculate the distance between the current pixel and the current neighbourhood pixel
						// a greater distance will be lower because we are subtracting it from the max distance
						double distance_weight = max - distance(j, i, j + l, i + k);

						// add the calculated distance to the total distance
						total_distance_weight += distance_weight;

						// add to the total the pixel's value * the distance weight
						total += (int)source_image.at<uchar>(Point(j + l, i + k)) * distance_weight;
					}
				}
			}

			// divide the total by the total distance weight of the neighbour hood to get the weighted average
			total /= total_distance_weight;

			// set the current pixel to the neighbourhood's weighted average
			target_image.at<uchar>(Point(j, i)) = total;
		}
	}
}

void spatial_negative(Mat& source_image, Mat& target_image) {
	cout << "Negative" << endl;

	// for all pixels in the image
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {

			// set the pixel value to be the maximum pixel value minus the pixel value
			target_image.at<uchar>(Point(j, i)) = 255 - source_image.at<uchar>(Point(j, i));
		}
	}
}

void binary_threshold(Mat& source_image, Mat& target_image, int threshold) {
	cout << "\nBinary threshold" << endl;

	// for all pixels within the image
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {

			// get the image value
			int val = source_image.at<uchar>(Point(j, i));

			// if the value is greater than the threshold, set the pixel value to the max
			if (val > threshold) {
				target_image.at<uchar>(Point(j, i)) = 255;
			}

			// if the value is less than or equal to the threshold, set the pixel value to the lowest
			else {
				target_image.at<uchar>(Point(j, i)) = 0;
			}
		}
	}
}

int get_image_grey_avg(Mat& source_image) {
	int avg = 0;

	// for all pixels in the image
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			
			// add the current pixels value to the total
			avg += source_image.at<uchar>(Point(j, i));
		}
	}

	// return the total divided by the pixel count
	return avg / (source_image.rows * source_image.cols);
}

void binary_threshold_auto(Mat& source_image, Mat& target_image) {

	// get the average image gray value
	int avg = get_image_grey_avg(source_image);
	
	// operate on the target image using the average threshold worked out by the other function
	binary_threshold(source_image, target_image, avg);
}

void spatial_median(Mat& source_image, Mat& target_image, int neighbourhood_size ) {
	cout << "Median smoothing" << endl;

	// for all pixels within the image
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			
			// define a vector to store the neighbourhood's values in
			vector<int> shade;

			// for the size of the neighbourhood
			for (int k = -(neighbourhood_size / 2); k < (neighbourhood_size / 2) + 1; k++) {
				for (int l = -(neighbourhood_size / 2); l < (neighbourhood_size / 2) + 1; l++) {
					
					// check that the pixel is within the image bounds
					if (j + l > -1 && j + l < source_image.cols && i + k > -1 && i + k < source_image.rows) {
						
						// add to the vector the value found at this pixel
						shade.emplace_back(source_image.at<uchar>(Point(j + l, i + k)));
					}
				}
			}

			// sort the vector by size
			sort(shade.begin(), shade.end());

			// set the current pixel value to the middle value within the sorted vector
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

	// perform the dft calculation of the dft_ready image and store the output in the destination image
	dft(dft_ready, destination, DFT_COMPLEX_OUTPUT);
}

void take_inverse_dft(Mat& source_image, Mat& destination){

	// call the dft function with flags to invert the image
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

	// normalise the image within the values 0.0 - 1.0
	normalize(destination_dft, destination_dft, 0, 1, CV_MINMAX);
}

void ellipse(Mat& image, int x, int y, int size_x, int size_y, int stroke) {

	// this function just simplifies the call to the opencv elipse function
	ellipse(image, Point(x, y), Size(size_x, size_y), 0, 0, 360, Scalar(255, 0, 0), stroke, 8);
}

void init_filter(Mat& mask, Size size) {

	// create a blank image with the size that we need, convert the image to the range 0-1
	mask = Mat::zeros(size.height, size.width, CV_32F);
	normalize(mask, mask, 0, 1, CV_MINMAX);
}

void invert_filter(Mat& mask) {
	mask = Mat::ones(mask.size(), CV_32F) - mask;
}

void shift(Mat& magnitude) {

	// crop if it has an odd number of rows or columns
	magnitude = magnitude(Rect(0, 0, magnitude.cols & -2, magnitude.rows & -2));

	// find the center x and center y of the image
	int cx = magnitude.cols / 2;
	int cy = magnitude.rows / 2;

	// get quadrants of the image
	Mat q0(magnitude, Rect(0, 0, cx, cy));   // Top-Left
	Mat q1(magnitude, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(magnitude, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(magnitude, Rect(cx, cy, cx, cy)); // Bottom-Right

	Mat temp;

	// swap quadrant top left with bottom right
	q0.copyTo(temp);
	q3.copyTo(q0);
	temp.copyTo(q3);
	
	// swap quadrant top right with bottom left
	q1.copyTo(temp);
	q2.copyTo(q1);
	temp.copyTo(q2);
}

void really_specific_mask(Mat& mask) {
	// this function creates a very specific mask to use on the noise image
	// it draws rectangles over the significant peaks within the image
	
	int stroke = 5;

	// TOP
	rectangle(mask, Point(39, 23), Point(39, 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(115, 23), Point(115, 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 115, 23), Point(mask.cols - 115, 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 39, 23), Point(mask.cols - 39, 23), Scalar(0, 0, 255), stroke);

	rectangle(mask, Point(39, 64), Point(39, 64), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(115, 64), Point(115, 64), Scalar(0, 0, 255), stroke / 2);
	rectangle(mask, Point(mask.cols - 39, 64), Point(mask.cols - 39, 64), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 115, 64), Point(mask.cols - 115, 64), Scalar(0, 0, 255), stroke / 2);

	// BOTTOM
	rectangle(mask, Point(39, mask.rows - 23), Point(39, mask.rows - 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(115, mask.rows - 23), Point(115, mask.rows - 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 115, mask.rows - 23), Point(mask.cols - 115, mask.rows - 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 39, mask.rows - 23), Point(mask.cols - 39, mask.rows - 23), Scalar(0, 0, 255), stroke);

	rectangle(mask, Point(39, mask.rows - 64), Point(39, mask.rows - 64), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(115, mask.rows - 64), Point(115, mask.rows - 64), Scalar(0, 0, 255), stroke / 2);
	rectangle(mask, Point(mask.cols - 39, mask.rows - 64), Point(mask.cols - 39, mask.rows - 64), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 115, mask.rows - 64), Point(mask.cols - 115, mask.rows - 64), Scalar(0, 0, 255), stroke / 2);

	// MIDDLE TOP
	rectangle(mask, Point(mask.cols / 2, 23), Point(mask.cols / 2, 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols / 2, 58), Point(mask.cols / 2, 70), Scalar(0, 0, 255), stroke / 2);

	// MIDDLE BOTTOM
	rectangle(mask, Point(mask.cols / 2, mask.rows - 23), Point(mask.cols / 2, mask.rows - 23), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols / 2, mask.rows - 58), Point(mask.cols / 2, mask.rows - 70), Scalar(0, 0, 255), stroke / 2);

	// MIDDLE OUTER
	rectangle(mask, Point(39, mask.rows / 2), Point(39, mask.rows / 2), Scalar(0, 0, 255), stroke);
	rectangle(mask, Point(mask.cols - 39, mask.rows / 2), Point(mask.cols - 39, mask.rows / 2), Scalar(0, 0, 255), stroke);

	// MIDDLE INNER
	rectangle(mask, Point(105, mask.rows / 2), Point(120, mask.rows / 2), Scalar(0, 0, 255), stroke / 2);
	rectangle(mask, Point(mask.cols - 105, mask.rows / 2), Point(mask.cols - 120, mask.rows / 2), Scalar(0, 0, 255), stroke / 2);
}


void normalise(Mat& mask) {

	// convert an image from the range 0-255 to the range 0-1
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

	// for all pixels in the image
	for (int i = 0; i < source.cols; i++) {
		for (int j = 0; j < source.rows; j++) {
			
			// convert the pixel value from the range 0-1 to the range 0-255 and add it ^2 to the total
			total += pow(map_value(source.at<float>(j, i), 0.0, 1.0, 0.0, 255.0) - map_value(target.at<float>(j, i), 0.0, 1.0, 0.0, 255.0), 2);
		}
	}

	// get the average error by dividing by the pixel count
	total /= (source.cols * source.rows);

	// display the MSE
	cout << "Mean square error of filtered compared to the no noise image [" << total << "]" << endl;
}

void mse_int_input(Mat& source, Mat& target) {

	float total = 0;
	
	// for all pixels in the image
	for (int i = 0; i < source.cols; i++) {
		for (int j = 0; j < source.rows; j++) {
			
			// add the pixel value^2 to the total
			total += pow(source.at<uchar>(j, i) - target.at<uchar>(j, i), 2);
		}
	}
	
	// divide the total error by the pixel count to get an average
	total /= (source.cols * source.rows);

	// display the MSE
	cout << "Mean square error of filtered compared to the no noise image [" << total << "]" << endl;
}

void mae_float_input(Mat& source, Mat& target) {

	float total = 0;

	// for all pixels in the image
	for (int i = 0; i < source.cols; i++) {
		for (int j = 0; j < source.rows; j++) {

			// convert the pixel value from the range 0-1 to the range 0-255 and add it ^2 to the total
			total += abs(map_value(source.at<float>(j, i), 0.0, 1.0, 0.0, 255.0) - map_value(target.at<float>(j, i), 0.0, 1.0, 0.0, 255.0));
		}
	}

	// get the average error by dividing by the pixel count
	total /= (source.cols * source.rows);

	// display the MSE
	cout << "Mean absolute error of filtered compared to the no noise image [" << total << "]\n" << endl;
}

void mae_int_input(Mat& source, Mat& target) {

	float total = 0;

	// for all pixels in the image
	for (int i = 0; i < source.cols; i++) {
		for (int j = 0; j < source.rows; j++) {

			// add the pixel value^2 to the total
			total += abs(source.at<uchar>(j, i) - target.at<uchar>(j, i));
		}
	}

	// divide the total error by the pixel count to get an average
	total /= (source.cols * source.rows);

	// display the MSE
	cout << "Mean absolute error of filtered compared to the no noise image [" << total << "]\n" << endl;
}


int main() {

	// load the image with no noise, converting values from 0-255 to 0-1 for frequency use
	Mat no_noise = imread("data/PandaOriginal.bmp", IMREAD_GRAYSCALE);
	Mat no_noise_float;
	no_noise.convertTo(no_noise_float, CV_32FC1, 1.0 / 255.0);

	// reading in the source image, converting values from 0-255 to 0-1
	Mat original = imread("data/PandaNoise.bmp", IMREAD_GRAYSCALE);
	Mat original_float;
	original.convertTo(original_float, CV_32FC1, 1.0 / 255.0);

	// comparing the clean image to the noise image
	cout << "No filter" << endl;
	mse_float_input(original_float, no_noise_float);
	mae_float_input(original_float, no_noise_float);

	float image_aspect_ratio = ((float)original.cols / (float)original.rows);

	// SPATIAL FILTERING
	// a box blur of the source image using a neighbourhood size of 5
	Mat box_output = original.clone();
	spatial_box_blur(original, box_output, 3);
	mse_int_input(original, box_output);
	mae_int_input(original, box_output);
	imshow("box blur", box_output);
	waitKey();

	// a gaussian blur usign a neighbourhood size of 5
	Mat gaussian_output = original.clone();
	spatial_gaussian_blur(original, gaussian_output, 3);
	mse_int_input(original, gaussian_output);
	mae_int_input(original, gaussian_output);
	imshow("gaussian blur", gaussian_output);
	waitKey();

	// inverting the image
	Mat negative_output = original.clone();
	spatial_negative(original, negative_output);
	mse_int_input(original, negative_output);
	mae_int_input(original, negative_output);
	imshow("negative", negative_output);
	waitKey();

	// performing a binary threshold of 50
	Mat binary_output = original.clone();
	binary_threshold(original, binary_output, 50);
	mse_int_input(original, binary_output);
	mae_int_input(original, binary_output);
	imshow("binary threshold", binary_output);
	waitKey();

	// getting the threshold of the imae automatically and performing a binary threshold
	Mat auto_binary_output = original.clone();
	binary_threshold_auto(original, auto_binary_output);
	mse_int_input(original, auto_binary_output);
	mae_int_input(original, auto_binary_output);
	imshow("auto binary threshold", auto_binary_output);
	waitKey();

	// a median sample of an image using a neighbourhood of size 3
	Mat median_output = original.clone();
	spatial_median(original, median_output, 3);
	mse_int_input(original, median_output);
	mae_int_input(original, median_output);
	imshow("median", median_output);
	waitKey();

	// performing a gaussian blur of size 5 followed by an auto binary threshold
	Mat gaussian_auto_binary_output = original.clone();
	spatial_gaussian_blur(original, gaussian_auto_binary_output, 5);
	binary_threshold_auto(gaussian_auto_binary_output, gaussian_auto_binary_output);
	mse_int_input(original, gaussian_auto_binary_output);
	mae_int_input(original, gaussian_auto_binary_output);
	imshow("Gaussian auto binary threshold", gaussian_auto_binary_output);
	waitKey();

	// FREQUENCY FILTERING

	// create a list of materials, this will hold the masks that will be applied to the source image
	vector<Mat> filter_list;

	// create a list of the filter names to display with the debug details
	vector<string> filter_name_list;

	// create a low pass filter and add it to the list
	Mat no_filter;
	init_filter(no_filter, original.size());
	normalise(no_filter);
	invert_filter(no_filter); 
	filter_list.push_back(no_filter);
	filter_name_list.push_back("no filter");

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
		mae_float_input(source_but_filtered, no_noise_float);
		imshow("inverse of dft * mask", source_but_filtered);
		waitKey();
	}

	return 0;
}