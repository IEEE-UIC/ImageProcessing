/*
 * spin_lock.hpp
 *
 *  Created on: Jun 18, 2014
 *      Author: mhalko
 */

#ifndef SPIN_LOCK_HPP_
#define SPIN_LOCK_HPP_


#include <atomic>

class spin_lock
{
	public:
		inline void lock()
		{
			while(lck.test_and_set(std::memory_order_acquire))
			{

			}
		}

		inline void unlock()
		{
			lck.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag lck = ATOMIC_FLAG_INIT;
};

#else

#include <boost\thread.hpp>

class spin_lock
{
	public:
		inline void lock()
		{
			m_Lock.lock();
		}

		inline void unlock()
		{
			m_Lock.unlock();
		}

	private:
		boost::mutex m_Lock;
};


#endif /* SPIN_LOCK_HPP_ */
