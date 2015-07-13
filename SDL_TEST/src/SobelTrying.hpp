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

inline RGBTRIPLE** alloc2D(int row,int col)
{
	RGBTRIPLE **multi_dim;
	multi_dim = (RGBTRIPLE**)malloc(row * col * sizeof(RGBTRIPLE));
	for (int k = 0; k < row; k++)
		multi_dim[k] = (RGBTRIPLE*)malloc(col * sizeof(RGBTRIPLE));


	return multi_dim;
}
inline BW** alloc2Dgray(int row,int col)
{
	BW **multi_dim;
	multi_dim = (BW**)malloc(row * col * sizeof(BW));
	for (int k = 0; k < row; k++)
		multi_dim[k] = (BW*)malloc(col * sizeof(BW));


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

	/*
	 * swap ack later
	 */

	free(in1);
	free(in2);
	return seg;
}


inline RGBTRIPLE **Thresholding(RGBTRIPLE **filter)
{
	//Example of Gray Level Thresholding

//
//	RGBTRIPLE **ObjectPixels;
//	ObjectPixels = alloc2D(ROWS,COLS);
//
//	for (int i = 0; i < ROWS; i++)
//	{
//		for (int j = 0; j < COLS; j++)
//		{
//			if(filter[i][j] > 40)//mean or median value
//				filter[i][j] = 255;
//			else
//				filter[i][j] = 0;
//
//			EdgeImage[i][j] = 255;
//		}
//	}
//	return EdgeImage;
}

inline char **EdgeDetection(char **filter)
{
	char **EdgeImage;
	EdgeImage = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		EdgeImage[k] = (char*)malloc(sizeof(char) * COLS);

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			if( i == 0 && j == 0)
			{
				if(filter[j][i+1] && filter[j+1][i+1] &&
						filter[j+1][i] == 255)
					EdgeImage[j][i] = 0;

			}
			else if(i == 0 && j == ROWS)
			{
				if(filter[j+1][i] && filter[j+1][i-1] &&
						filter[j][i-1] == 255)
					EdgeImage[j][i] = 0;

			}
			else if(i == ROWS && j == 0)
			{
				if(filter[j-1][i] && filter[j-1][i+1] &&
						filter[j][i+1] == 255)
					EdgeImage[j][i] = 0;

			}
			else if(i == ROWS && j == ROWS)
			{
				if(filter[j-1][i-1] && filter[j][i-1] &&
						filter[j-1][i] == 225)
					EdgeImage[j][i] = 0;

			}
			else if(i == 1)
			{
				if(filter[j][j-1] && filter[j+1][i-1] &&
						filter[j+1][i] && filter[j+1][i+1] &&
						filter[j][i+1] == 255)
					EdgeImage[j][i] = 0;

			}
			else if(i == ROWS)
			{
				if(filter[j-1][i-1] && filter[j+1][i-1] &&
						filter[j+1][i] && filter[j-1][i] &&
						filter[j][i-1] == 255)
					EdgeImage[j][i] = 0;
			}
			else if(j == 1)
			{
				if(filter[j][i-1] && filter[j-1][i-1] &&
						filter[j-1][i] && filter[j-1][i+1] &&
						filter[j][i+1] == 255)
					EdgeImage[j][i] = 0;
			}
			else{
				if(filter[j-1][i-1] && filter[j+1][i-1] &&
						filter[j+1][i] && filter[j+1][i+1] &&
						filter[j][i-1] && filter[j-1][i] == 255)
					EdgeImage[j][i] = 0;

			}
		}
	}
	return EdgeImage;
}

inline char** EnchanceImage(char **in1,char **in2)
{
	char **seg;
	seg = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		seg[k] = (char*)malloc(sizeof(char) * COLS);

	char **store;
	store = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		store[k] = (char*)malloc(sizeof(char) * COLS);

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			seg[i][j] = in1[i][j] - in2[i][j];

			//Not sure if this is correct check document
			if(seg[i][j] == 1)
				store[i][j] = seg[i][j];

		}
	}

	free(seg);
	return store;


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

inline RGBTRIPLE **MedianFilter(RGBTRIPLE **delFrame)
{

	RGBTRIPLE **temp;
	temp = alloc2D(ROWS,COLS);

	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{
			if (x > 0 &&
				x < (ROWS - 1) &&
				y > 0 &&
				y < (COLS - 1))
			{
				temp[x][y].rgbtRed = (uint8_t)FindMedian(delFrame, x, y);
				temp[x][y].rgbtGreen = (uint8_t)FindMedian(delFrame, x, y);
				temp[x][y].rgbtBlue = (uint8_t)FindMedian(delFrame, x, y);


			}
			else
			{
					temp[x][y].rgbtRed = delFrame[x][y].rgbtRed;
					temp[x][y].rgbtGreen = delFrame[x][y].rgbtGreen;
					temp[x][y].rgbtBlue = delFrame[x][y].rgbtBlue;
			}

		}
	}
	for (int y = COLS-1; y >= 0;--y)
	{
		for (int x = 0; x < ROWS; ++x)
		{

            delFrame[x][y].rgbtRed = temp[x][y].rgbtRed;
            delFrame[x][y].rgbtGreen = temp[x][y].rgbtGreen;
            delFrame[x][y].rgbtBlue = temp[x][y].rgbtBlue;


		}
	}

	free(temp);
	return delFrame;

}



#endif /* SOBLETRYING_HPP_ */
