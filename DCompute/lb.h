/// \file lb.h Copyright (C).
/// \brief Loading balance algorithm of router
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/31 17:21
#pragma once

#ifndef _DCOMPUTE_LBROUTER_H_
#define _DCOMPUTE_LBROUTER_H_

#include <DCompute/joberServer.h>

namespace DCompute 
{
	class ILBRouter : public cex::Interface
	{
	public:
		virtual void create()=0;

		virtual void destory()=0;


		virtual void start()=0;

		virtual void join()=0;

		
	};
}

#endif