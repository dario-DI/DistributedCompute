/// \file jobServer.h Copyright (C).
/// \brief brief-description
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/20 17:34
#pragma once

#ifndef _DCOMPUTE_JOBSERVER_H_
#define _DCOMPUTE_JOBSERVER_H_

#include <vector>

#include <cex/DeltaReflection.h>
#include <DCompute/config>
#include <DCompute/util.h>

namespace DCompute 
{
	class IReplyServer : public cex::Interface
	{
	public:

		virtual void create()=0;

		virtual void destory()=0;


		virtual void start()=0;

		virtual void join()=0;

		
	};

	class IJoberServer : public cex::Interface
	{
	public:
		virtual void create()=0;


		virtual void start()=0;

		virtual void join()=0;

		
	};

	namespace Contract
	{
		class WorkerInfo;
	}

	namespace detail
	{
		template<typename Base>
		class TRouterThread : public TThreadProxy<Base>
		{
		public:
			TRouterThread() : _context(0), _frontend(0), _backend(0) {}

			virtual ~TRouterThread()=0 { destory();}

			virtual void join()
			{
				_done = true;

				destory();

				__super::join();
			}

			virtual void destory()
			{
				if ( _context!=0 )
				{
					zmq_close(_backend);
					zmq_close(_frontend);
					zmq_term(_context);
				}

				_backend=0;
				_frontend=0;
				_context=0;
			}

		protected:
			void* _context;
			void* _frontend;
			void* _backend;
		};

		class CWokerCounter
		{
		public:
			~CWokerCounter(){}

			std::vector<std::string> workers;

		private:
			CWokerCounter() {}

			static CWokerCounter* Instance();

			friend class Contract::WorkerInfo;
		};
	}

}

#endif