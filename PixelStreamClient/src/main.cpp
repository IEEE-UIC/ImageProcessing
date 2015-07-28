
//The headers
#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "ImageViewer.hpp"
#include "SobelTrying.hpp"
#include "bmp2rgb.hpp"
#include <zmq.hpp>
#include <algorithm>
using namespace std;



void dumpMessage(zmq::message_t& msg)
{
    std::for_each((char*)msg.data(), (char*)msg.data() + msg.size(),[](char c){ std::cout << c;});
    std::cout << std::endl;
}

int main( int argc, char* argv[] )
{


	/* We only need to store one value technically because we are recieving BW pixels ( 1 byte ) */
	RGBTRIPLE **bitmapImage;
	char *subBuffer;

	unsigned int bytes_read = 0;


	zmq::message_t message;
    zmq::context_t context(1);
    zmq::socket_t subSocket(context, ZMQ_SUB);

    subSocket.connect("tcp://localhost:5563");
    subSocket.setsockopt( ZMQ_SUBSCRIBE, "", 0);

	//rc = zmq_connect(frontend, "ipc://cygdrive/c/cygwin/tmp/img_stream/0");
    //frontend.connect("epgm://239.128.1.99:5561");

    while (1) {


    	//  Process all parts of the message
    	subSocket.recv(&message);
    	subBuffer = (char *)malloc(message.size());
    	memcpy(subBuffer,message.data(),message.size());

    	bytes_read += message.size();
    	/* Idea , incremental update the image ?? that would be sweet. */

    	bitmapImage = (RGBTRIPLE*)malloc(message.size() * sizeof(RGBTRIPLE));

    	for(unsigned int i=0; i < message.size(); i++)
    		printf("0x%2X ", (unsigned int)subBuffer[i]);
    	printf("\n");


    	free(subBuffer);
    	free(bitmapImage);

    }

