/*
 * sobel.cpp
 * 
 * Created on: Sep 9, 2013
 * 			Author: Amir Yazdanbakhsh <a.yazdanbakhsh@gatech.edu>
 */
#include "rgb_image.hpp"
#include "convolution.hpp"
#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>

#include "approximate.hpp"

int k_bits;

bool approximate ( std::vector<std::vector<boost::shared_ptr<Pixel> > > & pixels, int start_px, int end_px);
double mse ( std::vector<std::vector<boost::shared_ptr<Pixel> > > p1,
			 std::vector<std::vector<boost::shared_ptr<Pixel> > > p2,
			 int start_px, int end_px);
double rms(std::vector<std::vector<boost::shared_ptr<Pixel> > > p1,
		   std::vector<std::vector<boost::shared_ptr<Pixel> > > p2);

void edge_highlight ( boost::shared_ptr<Image> src,
					  boost::shared_ptr<Image> dst );

FILE * fp;

void print_diff(std::vector<std::vector<boost::shared_ptr<Pixel> > > p1,
				std::vector<std::vector<boost::shared_ptr<Pixel> > > p2) {
	for (int i = 0; i < (int)p1.size(); i++)
		for (int j = 0; j < (int)p1[i].size(); j++) {
			if (p1[i][j]->r != p2[i][j]->r) {
				fprintf(fp, "(%d, %d) %d %d\n", i, j, p1[i][j]->r, p2[i][j]->r);
				fflush(fp);
				//return;
			}
		}
}

int main ( int argc, const char* argv[])
{
	int appx_level;
	int img_size_bytes, noutput, nwidth;

	// Source and destination image
	boost::shared_ptr<Image> srcImagePtr(new Image());
	boost::shared_ptr<Image> dstImagePtr(new Image());
	boost::shared_ptr<Image> appxImagePtr(new Image());
	boost::shared_ptr<Image> accImagePtr(new Image());

	//parse command line args here
	if (argc != 3 ||
		(appx_level = atoi(argv[2])) < 1 ||
		appx_level > N_LEVEL){
		printf("Usage: sobel.out srcImage appxLevel[1-%d]\n", N_LEVEL);
		return 1;
	}
	k_bits = K_LEVELS[appx_level - 1];

	srcImagePtr->loadRgbImage( argv[1] ); // source image
	srcImagePtr->makeGrayscale( ); // convert the source file to grayscale

	//get accurate output
	accImagePtr->loadRgbImage(argv[1]);
	edge_highlight(srcImagePtr, accImagePtr);
	accImagePtr->saveRgbImage("accurate.rgb");

	//calculate printing parameters
	img_size_bytes = srcImagePtr->height * srcImagePtr->width * UNIT_BYTE;
	noutput = img_size_bytes / CACHE_LINE_BYTE;
	for (nwidth = 0; noutput != 0; noutput /= 10)
		nwidth++;
	noutput = img_size_bytes / CACHE_LINE_BYTE;

	//file of statistics
	char fname[30];
	sprintf(fname, "diff2loss_appxl%d", appx_level);
	fp = fopen(fname, "w");
	if (fp == NULL)
		printf("NULL\n");

	//start approximation and collect statistics
	for (int i = 0; (i + 1) * CACHE_LINE_BYTE < img_size_bytes; i++) {
		dstImagePtr->loadRgbImage(argv[1]); // destination image
		appxImagePtr->loadRgbImage(argv[1]); // approximate image
		appxImagePtr->makeGrayscale();

		//generate output filename
		char buffer1[30], buffer2[30];
		sprintf(buffer1, "output_%%0%dd.rgb", nwidth);

		//do approximation
		int start_bytes = i * CACHE_LINE_BYTE,
			end_bytes = (i + 1) * CACHE_LINE_BYTE;
		bool is_lossy;
		is_lossy = approximate(appxImagePtr->pixels, start_bytes/UNIT_BYTE, end_bytes/UNIT_BYTE);

		if (is_lossy) {
			//collect difference
			double diff = mse(srcImagePtr->pixels, appxImagePtr->pixels,
							  start_bytes/UNIT_BYTE, end_bytes/UNIT_BYTE);

			//do_sobel
			edge_highlight(appxImagePtr, dstImagePtr);

			//collect final loss
			double loss = rms(accImagePtr->pixels, dstImagePtr->pixels);

			//save result
			fprintf(fp, "%s\tdiff\t%g\tloss\t%g\n", buffer2, diff, loss);
			fflush(fp);

			dstImagePtr->saveRgbImage(buffer2);
		}

		//print information
		sprintf(buffer2, buffer1, i);
		for (int k = 0; k < 60; k++)
			printf("\b");
 		printf("output: %s total %.2f%%", buffer2, (i+1.0) / noutput * 100);
		fflush(stdout);
	}

	printf("\n");

	fclose(fp);

	return 0 ;
}

