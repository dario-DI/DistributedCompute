#include "stdafx.h"
#include <assert.h>

#include <zmq.h>
#include <DCompute/util.h>
#include <DCompute/zmqEx.h>
#include <DCompute/string.h>
#include <DCompute/contracts.h>
#include <DCompute/task.h>

#pragma comment(lib,"ws2_32.lib")

//#if _WIN32||_WIN64
//static inline int DetectNumberOfWorkers() {
//	SYSTEM_INFO si;
//	GetSystemInfo(&si);
//	return static_cast<int>(si.dwNumberOfProcessors);
//}
//
//#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__sun) 
//static inline int DetectNumberOfWorkers() {
//	long number_of_workers;
//
//#if (defined(__FreeBSD__) || defined(__sun)) && defined(_SC_NPROCESSORS_ONLN) 
//	number_of_workers = sysconf(_SC_NPROCESSORS_ONLN);
//
//	// In theory, sysconf should work everywhere.
//	// But in practice, system-specific methods are more reliable
//#elif defined(__linux__)
//	number_of_workers = get_nprocs();
//#elif defined(__APPLE__)
//	int name[2] = {CTL_HW, HW_AVAILCPU};
//	int ncpu;
//	size_t size = sizeof(ncpu);
//	sysctl( name, 2, &ncpu, &size, NULL, 0 );
//	number_of_workers = ncpu;
//#else
//#error DetectNumberOfWorkers: Method to detect the number of online CPUs is unknown
//#endif
//
//	// Fail-safety strap
//	if ( number_of_workers < 1 ) {
//		number_of_workers = 1;
//	}
//
//	return number_of_workers;
//}
//
//#else
//#error DetectNumberOfWorkers: OS detection method is unknown
//#endif /* os kind */ 


namespace DCompute {

	int Util::RegistWorker(const String& id)
	{
		/*void* context = zmq_init(1);

		void* request = zmq_socket (context, ZMQ_REQ);
		int rc = zmq_connect(request, CDComputeConfig::Instance()->requestEndPoint.data());
		assert(rc==0);		

		char message[30];
		sprintf(message, "%s:%s", id.data(), DC_REQ_REGIST_WORKER);
		int size = ZmqEx::Send(request, message, strlen(message));
		if ( size < 0 ) return false;

		String result;
		size = ZmqEx::Recv(request, result);
		if ( size < 0 ) return false;

		zmq_close(request);
		zmq_term(context);

		printf(result.data());
		
		return true;*/

		Contract::WorkerInfo task;
		task.registType = Contract::WorkerInfo::regist;
		task.id = id;

		DoSingleTask(&task, CDComputeConfig::Instance()->requestEndPoint.data());

		return task.result;
		
	}

	int Util::UnRegistWorker(const String& id)
	{
		/*void* context = zmq_init(1);

		void* request = zmq_socket (context, ZMQ_REQ);
		int rc = zmq_connect(request, CDComputeConfig::Instance()->requestEndPoint.data());
		assert(rc==0);

		char message[30];
		sprintf(message, "%s:%s", id.data(), DC_REQ_UNREGIST_WORKER);
		int size = ZmqEx::Send(request, message, strlen(message));
		if ( size < 0 ) return false;

		String result;
		size = ZmqEx::Recv(request, result);
		if ( size < 0 ) return false;

		zmq_close(request);
		zmq_term(context);

		printf(result.data());

		return false;*/

		Contract::WorkerInfo task;
		task.registType = Contract::WorkerInfo::unregist;
		task.id = id;

		DoSingleTask(&task, CDComputeConfig::Instance()->requestEndPoint.data());

		return task.result;
	}

	int Util::GetWorkerSize()
	{
		/*void* context = zmq_init(1);

		void* request = zmq_socket (context, ZMQ_REQ);
		int rc = zmq_connect(request, CDComputeConfig::Instance()->requestEndPoint.data());
		assert(rc==0);

		char message[30];
		sprintf(message, "%s:%s", "*", DC_REQ_WORKER_NUM);
		ZmqEx::Send(request, message, strlen(message));

		String result;
		ZmqEx::Recv(request, result);

		zmq_close(request);
		zmq_term(context);

		printf( "worker counter: %s \n", result.data());

		size_t size = atoi(result.data());

		return size;*/

		Contract::WorkerInfo task;
		task.registType = Contract::WorkerInfo::getWorkerNumber;
		//task.id = id;

		DoSingleTask(&task, CDComputeConfig::Instance()->requestEndPoint.data());

		return task.result;
	}

