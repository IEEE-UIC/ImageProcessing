/*
 * IEEE@UIC
 * VISION HEADER
 *
 */
#ifndef SOBLETRYING_HPP_
#define SOBLETRYING_HPP_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "bmp2rgb.hpp"

using namespace std;

int ROWS = 0;
int COLS = 0;
const int MAX_BUFF = ROWS * COLS;


//Global pointers

char *b1;		// img1 raw data buff1
char *b2;		// img2 raw data buff2
char **input1;	// grayed img1
char **input2;	// grayed img2
char **dfram1;	// delta fram2 gen1
char **dframe2;	// delta frame gen2



void setDimensions(BITMAPINFOHEADER *bitmapInfoHeader)
{

	ROWS = bitmapInfoHeader->biWidth;
	COLS = bitmapInfoHeader->biHeight;
}
void setDimensions(int r,int c)
{

	ROWS = r;
	COLS = c;
}


inline RGBTRIPLE** alloc2D(int row,int col)
{
	RGBTRIPLE **multi_dim;
	multi_dim = (RGBTRIPLE**)malloc(row * col * sizeof(RGBTRIPLE));
	for (int k = 0; k < row; k++)
		multi_dim[k] = (RGBTRIPLE*)malloc(col * sizeof(RGBTRIPLE));


	return multi_dim;
}

inline char* fillBuffer(string inFileName)
{

	char* tbuff;
	tbuff = (char*)malloc(sizeof(char) * MAX_BUFF);

	FILE *fp;
	fp = fopen(inFileName.c_str(), "r");
	if(fp != NULL)
	{
		while ((fgets(tbuff, sizeof(tbuff), fp)) != NULL)
		{
			//printf("%s", tbuff);
		}

		fclose(fp);
	}

	return tbuff;
}

inline char** readInput(char *buff)
{

	printf("%s",buff);
	char **RedB;
	char **output;


	int MatVal = 0;
	int cp = 0;
	int ch;

	//Inititialize Arrays

	RedB = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		RedB[k] = (char*)malloc(sizeof(char) * COLS);

	output = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		output[k] = (char*)malloc(sizeof(char) * COLS);


	//cout<<"allocated arrays"<<endl;

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			while (true)
			{
				ch = buff[cp];

				if ((48 <= ch) && (ch <= 57))
				{
					MatVal = (MatVal * 10) + ch;
					//cout<<"		MatVal -> "<< MatVal << endl;

				}
				else
				{
					RedB[i][j] = MatVal;
					//cout<< (char)MatVal << endl;
 					MatVal = 0;
					cp++;

					if (ch == 10 || ch == 20 || ch == 3 || ch == 32)
					{
						cp++;
					}

					break;
				}
				cp++;
			}
			output[i][j] = RedB[i][j];
		}

	}

	for(int i = 0; i < ROWS; i++)
		for(int j = 0; j < COLS; j++)
			if( output[i][j] != NULL)
				cout<<"i: "<< i <<" j: "<< j <<" output: "<< output[i][j] << endl;

	free(RedB);
	return output;
}

inline RGBTRIPLE** DeltaFrameGeneration(RGBTRIPLE** in1, RGBTRIPLE** in2)
{
/*
 * Delta Frame generation is two images squashed together after being grayed out.
 * Any pixel that stands out i.e moves will stand out after this transformation.
 * Two picures that are identical will be the difference of zero on all RGB
 *
 * We first need to figure out the threshold before continuing
 */
	RGBTRIPLE **seg;
	seg = alloc2D(ROWS,COLS);

	RGBTRIPLE **seg1;
	seg1 = alloc2D(ROWS,COLS);

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
			//double check this shit
			seg[x][y].rgbtRed = abs(in1[x][y].rgbtRed - in2[x][y].rgbtRed);
			seg[x][y].rgbtGreen = abs(in1[x][y].rgbtGreen - in2[x][y].rgbtGreen);
			seg[x][y].rgbtBlue = abs(in1[x][y].rgbtBlue - in2[x][y].rgbtBlue);
		}
	}

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
			if(seg[x][y].rgbtRed > 20 || seg[x][y].rgbtBlue > 20 || seg[x][y].rgbtGreen > 20)
			{
				seg1[x][y].rgbtRed = 255;
				seg1[x][y].rgbtGreen = 255;
				seg1[x][y].rgbtBlue = 255;
			}else{
				seg1[x][y].rgbtRed = 0;
				seg1[x][y].rgbtGreen = 0;
				seg1[x][y].rgbtBlue = 0;
			}
		}
	}


	//free(in1);
	free(seg);
	return seg1;
}

