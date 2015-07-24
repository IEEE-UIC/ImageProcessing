/*
 * VivoTechConnection.hpp
 *
 *  Created on: Jul 18, 2015
 *      Author: Peter Nowak
 */


#include <stdio.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <list>
#include <sstream>
#include <map>
#include "ConsumerThread.hpp"
#include "DataConsumer.hpp"
#include "MessageCallback.hpp"
#include "SpinLock.hpp"
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "assert.h"
#include <jpeglib.h>


using namespace std;
using namespace Architecture;
using namespace boost::posix_time;

class VivoTechConnection
{

private:
	bool m_IsRunning;

	Event<uint8_t*> OnNewData;
	boost::thread *m_ApplicationConsumerThread;

	spin_lock m_ParameterLock;

public:

	/*
	 * Public Variables Used throughout the program
	 *
	 */

	void *ctx_t;
	void *client;

	int rc;
	uint8_t id [256];
	size_t id_size = 256;

	/* Data structure to hold the ZMQ_STREAM received data */
	uint8_t raw [60000];
	size_t raw_size = 60000;

	/*stores the header of the packet*/
	unsigned char packet_header[148];
	unsigned char packet_payload[60000];

	/*race condition if we decide to use multiple threads (need to lock)*/
	unsigned int bytes_read = 0;



	// Variables for the decompressor itself
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	int row_stride, width, height, pixel_size;

	VivoTechConnection():LPrintData(this){}

	LISTENER(VivoTechConnection, PrintData, uint8_t*);


	~VivoTechConnection(){}


	inline void StreamInit(){


		OnNewData += &LPrintData;


		/*Initialize and Connect to server*/
		ZmqConnect();
		/*Send Packet to server*/
		ZmqSend();

		m_ApplicationConsumerThread = new boost::thread(&VivoTechConnection::StartConsumerThread, this);

	}


	inline void StartConsumerThread()
	{


		int rc = 0;
		int term_byte;


		while (m_IsRunning)
		{


			/*Get the Identity frame of the zmq message*/
			id_size = zmq_recv (client, id, 256, 0);

			/* The first message we receive from the socket should be the zmq id
			 * of the connected client.  Print it out to verify*/

			int i;
			for (i=0; i< id_size; i++) {
				printf ("%02X", (unsigned char) id [i]);
			}
			printf ("\n");


			rc = zmq_recv (client, raw + bytes_read, raw_size, 0);

			if(rc != -1)
			{

				term_byte = bytes_read;
				bytes_read += rc;
				cout<<"bytes read: "<< bytes_read <<endl;



			}
			cout<<"term byte: "<< term_byte <<" bytes read: "<< bytes_read << endl;
			if(bytes_read > 1 && (term_byte - bytes_read) == 0)
			{

				/* Add null terminator for printing */
				raw[bytes_read+1] = '\0';

				/* Store the header in the buffer
				 * The first 148 bits are always the header of the packet.*/

				int i;
				for (i=0; i< 148; i++) {
					//printf ("%c", (unsigned char) raw [i]);
					packet_header[i] = (unsigned char)raw[i];
				}
				printf ("\n");

				/*Write to file*/
				JpegToPixels(raw);

				/* Healthy Sleep 4s*/

				sleep(1);

				/*Reset variables*/

				rc = 0;
				term_byte = 0;
				bytes_read = 0;

				/*Not going back into the consumer thread until we request a new image*/

				m_IsRunning = false;

				cout<<"Request Image"<< endl;

				/* Clear the buffer for new data*/

				memset(raw, 0, sizeof(raw));
				memset(packet_header,0,sizeof(packet_header));

				/*New image, the thread should flow naturally*/

				RequestImage();
			}
		}
	}
	inline void RequestImage()
	{


		while(!m_IsRunning)
		{

			//  First frame is server identity
			rc = zmq_send (client, id, id_size, ZMQ_SNDMORE);

			char *packet = "GET /cgi-bin/viewer/video.jpg HTTP/1.1\r\n\r\n";

			//  Second frame is HTTP GET request
			rc = zmq_send (client, packet,strlen(packet), 0);

			switch(errno){
			case EAGAIN:
				/*Pipe has been closed reset sockets and send */
				cout<<"Close and Reconnect socket"<<endl;
				ZmqClose();
				ZmqConnect();
				ZmqSend();
				break;
			default:
				cout<<"Unknown error: "<< errno << endl;

			}
			/* if by chance the pipe does not close continue back to ConsumerThread*/
			m_IsRunning = true;
		}

	}

