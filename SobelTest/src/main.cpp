#include <iostream>
#include <fstream>
#include <malloc.h>
#include "SobelTrying.hpp"
#include "bmp2rgb.hpp"

using namespace std;


int main(int argc, char *argv[])
{


	BITMAPINFOHEADER bitmapInfoHeader1;
	BITMAPINFOHEADER bitmapInfoHeader2;
	RGBTRIPLE *background_buf;
	RGBTRIPLE *object_buf;

	RGBTRIPLE **background_map;
	RGBTRIPLE **object_map;


	background_buf = LoadBitmapFile(argv[1],&bitmapInfoHeader1);
	object_buf 	= LoadBitmapFile(argv[2],&bitmapInfoHeader2);

//	PrintHeaderInfo(&bitmapInfoHeader1);
//	cout<<"__________________________"<<endl;
//	PrintHeaderInfo(&bitmapInfoHeader2);

	background_map = ConvertTo2D(background_buf);
	background_map = ToGrayScale(background_map);

	object_map = ConvertTo2D(object_buf);
	object_map = ToGrayScale(object_map);


	RGBTRIPLE **delta_frame;
	//need two gray scale images 1)background 2)object
	//delta_frame = DeltaFrameGeneration(pixelmap2,pixelmap2);

	//sobel_printf(pixelmap2);


	free(background_buf);
	free(background_map);
	free(object_buf);
	free(object_map);
	return 0;
}