inline RGBTRIPLE **DeltaThresh(RGBTRIPLE **first_filter)
{
    int threshold    = rand() % 256;
    float sumObjPix  = 0;
    int numObjPix    = 0;
    float sumBacPix  = 0;
    int numBacPix    = 0;
    float meanObjPix = 0;
    float meanBacPix = 0;

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
			if (first_filter[x][y].rgbtRed > threshold)
            {
                sumObjPix = sumObjPix + first_filter[x][y].rgbtRed;
                numObjPix = numObjPix + 1;
            }
            else
            {
                sumBacPix = sumBacPix + first_filter[x][y].rgbtRed;
                numBacPix = numBacPix + 1;
            }
        }
    }

    meanObjPix = sumObjPix/numObjPix;
    meanBacPix = sumBacPix/numBacPix;

    threshold = (meanObjPix + meanBacPix)/2;

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
            if (first_filter[x][y].rgbtRed > threshold)
            {
            	first_filter[x][y].rgbtRed = 255;
            	first_filter[x][y].rgbtBlue = 255;
            	first_filter[x][y].rgbtGreen = 255;
            }
            else
            {
            	first_filter[x][y].rgbtRed = 0;
            	first_filter[x][y].rgbtGreen = 0;
            	first_filter[x][y].rgbtBlue = 0;
            }
        }
    }
	return first_filter;
}
inline RGBTRIPLE **EdgeImage(RGBTRIPLE **first_filter)
{

	RGBTRIPLE **edge_image;
	edge_image = alloc2D(ROWS,COLS);

    int threshold    = rand() % 256;
    float sumObjPix  = 0;
    int numObjPix    = 0;
    float sumBacPix  = 0;
    int numBacPix    = 0;
    float meanObjPix = 0;
    float meanBacPix = 0;

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
			if (first_filter[x][y].rgbtRed > threshold)
            {
                sumObjPix = sumObjPix + first_filter[x][y].rgbtRed;
                numObjPix = numObjPix + 1;
            }
            else
            {
                sumBacPix = sumBacPix + first_filter[x][y].rgbtRed;
                numBacPix = numBacPix + 1;
            }
        }
    }

    meanObjPix = sumObjPix/numObjPix;
    meanBacPix = sumBacPix/numBacPix;

    threshold = (meanObjPix + meanBacPix)/2;

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
            if (first_filter[x][y].rgbtRed > threshold)
            {
            	first_filter[x][y].rgbtRed = 255;
            	first_filter[x][y].rgbtBlue = 255;
            	first_filter[x][y].rgbtGreen = 255;
            }
            else
            {
            	first_filter[x][y].rgbtRed = 0;
            	first_filter[x][y].rgbtGreen = 0;
            	first_filter[x][y].rgbtBlue = 0;
            }
            edge_image[x][y].rgbtRed = 255;
            edge_image[x][y].rgbtGreen = 255;
            edge_image[x][y].rgbtBlue = 255;

        }
    }
	return edge_image;
}

