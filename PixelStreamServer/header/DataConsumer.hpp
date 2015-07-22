/*
 * MulticastConsumer.hpp
 *
 *  Created on: May 3, 2013
 *      Author: matthew.halko
 */

#ifndef DATACONSUMER_HPP_
#define DATACONSUMER_HPP_

#include <stdio.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <list>
#include <sstream>
#include <map>
#include <zmq.hpp>
#include <boost/thread.hpp>

#include "ConsumerThread.hpp"
#include "Event.hpp"
#include "Delegate.hpp"
#include "MessageCallback.hpp"

using namespace std;


//! Bring together multiple consumer threads into a single pipe.
class DataConsumer
{
	private:
			bool m_Continue;
			bool m_IsRunning;
			int m_Affinity;
			map<string, int> m_SubscribeIPs;
			//Key:ip value:protocol
			map<string, string> m_Protocols;
			map<string, ConsumerThread*> m_ConsumerThreads;

		public:
			/**  @brief Event fired when any one of the IP consumers receives a message.
			 *   @param MessageCallback MessageCallback that was received, subscriber deallocates.
			 */
			Event<MessageCallback*> OnNewDataMessage;

			/**  @brief Local address to bind to.*/
			string strBindIPAddress;
			/**  @brief Protocol to use for communication, tcp/epgm/inproc. */
			string strProtocol;
			/**  @brief True if multicast headers should be used, use for mutiple multicast ips on the same port.*/
			bool UseMulticastIPHeaders;
			/**  @brief True if the tcp connection should bind to the address.*/
			bool TCPBind;
			bool EnableInfinibandRDMA;

			DataConsumer();
			~DataConsumer();

			/** @brief Subscribe to data on the given ip address.
			 *  @param strIP IPAdress to subscribe to.
			 *  @param iPort Port to subscribe to.
			 */
			void AddIP(std::string strIP, int iPort);
			/** @brief Subscribe to data on the given ip adress and protocol.
			 *  @param strProtocol Protocol to use
			 *  @param strIP IPAdress to subscribe to
			 *  @param iPort Port to subscribe to
			 */
			void AddIP(string strProtocol,std::string strIP, int iPort);

			void HandleIncomingMessage(MessageCallback* newMessage);

			LISTENER(DataConsumer, HandleIncomingMessage, MessageCallback*);

			/**  @brief Start all consumption threads. */
			void Start();
			/**  @brief Stop all consumption threads. */
			void Stop();
			/**  @brief Stop all consumption threads and close sockets. */
			void Close();

			/**  @brief Set the core that this producer will run on.
			 *   @param core ID of the core to utilize.
			 */
			void SetAffinity(int core);

			/** @brief Get the zmq context that is being used for a consumer thread.
			 * 	@param IPAddress of the consumer
			 *  @return Pointer to the zmq context
			 */
			zmq::context_t* GetContext(string strIP);
};
#endif /* DATACONSUMER_HPP_ */