inline int row(int px_num, int width) { return px_num / width; }
inline int col(int px_num, int width) { return px_num % width; }

bool approximate(std::vector<std::vector<boost::shared_ptr<Pixel> > > & pixels,
				 int start_px, int end_px) {
	int base;  //the minimum value
	int lossless_bits, lossy_bits;

	int width = pixels[0].size();

	//select base
	base = pixels[row(start_px, width)][col(start_px, width)]->r;
	for (int i = start_px + 1; i < end_px; i++) {
		int r = row(i, width), c = col(i, width);
		if (pixels[r][c]->r < base)
			base = pixels[r][c]->r;
	}

	//calculate lossless/lossy number of bits
	int max_delta = -1;
	for (int i = start_px; i < end_px; i++) {
		int r = row(i, width), c = col(i, width);
		int delta;

		delta = pixels[r][c]->r - base;
		if (delta > max_delta)
			max_delta = delta;
	}
	lossless_bits = 0;
	for (int i = UNIT_BIT - 1; i >= 0 && (max_delta & (1 << i)) == 0; i--)
		lossless_bits++;

	lossy_bits = 0;
	if ((UNIT_BIT - k_bits) > lossless_bits)
		lossy_bits = UNIT_BIT - k_bits - lossless_bits;
	//no loss
	if (lossy_bits == 0)
		return false;

	//do approximation
	for (int i = start_px; i < end_px; i++) {
		int r = row(i, width), c = col(i, width);
		int appx;

		int delta = pixels[r][c]->r - base;
		delta &= ~1 << (lossy_bits - 1);
		appx = delta + base;

		pixels[r][c]->r = pixels[r][c]->g = pixels[r][c]->b = appx;
	}

	return true;
}

double mse ( std::vector<std::vector<boost::shared_ptr<Pixel> > > p1,
			 std::vector<std::vector<boost::shared_ptr<Pixel> > > p2,
			 int start_px, int end_px) {
	int width = p1[0].size();
	long sum = 0;
	for (int i = start_px; i < end_px; i++) {
		int r = row(i, width), c = col(i, width);

		sum += pow(p1[r][c]->r - p2[r][c]->r, 2);
	}

	return (double)sum / (end_px - start_px);
}

double rms(std::vector<std::vector<boost::shared_ptr<Pixel> > > p1,
		   std::vector<std::vector<boost::shared_ptr<Pixel> > > p2){
	long sum = 0;
	for (size_t i = 0; i < p1.size(); i++)
		for (size_t j = 0; j < p1[i].size(); j++)
			sum += pow(p1[i][j]->r - p2[i][j]->r, 2);

	return sqrt((double)sum / (p1.size() * p1[0].size()));
}


void edge_highlight(boost::shared_ptr<Image> srcImagePtr,
					boost::shared_ptr<Image> dstImagePtr) {
	int x, y;
	int s = 0;
	int w[][3] = {
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0}
	};

	for (y = 0 ; y < srcImagePtr->height; y++) {
		for( x = 0 ; x < srcImagePtr->width; x++ ) {
			if (y == 0 || x == 0
				|| y == srcImagePtr->height - 1
				|| x == srcImagePtr->width - 1) {
				HALF_WINDOW(srcImagePtr, x, y, w);
			} else {
				WINDOW(srcImagePtr, x, y, w) ;
			}

			s = sobel(w);

			dstImagePtr->pixels[y][x]->r = s ;
			dstImagePtr->pixels[y][x]->g = s ;
			dstImagePtr->pixels[y][x]->b = s ;
		}
	}
}
