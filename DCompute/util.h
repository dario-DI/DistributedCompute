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
	class DCOMPUTE_API Util
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
		virtual const char* getJoberAddress() const=0;

		virtual const char* getClientEndPoint() const=0;
		virtual const char* getWorkerEndPoint() const=0;

		virtual const char* getRequestEndPoint() const=0;
	};

	class CThreadProxy
	{
	public:
		CThreadProxy() : _done(false) {}

		virtual ~CThreadProxy()=0
		{
			if (thread != nullptr)
			{
				thread->join();
			}
		}

	public:
		void start()
		{
			thread = std::make_shared<std::thread>(&CThreadProxy::run, this);
			thread->join();
		}

		void stop()
		{
			_done = true;
			if (thread != nullptr)
			{
				thread->join();
			}
		}

	protected:

		virtual unsigned int run()=0;

	protected:
		std::shared_ptr<std::thread> thread;

		volatile bool _done;
	};
}

#endif