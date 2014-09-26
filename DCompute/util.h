/// \file util.h Copyright (C).
/// \brief brief-description
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/22 15:04
#pragma once

#ifndef _DCOMPUTE_UTIL_H_
#define _DCOMPUTE_UTIL_H_

#include <thread>
#include <DCompute/config>
#include <cex/string.h>

namespace DCompute 
{
	class DCOMPUTE_EXPORT_CLASS Util
	{
	public:
		static int RegistWorker(const char* id);

		static int UnRegistWorker(const char* id);

		static int GetWorkerSize();


		//static bool GetModulePath(HMODULE hModule, String& path);

		/// 获取临时文件(路径名)
		static std::shared_ptr<cex::IString> CreateUniqueTempFile();

		static void DeleteTempFile(const char* strTempName);


		static int DetectNumberOfProcessor();

		static std::shared_ptr<cex::IString> GetHostName();

	};

	class IDComputeConfig : public cex::Interface
	{
	public:
		// get the host address
		virtual const char* getJoberAddress()=0;

		virtual const char* getClientEndPoint()=0;
		virtual const char* getWorkerEndPoint()=0;

		virtual const char* getRequestEndPoint()=0;
	};

	namespace detail
	{
		template<typename Base>
		class TThreadProxy : public Base
		{
		public:
			TThreadProxy() : _done(false), thread(nullptr) {}

			virtual ~TThreadProxy()=0
			{
				join();
			}

		public:
			virtual void start()
			{
				if (thread == nullptr)
				{
					thread = std::make_shared<std::thread>(&TThreadProxy<Base>::run, this);
				}
			}

			virtual void join()
			{
				_done = true;
				
				if (thread != nullptr)
				{
					if (thread->joinable())
					{
						thread->join();
					}

					thread = nullptr;
				}
			}

			//virtual int stop(int milliseconds=TIME_WAITFOR_THREADSTOP)
			//{
			//	if (thread == nullptr) return 0;

			//	/* return exit code when thread terminates */
			//	HANDLE thrHandle = thread->native_handle();
			//	thread->detach();
			//	thread = nullptr;

			//	unsigned long res;
			//	if (WaitForSingleObject(thrHandle, milliseconds) == WAIT_FAILED
			//		|| GetExitCodeThread(thrHandle, &res) == 0)
			//		return _Thrd_error;

			//	return (CloseHandle(thrHandle) == 0 ? _Thrd_error : 0);
			//}

		protected:

			virtual unsigned int run()=0;

		protected:
			std::shared_ptr<std::thread> thread;

			volatile bool _done;

		};
	}
}

#endif