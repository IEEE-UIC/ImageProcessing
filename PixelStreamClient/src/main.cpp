
//The headers
#include "SDL.h"
#include "SDL_image.h"
//#include "SDL_Texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "ImageViewer.hpp"
#include "SobelTrying.hpp"
#include "bmp2rgb.hpp"
#include <zmq.hpp>
#include <algorithm>
#include "MessageCallback.hpp"
#include "SpinLock.hpp"
#include "nano_msg.hpp"
#include "Event.hpp"
#include <boost/thread/thread.hpp>
using namespace std;
using namespace nano_message;


void dumpMessage(zmq::message_t& msg)
{
    std::for_each((char*)msg.data(), (char*)msg.data() + msg.size(),[](char c){ std::cout << c;});
    std::cout << std::endl;
}

int row_stride = 176 * 3;
unsigned char image_buff[144 * (176*3)];

int m_LineScanner = 0;
int m_StrSize = 0;
unsigned char* m_Native;

//SDL_Texture* mTexture;
SDL_Surface *screen;
SDL_Event event;

/* We only need to store one value technically because we are recieving BW pixels ( 1 byte ) */
RGBTRIPLE *bitmapImage;

nano_msg native_update;

spin_lock MemLock;

bool ImageReady = false;

zmq::message_t message;
zmq::context_t *context;
zmq::socket_t *subSocket;

boost::thread *m_ConsumerThread;
boost::thread *m_ApplicationThread;

MessageCallback *msg;




void RecieveData(){


	msg = new MessageCallback();

	context = new zmq::context_t(1);
	subSocket = new zmq::socket_t(*context, ZMQ_SUB);
	subSocket->connect("tcp://localhost:5563");
	subSocket->setsockopt( ZMQ_SUBSCRIBE, "", 0);



	while(1)
	{

		subSocket->recv(&message);
		msg->setFromZMQ(&message);

		nano_msg dat;
		string data;
		int size = msg->length;
		char* check_data = msg->data;

		dat.decode(check_data,size,0,2);
		dat.getString(2,data);

		m_LineScanner = dat.getInt(1);

		cout<<" Row Count: "<< m_LineScanner << endl;


		m_StrSize = strlen(data.c_str());

		m_Native = (unsigned char*)data.c_str();


		cout<<"index: "<< ((m_LineScanner-3)*528 ) << endl;

		MemLock.lock();

		memcpy(image_buff+((m_LineScanner-3)*528 ),m_Native,m_StrSize);


		cout<<"Mem transfer done... "<<endl;


		//    	for(unsigned int i=0; i < str_size; i++)
		//    		printf("0x%X,", native[i]);
		//    	printf("\n");

		if(m_LineScanner == 144)
		{
			cout<<"Begin RGB Transfer... "<<endl;
			bitmapImage = (RGBTRIPLE*)malloc(176*144 * sizeof(RGBTRIPLE));
			int location = 0;
			for(int i = 0; i < 176*144; i+=3)
			{
				bitmapImage[location].rgbtRed = image_buff[i++];
				bitmapImage[location].rgbtBlue = image_buff[i++];
				bitmapImage[location].rgbtGreen = image_buff[i++];
				location++;

			}
			ImageReady = true;
		}

		MemLock.unlock();



	}
}

void ScreenUpdate(){

	bool quit = false;


	while(quit == false)
	{
		while( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_QUIT )
				quit = true;
		}

		if(ImageReady)
		{
			cout<<"Updating Image..."<<endl;
			buf_display(screen,bitmapImage);
			SDL_Delay( 2000 );
			ImageReady = false;
			free(bitmapImage);
		}

	}

    	clean_up(screen);


}


int main( int argc, char* argv[] )
{



	screen = init();
	setDimensions(144,176);


	m_ConsumerThread = new boost::thread(RecieveData);
	m_ApplicationThread = new boost::thread(ScreenUpdate);

	while(1){}

}



/* For Future Reference	*/
//rc = zmq_connect(frontend, "ipc://cygdrive/c/cygwin/tmp/img_stream/0");
//frontend.connect("epgm://239.128.1.99:5561");
