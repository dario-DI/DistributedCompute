/// \file worker.h Copyright (C).
/// \brief brief-description
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/20 17:34
#pragma once

#ifndef _DCOMPUTE_WORKER_H_
#define _DCOMPUTE_WORKER_H_

#include <cex/DeltaReflection.h>
#include <DCompute/config>
#include <DCompute/util.h>

namespace DCompute 
{
	class IWorker : public cex::Interface
	{
	public:

		virtual bool create()=0;

		virtual void destory()=0;


		virtual void start()=0;
		
		virtual void join()=0;

		


		virtual int getID()=0;

		virtual void setID(int id)=0;
	};

}

#endif