inline RGBTRIPLE **EdgeDetection(RGBTRIPLE **deltaFrame)
{
	RGBTRIPLE **EdgeImage;
	EdgeImage = alloc2D(ROWS,COLS);

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
			if( x == 0 && y == 0)
			{
				if(deltaFrame[x][y+1].rgbtRed && deltaFrame[x+1][y+1].rgbtRed &&
						deltaFrame[x+1][y].rgbtRed == 255)
					EdgeImage[x][y].rgbtRed = 0;
					EdgeImage[x][y].rgbtGreen = 0;
					EdgeImage[x][y].rgbtBlue = 0;
			}
			else if(x == 0 && y == ROWS)
			{
				if(deltaFrame[x+1][y].rgbtRed && deltaFrame[x+1][y-1].rgbtRed &&
						deltaFrame[x][y-1].rgbtRed == 255)
					EdgeImage[x][y].rgbtRed = 0;
					EdgeImage[x][y].rgbtGreen = 0;
					EdgeImage[x][y].rgbtBlue = 0;
			}
			else if(x == ROWS && y == 0)
			{
				if(deltaFrame[x-1][y].rgbtRed && deltaFrame[x-1][y+1].rgbtRed &&
						deltaFrame[x][y+1].rgbtRed == 255)
					EdgeImage[x][y].rgbtRed = 0;
					EdgeImage[x][y].rgbtGreen = 0;
					EdgeImage[x][y].rgbtBlue = 0;
			}
			else if(x == ROWS && y == ROWS)
			{
				if(deltaFrame[x-1][y-1].rgbtRed && deltaFrame[x][y-1].rgbtRed &&
						deltaFrame[x-1][y].rgbtRed == 225)
					EdgeImage[x][y].rgbtRed = 0;
					EdgeImage[x][y].rgbtGreen = 0;
					EdgeImage[x][y].rgbtBlue = 0;
			}
			else if(x == 1)
			{
				if(deltaFrame[x][y-1].rgbtRed && deltaFrame[x+1][y-1].rgbtRed &&
						deltaFrame[x+1][y].rgbtRed && deltaFrame[x+1][y+1].rgbtRed &&
						deltaFrame[y][x+1].rgbtRed == 255)
					EdgeImage[x][y].rgbtRed = 0;
					EdgeImage[x][y].rgbtGreen = 0;
					EdgeImage[x][y].rgbtBlue = 0;
			}
			else if(x == ROWS)
			{
				if(deltaFrame[x-1][y-1].rgbtRed && deltaFrame[x+1][y-1].rgbtRed &&
						deltaFrame[x+1][y].rgbtRed && deltaFrame[x-1][y].rgbtRed &&
						deltaFrame[x][y-1].rgbtRed == 255)
					EdgeImage[x][y].rgbtRed = 0;
					EdgeImage[x][y].rgbtGreen = 0;
					EdgeImage[x][y].rgbtBlue = 0;
			}
			else if(y == 1)
			{
				if(deltaFrame[x][y-1].rgbtRed && deltaFrame[x-1][y-1].rgbtRed &&
						deltaFrame[x-1][y].rgbtRed && deltaFrame[x-1][y+1].rgbtRed &&
						deltaFrame[x][y+1].rgbtRed == 255)
					EdgeImage[x][y].rgbtRed = 0;
					EdgeImage[x][y].rgbtGreen = 0;
					EdgeImage[x][y].rgbtBlue = 0;
			}
			else{
				if(deltaFrame[x-1][y-1].rgbtRed && deltaFrame[x+1][y-1].rgbtRed &&
						deltaFrame[x+1][y].rgbtRed && deltaFrame[x+1][y+1].rgbtRed &&
						deltaFrame[x][y-1].rgbtRed && deltaFrame[x-1][y].rgbtRed == 255)
					EdgeImage[x][y].rgbtRed = 0;
					EdgeImage[x][y].rgbtGreen = 0;
					EdgeImage[x][y].rgbtBlue = 0;
			}
		}
	}
	return EdgeImage;
}



inline RGBTRIPLE** ToGrayScale(RGBTRIPLE **rgbmap)
{
	double L = 0;
	RGBTRIPLE **graymap;
	graymap = alloc2D(ROWS,COLS);

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{

			L = 0.2126 * rgbmap[x][y].rgbtRed + 0.7152 * rgbmap[x][y].rgbtGreen + 0.0722 * rgbmap[x][y].rgbtBlue;

			graymap[x][y].rgbtRed = (unsigned char)(L+0.5);
			graymap[x][y].rgbtGreen = (unsigned char)(L+0.5);
			graymap[x][y].rgbtBlue = (unsigned char)(L+0.5);
		}
	}

	return graymap;
}

inline RGBTRIPLE** ConvertTo2D(RGBTRIPLE *rgbmap)
{

	RGBTRIPLE **multi_dim;
	multi_dim = alloc2D(ROWS,COLS);

	int px = 0;
	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{

			//ofs = (j * ROWS) + i;

			multi_dim[x][y].rgbtRed = rgbmap[px].rgbtRed;
			multi_dim[x][y].rgbtBlue = rgbmap[px].rgbtBlue;
			multi_dim[x][y].rgbtGreen = rgbmap[px].rgbtGreen;
			px++;
		}
	}
	return multi_dim;
}

