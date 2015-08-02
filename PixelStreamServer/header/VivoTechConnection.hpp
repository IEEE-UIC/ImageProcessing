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
#include "nano_msg.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include "assert.h"
#include <jpeglib.h>


using namespace std;
using namespace Architecture;
using namespace boost::posix_time;
using namespace nano_message;

class VivoTechConnection
{

private:
	bool m_IsRunning;

	Event<uint8_t*> OnNewData;
	Event<unsigned char*> OnNewPixels;
	Event<MessageCallback *> OnEncode;

	boost::thread *m_ApplicationConsumerThread;

	spin_lock m_ParameterLock;

public:

	/*
	 * Public Variables Used throughout the program
	 *
	 */

	void *ctx_t;
	void *client;
	void *ipc_socket;
	void *ipc_ctx;

	zmq::context_t* m_IpcContext;
	zmq::socket_t* m_IpcSocket;

	zmq::context_t* m_PubContext;
	zmq::socket_t* m_PubSocket;


	char *data_buffer;

	MessageCallback *msg;
    nano_msg native_update;


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

	VivoTechConnection():LPrintData(this),LIpcSend(this),LSendNanoMessage(this){

		msg = new MessageCallback();

		//data_buffer = (char*)malloc(4096);

	}

	LISTENER(VivoTechConnection, PrintData, uint8_t*);
	LISTENER(VivoTechConnection, IpcSend, unsigned char*);
	LISTENER(VivoTechConnection, SendNanoMessage, MessageCallback*);



	~VivoTechConnection(){

		delete m_PubSocket;
		delete m_PubContext;
		delete msg;
	}


	inline void StreamInit(){


		OnNewData += &LPrintData;
		OnNewPixels += &LIpcSend;
		OnEncode += &LSendNanoMessage;

		/* Open IPC pipe */
		//IpcInit();
		/*Open TCP pipe*/
		PublishInit();
		/*Initialize and Connect to server*/
		ZmqConnect();
		/*Send Packet to server*/
		ZmqSend();

		m_ApplicationConsumerThread = new boost::thread(&VivoTechConnection::StartConsumerThread, this);

	}
	inline void PublishInit()
	{

		m_PubContext = new zmq::context_t(1);
		m_PubSocket = new zmq::socket_t(*m_PubContext, ZMQ_PUB);
		m_PubSocket->bind("tcp://127.0.0.1:5563");



	}
	inline void IpcInit()
	{

		/* Assign the pathname */

		int rc;


		ipc_ctx = zmq_ctx_new();
		assert(ipc_ctx);

		ipc_socket = zmq_socket (ipc_ctx, ZMQ_PUB);
		assert(ipc_socket);

		rc = zmq_bind(ipc_socket, "ipc:///cygdrive/c/cygwin/tmp/img_stream/0");
		assert (rc == 0);


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
				//WriteToFile(raw);

				/* Transform jpeg into RGB */
				JpegToPixels(raw);

				/* Healthy Sleep 1s*/

				usleep(1000);

				/*Reset variables*/

				rc = 0;
				term_byte = 0;
				bytes_read = 0;

				/*Not going back into the consumer thread until we request a new image*/

				m_IsRunning = false;

				cout<<"Request Image"<< endl;

				/* Clear the buffer for new data*/

				memset(raw, 0, 60000);
				memset(packet_header,0,148);

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

	inline void IpcSend(unsigned char *buffer)
	{


		 std::string buff(buffer, buffer + sizeof(buffer));

		zmq::message_t msg((void*)buff.c_str(),buff.length(),NULL);

		m_PubSocket->send(msg);

		cout<<"Published Data Size: "<< buff.length() << endl;
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

	inline void SendNanoMessage(MessageCallback *msg)
	{
		zmq::message_t outbound(msg->length);
		memcpy(outbound.data(),msg->data,msg->length);
		m_PubSocket->send(outbound,0);
		cout<<"Published Data Size: "<< msg->length << endl;

	}

	inline void JpegToPixels(unsigned char *buffer)
	{

		unsigned int rc = 0;
		unsigned char *raw_image;
		unsigned char *row_pointer;

		int mem_counter = 0;

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

		/* 3 rows of row_stride size */
		raw_image = (unsigned char*)malloc( 3 * row_stride);

		/* Make space for the entire image */
		row_pointer = (unsigned char *)malloc( height * row_stride );

		unsigned char *image_pntr = &raw_image[0];


		/*Buffer to store the nano_msg data*/
		data_buffer = (char *)malloc(3*row_stride + sizeof(int));
		/*set the buffer for the nano_msg*/
		native_update.set_buffer(data_buffer);

		while( cinfo.output_scanline < cinfo.image_height )
		{
		    unsigned char *rowp[1];
		    rowp[0] = row_pointer + row_stride * cinfo.output_scanline;

			jpeg_read_scanlines( &cinfo, rowp, 1 );

			for(int i = 0; i < row_stride; i++) {
				raw_image[i] = rowp[0][i];
			}

			cout<<"scan line index: "<< cinfo.output_scanline << endl;
			if(cinfo.output_scanline % 3 == 0)
			{

				cout<< &raw_image << endl;
				string pay_me = reinterpret_cast<char *>(raw_image);

				cout<<"Nano_Msg buffer set "<< endl;

				/* Grab the RawImage[3][3] then encode it to binary
				 * Then take the Binary convert to MessageCallback
				 * Take the MessageCallback and convert it into a zmq::message_t */


				native_update.reset();
				native_update.setColumnCount(2);
				native_update.setInt(1,cinfo.output_scanline);
				native_update.setString(2,pay_me);

				msg->data = native_update.encode();
				msg->length = native_update.getSize();

				cout<<"Set Nano Msg and converted to MessageCallback"<<endl;
				/******************************************************************/

				OnEncode(msg);


				//memset(raw_image,0,3*row_stride);
				memset(data_buffer,0,3 * row_stride + sizeof(int));


				raw_image = image_pntr;
			}
			else
				raw_image+=row_stride;


//			if(cinfo.output_scanline == height)
//			{
				//raw_image+=3*row_stride;
				//memset(raw_image,0,3*row_stride);

//				cout<<"scanline == height, "<< cinfo.output_scanline <<" === " << height <<endl;
//				break;
//			}


//			}else{
//
//				raw_image+=row_stride;
//				mem_counter += row_stride;
//				cout<<"mem_counter: "<< mem_counter <<endl;
//
//			}

		}//end while

		  jpeg_finish_decompress(&cinfo);
		  jpeg_destroy_decompress(&cinfo);

		  free(row_pointer);
		  free(raw_image);
		  free(data_buffer);

	}

	void int64ToChar(char a[], int64_t n) {
	  memcpy(a, &n, 8);
	}

	int64_t charTo64bitNum(char a[]) {
	  int64_t n = 0;
	  memcpy(&n, a, 8);
	  return n;
	}

};







