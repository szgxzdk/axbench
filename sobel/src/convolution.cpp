/*
 * convolution.cpp
 * 
 * Created on: Sep 9, 2013
 * 			Author: Amir Yazdanbakhsh <a.yazdanbakhsh@gatech.edu>
 */

#include "convolution.hpp"
#include <cmath>

static int kx[][3] =
		{
			{ -1, -2, -1 },
			{  0,  0,  0 },
			{  1,  2,  1 }
		} ;

static int ky[][3] =
		{
			{ -1, 0, 1 },
			{ -2, 0, 2 },
			{ -1, 0, 1 }
		} ;

int convolve(int w[][3], int k[][3])
{
	int r = 0;
	for( int j = 0 ; j < 3 ; j++ )
		for ( int i = 0 ; i < 3 ; i++ )
			r += w[i][j] * k[j][i] ;

	return r ;
}

int sobel(int w[][3])
{
	int sx, sy;
	double s;

	sx = convolve(w, ky) ;
	sy = convolve(w, kx) ;

	s = sqrt(sx * sx + sy * sy) ;
	//if (s >= (256 / sqrt(256 * 256 + 256 * 256)))
	//	s = 255 / sqrt(256 * 256 + 256 * 256);
	if (s >= 256)
		s = 255;

	return s ;
}
