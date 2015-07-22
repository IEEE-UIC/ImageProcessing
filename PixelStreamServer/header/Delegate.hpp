/*
 * Delegate.hpp
 *
 *  Created on: Jul 18, 2015
 *      Author: Peter
 */

#ifndef DELEGATE_HPP_
#define DELEGATE_HPP_


#include <vector>
#include<iostream>
#include <stdio.h>

using namespace std;

namespace Architecture
{
	#define LISTENER(thisType, handler, type)\
		class __L##handler##__ : public Delegate< type >\
		{\
			public:\
				__L##handler##__ ( thisType * obj )\
				: _obj(obj) {}\
				inline void operator()( type param )\
				{\
					_obj-> handler (param);\
				}\
				thisType * _obj;\
		};\
		__L##handler##__ L##handler;

	//! C# Style delegate
	template <typename T>
	class Delegate
	{
		public:
			virtual void operator()(T param) = 0;
	};
}





#endif /* DELEGATE_HPP_ */
