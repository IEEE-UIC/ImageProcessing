/*
 * ConsumerThread.hpp
 *
 *  Created on: May 3, 2013
 *      Author: matthew.halko
 */

#ifndef CONSUMERTHREAD_HPP_
#define CONSUMERTHREAD_HPP_

#include <stdio.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <list>
#include <sstream>
#include <map>
#include <zmq.hpp>
#include <boost/thread/thread.hpp>
#include "Event.hpp"
#include "Delegate.hpp"
#include "MessageCallback.hpp"

using namespace std;
using namespace Architecture;


//! Listen to data for the given protocol and ip address/port on its own thread.
class ConsumerThread
{
	private:
			int m_HeaderLength;
			bool m_Continue;
			bool m_IsRunning;
			bool m_ThreadStopped;
			string m_Header;
			bool m_UseHeader;
			int m_HeaderSize;
			bool m_Close;
			int m_Affinity;

			int DEFAULT_QUEUE_SIZE;
			int DEFAULT_MAX_MESSAGE;
			char* default_buffer;

			boost::thread* m_WorkerThread;
			zmq::context_t* m_Context;
			zmq::socket_t* m_Socket;
			MessageCallback* m_CurrentCallback;


			std::string GetConnectionString(string protocol, string multicastIP, int port);
			void Run();
			void RunInfinibandConsumer();

		public:

			/**  @brief Event fired when a new message is received.
			 *   @param MessageCallback MessageCallback that was received, subscriber deallocates.
			 */
			Event<MessageCallback*> OnNewConsumerMessage;

			ConsumerThread();
			~ConsumerThread();

			/**  @brief Protocol to use for communication, tcp/epgm. */
			string strProtocol;
			/**  @brief IPAdress to subscribe to. */
			string strIPAdress;
			/**  @brief Local address to bind to. */
			string strBindIPAddress;
			/**  @brief Port for communication. */
			int iPort;
			/**  @brief True if multicast headers should be used, use for mutiple multicast ips on the same port.*/
			bool UseMulticastIPHeaders;
			/**  @brief True if the tcp connection should bind to the address.*/
			bool TCPBind;
			/**  @brief True if messages will be received using function RecieveMessage, disabled by default.*/
			bool EnableRecieveFuntion;

			bool EnableDirectRecieve;

			/**  @brief True if Infiniband RDMA will be used rather than ethernet.  */
			bool EnableInfinibandRDMA;

			/** @brief Receive a pointer the MessageCallback when the data is received.
			 *  @return Pointer to the MessageCallback, subscriber deallocates.
			 */
			MessageCallback* RecieveMessage();

			/**  @brief Start listening to data. */
			void Start();
			/**  @brief Stop listening to data. */
			void Stop();
			/**  @brief Make the current thread wait until consumer thread is done. */
			void Wait();
			/**  @brief Stop listening to data and close sockets. */
			void Close();
			/**  @brief Set a message header to subscribe to. */
			void SetHeader(string header, int allloc_size);

			inline void ReceiveMessageDirect(char** out_data, int &out_size)
			{
				zmq::message_t* msg = new zmq::message_t();
				m_Socket->recv(msg);

				*out_data = (char*)msg->data();
				out_size = msg->size();
			}

			inline void ReceiveMessageDirect(zmq::message_t* msg)
			{
				m_Socket->recv(msg);
			}

			inline char* getDefaultBuffer()
			{
				return default_buffer;
			}

//			inline int RecieveMulticast()
//			{
//				return m_UdpConsumer.recv(default_buffer,DEFAULT_MAX_MESSAGE);
//			}

			/**  @brief Set the core that this producer will run on.
			 *   @param core ID of the core to utilize.
			 */
			void SetAffinity(int core);

			void SetMaxMessageSize(int size);

			/** @brief Get the zmq context that is being used for this consumer.
			 *  @return Pointer to the zmq context.
			 */
			zmq::context_t* GetContext();
};
#endif /* CONSUMERTHREAD_HPP_ */