	inline void ZmqSend()
	{
		int rc;
		//  First frame is server identity
		rc = zmq_send (client, id, id_size, ZMQ_SNDMORE);
		assert (rc == (int) id_size);

		char *packet = "GET /cgi-bin/viewer/video.jpg HTTP/1.1\r\n\r\n";

		//  Second frame is HTTP GET request
		rc = zmq_send (client, packet,strlen(packet), 0);

	}
	inline void ZmqConnect()
	{

		m_IsRunning = true;

		int rc;

		ctx_t = zmq_ctx_new();
		assert(ctx_t);

		client = zmq_socket (ctx_t, ZMQ_STREAM);
		assert(client);

		rc = zmq_connect (client, "tcp://192.168.1.2:80");
		assert(rc == 0);


		rc = zmq_getsockopt (client, ZMQ_IDENTITY, id, &id_size);
		assert(rc == 0);


	}

	inline void ZmqClose()
	{
		m_IsRunning = false;
		zmq_close (client);
		zmq_ctx_destroy(ctx_t);

	}


	inline void PrintData(uint8_t *obj)
	{

		for(int i = 0; sizeof(obj);i++)
		{

			std::string s( obj, obj+i );
			cout<<  s <<endl;
		}


	}
	inline void WriteToFile(unsigned char* buffer)
	{

		unsigned int byte_count = bytes_read - 148;
		unsigned int rc;

		/*maybe do a memcpy to packet_payload eventually*/
		//memcpy(packet_payload,buffer+148,byte_count);

		FILE *fp;
		fp=fopen("/cygdrive/c/Users/Peter/git/ImageProcessing/PixelStreamServer/test_me.jpg", "wb");
		rc = fwrite(buffer+148, sizeof(unsigned char), byte_count, fp);
		if(rc > 0)
			cout<<"Number of elements bytes written: "<< rc << endl;

		fclose (fp);
	}
	inline void JpegToPixels(unsigned char *buffer)
	{

		unsigned int byte_count = bytes_read - 148;
		unsigned int rc = 0;

		unsigned char *raw_image;
		unsigned long location = 0;
		JSAMPROW row_pointer[1];

		cinfo.err = jpeg_std_error(&jerr);


		jpeg_create_decompress(&cinfo);
		jpeg_mem_src(&cinfo, buffer+148, bytes_read);

		rc = jpeg_read_header(&cinfo, TRUE);

		if (rc != 1) {
			cout<<"File does not seem to be a normal JPEG "<< endl;
		}


		jpeg_start_decompress(&cinfo);

		width = cinfo.output_width;
		height = cinfo.output_height;
		pixel_size = cinfo.output_components;

		// The row_stride is the total number of bytes it takes to store an entire scanline (row).
		row_stride = width * pixel_size;

		 raw_image = (unsigned char*)malloc( width*height*pixel_size );
		 row_pointer[0] = (unsigned char *)malloc( row_stride );

		 while( cinfo.output_scanline < cinfo.image_height )
		 {
		  jpeg_read_scanlines( &cinfo, row_pointer, 1 );

		  for( int i = 0; i < row_stride; i++)
			  raw_image[location++] = row_pointer[0][i];

		 }

		 for(int i=0; i < bytes_read; i++)
			 printf("0x%3X ", (unsigned int)raw_image[i]);

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		free( row_pointer[0] );
		free(raw_image);


		//return raw_image;


	}


};







