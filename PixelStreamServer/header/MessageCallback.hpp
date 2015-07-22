/*
 * DataMessageCallback.hpp
 *
 *  Created on: May 3, 2013
 *      Author: matthew.halko
 */

#ifndef MESSAGECALLBACK_HPP_
#define MESSAGECALLBACK_HPP_

#include <zmq.hpp>

//! Message received from transport layer.
class MessageCallback
{
	private:
		bool m_FromZMQ;
		zmq::message_t* message;

	public:
		MessageCallback()
		{
			m_FromZMQ = false;
		}
		~MessageCallback()
		{
			if(m_FromZMQ)
				delete message;
			else
				free(data);
		}

		/**  @brief Serialized data */
		char* data;
		/**  @brief Length of the data */
    	int length;

    	inline void setFromZMQ(zmq::message_t* msg)
    	{
    		message = msg;
    		m_FromZMQ = true;
    		data = (char*)msg->data();
    		length = msg->size();
    	}

};
#endif /* MESSAGECALLBACK_HPP_ */
