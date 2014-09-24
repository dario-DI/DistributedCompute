/// \file client.h Copyright (C).
/// \brief brief-description
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/20 17:33
#pragma once

#ifndef _DCOMPUTE_CLIENT_H_
#define _DCOMPUTE_CLIENT_H_

//#include <boost/function.hpp>

#include <cex/DeltaReflection.h>
#include <DCompute/config>
#include <DCompute/util.h>

namespace DCompute 
{
	class IClient : public cex::Interface
	{
	public:

		virtual void setEndpoint(const char* endpoint)=0;


		virtual bool create()=0;

		virtual void destory()=0;


		virtual void setTaskFile(const char* filename)=0;

		virtual void setResultFile(const char* filename)=0;


		virtual bool sendTask()=0;

		virtual bool recieveResult(int doNotWait=0)=0;

	};

	/////////////////////////////////////////////////////
	//
	class IClientThread : public cex::Interface
	{
	public:

		virtual bool create()=0;

		virtual void destory()=0;


		virtual void start()=0;

		virtual void join()=0;	


		virtual void setTaskFile(const char* filename)=0;

		virtual const char* getResultFile() const=0;
	};

}

#endif