/// \file lru.h Copyright (C).
/// \brief least recently used algorithm of router
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/31 17:21
#pragma once

#ifndef _DCOMPUTE_LRUROUTER_H_
#define _DCOMPUTE_LRUROUTER_H_


#include <DCompute/joberServer.h>

namespace DCompute 
{
	class ILRURouter : public cex::Interface
	{
	public:
		virtual void create()=0;

		virtual void destory()=0;


		virtual void start()=0;

		virtual void join()=0;

		
	};

	class DCOMPUTE_EXPORT_CLASS LRURouterMethod
	{
	public:
		static std::shared_ptr<cex::IString> ReciveAddress(void* socket);

		static int SendAddress(void* socket, const char* addr, int size);

		static int SendReady(void* socket);

	};
}

#endif