	static bool GetModulePath(HMODULE hModule, String& path)
	{
		char pBuf[MAX_PATH+1];
		GetModuleFileNameA(hModule, pBuf,MAX_PATH);

		size_t len = strlen(pBuf);
		int pos=0;

		for(size_t i=0;i<len; ++i)
		{
			if(pBuf[i]=='\\')
			{
				pos=i;
			}
		}

		pBuf[pos+1] = 0;

		path = pBuf;

		return true;
	}

	bool Util::CreateUniqueTempFile(String& strTempName)
	{
		//HANDLE hMutex = CreateMutexA(NULL, FALSE, "CreateUniqueTempFile");
		//if (NULL == hMutex)
		//{
		//	assert(false);//Error("Create mutex error.");
		//	return false;
		//}

		//DWORD dw = WaitForSingleObject(hMutex, 10000);
		//if (WAIT_FAILED == dw)
		//{
		//	assert(false);//Error("Wait for mutex error.");
		//	CloseHandle(hMutex); // 释放句柄，当指向同一系统对象的所有句柄释放后，该对象将被删除。
		//	return false;
		//}
		//else if (WAIT_TIMEOUT == dw)
		//{
		//	// 另外一个实例正在运行
		//	CloseHandle(hMutex);
		//	assert(false);
		//	return false;
		//}

		//Get the temporary files directory.       
		char szTempPath[MAX_PATH];       
		DWORD dwResult=:: GetTempPathA(MAX_PATH, szTempPath);       
		assert(dwResult);       
		//Create a unique temporary file.      
		char szTempFile[MAX_PATH];    
		UINT nResult=GetTempFileNameA(szTempPath, "~dc", 0,szTempFile);       
		assert (nResult);

		strTempName=szTempFile;

		//ReleaseMutex(hMutex); // 释放hMutex的持有权，注意这并不等同于删除Mutex对象
		//CloseHandle(hMutex);

		return true;

	}

	bool Util::DeleteTempFile(const String& strTempName)
	{
		//remove(strTempName.data());
		BOOL bRet = ::DeleteFileA(strTempName.data());
		return bRet==TRUE;
	}

	static bool GetConfigureJoberAddress(String& addr)
	{
		addr = CDComputeConfig::Instance()->joberAddress;

		/*String strModulePath;
		GetModulePath(0, strModulePath);

		char strConfigFileName[MAX_PATH+1];
		sprintf(strConfigFileName, "%s%s", strModulePath.data(), "Configure/DCompute.ini");

		char pBuf[MAX_PATH+1];

		GetPrivateProfileStringA( 
			"DCompute",
			"JoberAddress",
			"127.0.0.1",
			pBuf,
			MAX_PATH,
			strConfigFileName
			);

		addr = pBuf;*/

		return true;
	}

	int Util::DetectNumberOfProcessor() 
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return static_cast<int>(si.dwNumberOfProcessors);
	}

	void Util::GetHostName( String& name )
	{
		char temp[60];
		gethostname(temp, sizeof(temp));
		name = temp;
	}

	/////////////////////////////////////////////////////////////////
	// class CDComputeConfig
	CDComputeConfig* CDComputeConfig::Instance()
	{
		static CDComputeConfig theInstance;
		return &theInstance;
	}

	CDComputeConfig::CDComputeConfig()
	{
		String strModulePath;
		GetModulePath(0, strModulePath);

		char strConfigFileName[MAX_PATH+1];
		sprintf(strConfigFileName, "%s%s", strModulePath.data(), "Configure/DCompute.ini");

		char pBuf[MAX_PATH+1];

		GetPrivateProfileStringA( 
			"DCompute",
			"JoberAddress",
			"127.0.0.1",
			pBuf,
			MAX_PATH,
			strConfigFileName
			);

		joberAddress = pBuf;

		char clientAdress[128];
		sprintf(clientAdress, "tcp://%s:%d", joberAddress.data(), DCOMPUTE_JOB_CLIENT_PORT);
		clientEndPoint = clientAdress;

		char workerAdress[128];
		sprintf(workerAdress, "tcp://%s:%d", joberAddress.data(), DCOMPUTE_JOB_WORKER_PORT);
		workerEndPoint = workerAdress;

		char requestAdress[30];
		sprintf(requestAdress, "tcp://%s:%d", joberAddress.data(), DCOMPUTE_JOB_REPLY_PORT);
		requestEndPoint = requestAdress;
	}
}