inline void sobel_printf(RGBTRIPLE ** rgbmap){

	/*
	 *  w pamieci MJ
	 *  karabin pow pw
	 *
	 */

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			printf("i-j[%d-%d] R:%d , B:%d , G:%d\n",i,j,rgbmap[i][j].rgbtRed,rgbmap[i][j].rgbtBlue,rgbmap[i][j].rgbtGreen);

		}
	}

}

inline float FindMedian(RGBTRIPLE **buff,int i,int j)
{

    float med;
    int tmp;

    RGBTRIPLE *values;
    values = (RGBTRIPLE*)malloc(sizeof(RGBTRIPLE)*8);

    values[0].rgbtRed = buff[i - 1][j - 1].rgbtRed;
    values[1].rgbtRed = buff[i][j - 1].rgbtRed;
    values[2].rgbtRed = buff[i + 1][j].rgbtRed;
    values[3].rgbtRed = buff[i - 1][j].rgbtRed;
    values[4].rgbtRed = buff[i + 1][j].rgbtRed;
    values[5].rgbtRed = buff[i - 1][j + 1].rgbtRed;
    values[6].rgbtRed = buff[i][j + 1].rgbtRed;
    values[7].rgbtRed = buff[i + 1][j + 1].rgbtRed;

	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8 - i; j++)
		{
			if(values[j].rgbtRed > values[j+1].rgbtRed)
			{
				tmp = values[j].rgbtRed;
				values[j].rgbtRed = values[j+1].rgbtRed;
				values[j+1].rgbtRed = tmp;
			}
		}
	}

	med = (values[3].rgbtRed + values[4].rgbtRed)/2.0;

	free(values);
    return med;

	}

inline RGBTRIPLE **MedianFilter(RGBTRIPLE **seg1)
{

	RGBTRIPLE **bbr;
	bbr = alloc2D(ROWS,COLS);

	RGBTRIPLE **firstfilter;
	firstfilter = alloc2D(ROWS,COLS);

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
			if (x > 0 &&
				x < (ROWS - 1) &&
				y > 0 &&
				y < (COLS - 1))
			{
				/*
				 * replace RGB with medians of Red i.e Red is the new Black&White :P
				 */
				bbr[x][y].rgbtRed = (uint8_t)FindMedian(seg1, x, y);
				bbr[x][y].rgbtGreen = (uint8_t)FindMedian(seg1, x, y);
				bbr[x][y].rgbtBlue = (uint8_t)FindMedian(seg1, x, y);


			}
			else
			{
				bbr[x][y].rgbtRed = seg1[x][y].rgbtRed;
				bbr[x][y].rgbtGreen = seg1[x][y].rgbtGreen;
				bbr[x][y].rgbtBlue = seg1[x][y].rgbtBlue;
			}

		}
	}
	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{

			firstfilter[x][y].rgbtRed = bbr[x][y].rgbtRed;
			firstfilter[x][y].rgbtGreen = bbr[x][y].rgbtGreen;
			firstfilter[x][y].rgbtBlue = bbr[x][y].rgbtBlue;


		}
	}

	free(bbr);
	free(seg1);
	return firstfilter;

}
inline RGBTRIPLE** EnhanceImage(RGBTRIPLE** in1, RGBTRIPLE** in2)
{
/*
 * Delta Frame generation is two images squashed together after being grayed out.
 * Any pixel that stands out i.e moves will stand out after this transformation.
 * Two picures that are identical will be the difference of zero on all RGB
 *
 * We first need to figure out the threshold before continuing
 */
	RGBTRIPLE **seg;
	seg = alloc2D(ROWS,COLS);

	RGBTRIPLE **store;
	store = alloc2D(ROWS,COLS);

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
			//double check this shit
			seg[x][y].rgbtRed = abs(in1[x][y].rgbtRed - in2[x][y].rgbtRed);
			seg[x][y].rgbtGreen = abs(in1[x][y].rgbtGreen - in2[x][y].rgbtGreen);
			seg[x][y].rgbtBlue = abs(in1[x][y].rgbtBlue - in2[x][y].rgbtBlue);
		}
	}

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
			if(seg[x][y].rgbtRed == 1 || seg[x][y].rgbtBlue == 1 || seg[x][y].rgbtGreen == 1)
			{
				store[x][y].rgbtRed = 102;
				store[x][y].rgbtGreen = 255;
				store[x][y].rgbtBlue = 0;


			}
		}
	}


	//free(in1);
	free(seg);
	return store;
}

#endif /* SOBLETRYING_HPP_ */
