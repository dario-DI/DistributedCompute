#include <assert.h>

#include <sstream>

#include <zmq.h>
#include <DCompute/util.h>
#include <DCompute/zmqEx.h>

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

	int Util::RegistWorker(const char* id)
	{
		/*void* context = zmq_init(1);

		void* request = zmq_socket (context, ZMQ_REQ);
		int rc = zmq_connect(request, cex::DeltaInstance<IDComputeConfig>().getRequestEndPoint());
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

		DoSingleTask(&task, cex::DeltaInstance<IDComputeConfig>().getRequestEndPoint());

		return task.result;
		
	}

	int Util::UnRegistWorker(const char* id)
	{
		/*void* context = zmq_init(1);

		void* request = zmq_socket (context, ZMQ_REQ);
		int rc = zmq_connect(request, cex::DeltaInstance<IDComputeConfig>().getRequestEndPoint());
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

		DoSingleTask(&task, cex::DeltaInstance<IDComputeConfig>().getRequestEndPoint());

		return task.result;
	}

	int Util::GetWorkerSize()
	{
		/*void* context = zmq_init(1);

		void* request = zmq_socket (context, ZMQ_REQ);
		int rc = zmq_connect(request, cex::DeltaInstance<IDComputeConfig>().getRequestEndPoint());
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

		DoSingleTask(&task, cex::DeltaInstance<IDComputeConfig>().getRequestEndPoint());

		return task.result;
	}

	static bool GetModulePath(HMODULE hModule, std::string& path)
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

	std::shared_ptr<cex::IString> Util::CreateUniqueTempFile()
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

		std::shared_ptr<cex::IString> filename = cex::DeltaCreateRef<cex::IString>();
		filename->assign(szTempFile);

		//ReleaseMutex(hMutex); // 释放hMutex的持有权，注意这并不等同于删除Mutex对象
		//CloseHandle(hMutex);

		return filename;

	}

	void Util::DeleteTempFile(const char* strTempName)
	{
		//remove(strTempName.data());
		::DeleteFileA(strTempName);
	}

	int Util::DetectNumberOfProcessor() 
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return static_cast<int>(si.dwNumberOfProcessors);
	}

	std::shared_ptr<cex::IString> Util::GetHostName()
	{
		char temp[60];
		gethostname(temp, sizeof(temp));
		std::shared_ptr<cex::IString> str = cex::DeltaCreateRef<cex::IString>();
		str->assign(temp);
		return str;
	}

	/////////////////////////////////////////////////////////////////
	// class CDComputeConfig	
	class CDComputeConfigProxy : public IDComputeConfig
	{
	public:
		virtual const char* getJoberAddress()
		{
			InitializationIfNot();
			return joberAddress.data();
		}

		virtual const char* getClientEndPoint()
		{
			InitializationIfNot();
			return clientEndPoint.data();
		}

		virtual const char* getWorkerEndPoint()
		{
			InitializationIfNot();
			return workerEndPoint.data();
		}

		virtual const char* getRequestEndPoint()
		{
			InitializationIfNot();
			return requestEndPoint.data();
		}

	protected:
		void InitializationIfNot()
		{
			if (joberAddress.length()!=0 || clientEndPoint.length()!=0 ||
				workerEndPoint.length()!=0 || requestEndPoint.length()!=0)
			{
				return;
			}

			std::string strModulePath;
			GetModulePath(0, strModulePath);

			std::ostringstream oss;
			oss << strModulePath.data() << "Configure/DCompute.ini";

			char pBuf[MAX_PATH+1];

			GetPrivateProfileStringA( 
				"DCompute",
				"JoberAddress",
				"127.0.0.1",
				pBuf,
				MAX_PATH,
				oss.str().c_str()
				);

			joberAddress = pBuf;

			oss.clear();
			oss.str("");
			oss << "tcp://" << joberAddress.data() << ":" << DCOMPUTE_JOB_CLIENT_PORT;
			clientEndPoint = oss.str();

			oss.clear();
			oss.str("");
			oss << "tcp://" << joberAddress.data() << ":" << DCOMPUTE_JOB_WORKER_PORT;
			workerEndPoint = oss.str();

			oss.clear();
			oss.str("");
			oss << "tcp://" << joberAddress.data() << ":" << DCOMPUTE_JOB_REPLY_PORT;
			requestEndPoint = oss.str();
		}

	protected:
		bool bInitialization;

		std::string joberAddress;

		std::string clientEndPoint;
		std::string workerEndPoint;

		std::string requestEndPoint;
	};

	REGIST_DELTA_INSTANCE(IDComputeConfig, CDComputeConfigProxy);
}