/// \file task.h Copyright (C).
/// \brief brief-description
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/23 11:20
#pragma once

#ifndef _DCOMPUTE_TASK_H_
#define _DCOMPUTE_TASK_H_

#include <vector>

//#include <boost/bind.hpp>
#include <DCompute/config>
#include <DCompute/typeWrappers.h>
#include <DCompute/client.h>

namespace DCompute 
{
	template<typename T>
	class CTaskProxy
	{
	public:
		typedef CTaskProxy<T> ThisClass;

		CTaskProxy() : _task(0),_jobDone(false)
		{
			_client = cex::DeltaCreateRef<IClient>();
		}

		~CTaskProxy()
		{
			Util::DeleteTempFile( _strTaskFileName.data() );
			Util::DeleteTempFile( _strResultFileName.data() );
		}

		void setEndpoint(const char* endpoint) { _client->setEndpoint(endpoint); }

		void setTask(T* task)
		{
			_task = task;

			_strTaskFileName.assign(Util::CreateUniqueTempFile()->data());
			detail::MakeObject2File(*_task, _strTaskFileName.data());

			_strResultFileName.assign(Util::CreateUniqueTempFile()->data());

			_client->setTaskFile(_strTaskFileName.data());
			_client->setResultFile(_strResultFileName.data());
		}

		void sendTask(T* task)
		{
			_client->create();
			_client->sendTask();
		}

		bool tryRecieveResult()
		{
			if(_jobDone) return true;

			_jobDone = _client->recieveResult(1);
			if(!_jobDone) return false;

			fetchResult();
			_client->destory();

			return true;
		}

		void terminateAndReSendTask()
		{
			//terminate
			_client->destory();

			//redo
			_client->create();
			_client->sendTask();
		}

	protected:

		void fetchResult()
		{
			detail::MakeFile2Object(_task->result, _strResultFileName.data());
		}

	protected:
		T* _task;
		bool _jobDone;

		std::string _strTaskFileName;
		std::string _strResultFileName;

		std::shared_ptr<IClient> _client;
	};

	template<typename T>
		bool DoSingleTask(T* tasks, const char* endpoint, size_t maxWaitTimeForSingleTask=50000)
	{
		CTaskProxy<T> proxy;
		proxy.setEndpoint(endpoint);

		proxy.setTask(tasks);
		proxy.sendTask(tasks);

		size_t nWaitTime=0;

		int nLoop=0;

		while( (++nLoop)<5 )
		{
			while( false == proxy.tryRecieveResult() )
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));//Sleep(100);
				nWaitTime+=100;

				if (nWaitTime>maxWaitTimeForSingleTask) break;
			}

			if (nWaitTime<=maxWaitTimeForSingleTask)
			{
				break;
			}

			if( false == proxy.tryRecieveResult() )
			{
				printf("terminate task and redo.\n");
				proxy.terminateAndReSendTask();
			}
		}

		return true;
	}

	template<typename T>
		bool DoMultiTask(T* tasks, size_t size, size_t maxWaitTimeForSingleTask=50000)
	{
		CTaskProxy<T>* proxy = new CTaskProxy<T>[size];

		for (size_t i=0; i<size; ++i)
		{
			proxy[i].setTask(&tasks[i]);
			proxy[i].sendTask(&tasks[i]);
		}

		std::vector<bool> doneFlags(size, false);

		size_t nFinishTask=0;

		while(nFinishTask<size)
		{
			// recieve success result
			for (size_t i=0; i<size; ++i)
			{
				if ( doneFlags[i] ) continue;

				size_t nWaitTime=0;

				while( false == proxy[i].tryRecieveResult() )
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));//Sleep(100);
					nWaitTime+=100;

					if (nWaitTime>maxWaitTimeForSingleTask) break;
				}

				if (nWaitTime<=maxWaitTimeForSingleTask)
				{
					doneFlags[i] = true;
					++nFinishTask;
				}
			}

			// resend failed tasks
			for (size_t i=0; i<size; ++i)
			{
				if ( doneFlags[i] ) continue;

				if( false == proxy[i].tryRecieveResult() )
				{
					printf("terminate:%d, and redo.\n", i);
					proxy[i].terminateAndReSendTask();
					//++nFinishTask;
				}
				else
				{
					doneFlags[i] = true;
					++nFinishTask;
				}
			}
		}

		delete [] proxy;

		return true;
	}

	template<typename T>
	bool DoMultiTask1(T* tasks, size_t size, size_t maxWaitTimeForSingleTask=50000)
	{
		CTaskProxy<T>* proxy = new CTaskProxy<T>[size];

		for (size_t i=0; i<size; ++i)
		{
			proxy[i].setTask(&tasks[i]);
			proxy[i].sendTask(&tasks[i]);
		}

		std::vector<bool> doneFlags(size, false);
		std::vector<size_t> vtWaitTime(size, 0);

		size_t nFinishTask=0;

		while(nFinishTask<size)
		{
			// recieve success result
			for (size_t i=0; i<size; ++i)
			{
				if ( doneFlags[i] ) continue;

				if( false == proxy[i].tryRecieveResult() )
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));//Sleep(100);
					vtWaitTime[i]+=100;
				}
				else
				{
					doneFlags[i] = true;
					++nFinishTask;
				}				
			}

			// resend failed task
			for (size_t i=0; i<size; ++i)
			{
				if ( doneFlags[i] ) continue;

				if( false == proxy[i].tryRecieveResult() )
				{
					if (vtWaitTime[i] > maxWaitTimeForSingleTask )
					{
						printf("terminate:%d, and redo.\n", i);
						proxy[i].terminateAndReSendTask();
						vtWaitTime[i]=0;
					}
				}
				else
				{
					doneFlags[i] = true;
					++nFinishTask;
				}
			}
		}

		delete [] proxy;

		return true;
	}
}

#endif