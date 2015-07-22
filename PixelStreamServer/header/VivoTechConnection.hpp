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


		unsigned int bytes_read = 0;
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
				raw[bytes_read+1] = '\0';

				bytes_read += rc;
				cout<<"bytes read: "<< bytes_read <<endl;

			}
			cout<<"term byte: "<< term_byte <<" bytes read: "<< bytes_read << endl;
			if(bytes_read > 1 && (term_byte - bytes_read) == 0)
			{
				/* Healthy Sleep 4s*/

				sleep(4);

				/*Reset variables*/

				rc = 0;
				term_byte = 0;
				bytes_read = 0;

				/*Not going back into the consumer thread until we request a new image*/

				m_IsRunning = false;

				cout<<"Request Image"<< endl;

				/* Clear the buffer for new data*/

				memset(raw, 0, sizeof(raw));

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


};







