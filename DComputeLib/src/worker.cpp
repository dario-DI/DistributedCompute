
#include <assert.h>

#include <zmq.h>
#include <DCompute/zmqEx.h>
#include <DCompute/worker.h>
#include <DCompute/util.h>
#include <DCompute/typeWrappers.h>
#include <DCompute/lru.h>

namespace DCompute {

	class CWorker : public detail::TThreadProxy<IWorker>
	{
	public:
		CWorker();
		~CWorker();

	public:

		bool create();

		void destory();

		virtual void join()
		{
			_done = true;

			destory();

			__super::join();
		}


		int getID() {return id;}

		void setID(int id){this->id = id;}

	protected:

		virtual unsigned int run();

	private:
		int id;
		void* _context;
		void* _worker;
	};

CWorker::CWorker() :_context(0),_worker(0)
{
}

CWorker::~CWorker()
{
	Util::UnRegistWorker(Util::GetHostName()->data());

	destory();
}

bool CWorker::create()
{
	_context = zmq_init(1);

#if DCOMPUTE_ROUTE_TYPE == DCOMPUTE_ROUTE_LRU
	_worker = zmq_socket (_context, ZMQ_REQ);
#else
	_worker = zmq_socket (_context, ZMQ_REP);
#endif

	int rc = zmq_connect(_worker, cex::DeltaInstance<IDComputeConfig>().getWorkerEndPoint());
	assert(rc==0);
	//int erro_code = zmq_errno();

	Util::RegistWorker(Util::GetHostName()->data());

	Sleep(1000);

	return rc==0;
}


void CWorker::destory()
{
	if ( _context!= 0 )
	{
		zmq_close(_worker);
		zmq_term(_context);
	}

	_worker=0;
	_context=0;
}

unsigned int CWorker::run()
{
#if DCOMPUTE_ROUTE_TYPE == DCOMPUTE_ROUTE_LRU
	LRURouterMethod::SendReady(_worker);
#endif

	while(!_done)
	{
#if DCOMPUTE_ROUTE_TYPE == DCOMPUTE_ROUTE_LRU
		std::shared_ptr<cex::IString> client_addr = LRURouterMethod::ReciveAddress(_worker);
#endif

		// get task
		std::shared_ptr<cex::IString> strTaskFile = Util::CreateUniqueTempFile();

		bool nRet = ZmqEx::Recv2File(_worker, strTaskFile->data());
		if (nRet == false)
		{
			if (_done) return 0;
			assert(false);
			return -1;
		}

		std::shared_ptr<cex::Interface> ptr = ReflectFile2Object( strTaskFile->data() );
		IDCTask* taskPtr = cex::DeltaQueryInterface<IDCTask>(ptr.get());
		assert(taskPtr!=NULL);

		if (taskPtr==NULL)
		{
#if DCOMPUTE_ROUTE_TYPE == DCOMPUTE_ROUTE_LRU
			LRURouterMethod::SendAddress(_worker, client_addr->data(), client_addr->length());
#endif
			char* errorMsg = "Type reflection error.";
			int nRet = ZmqEx::Send(_worker, errorMsg, strlen(errorMsg));
			assert(nRet>0);

			Util::DeleteTempFile(strTaskFile->data());

			Sleep(100);
			continue;
		}

		//test terminate
		//if(id%2==0) Sleep(10000000);

		taskPtr->Do();

		std::shared_ptr<cex::IString> strResultFile = Util::CreateUniqueTempFile();

		taskPtr->result2File(strResultFile->data());

#if DCOMPUTE_ROUTE_TYPE == DCOMPUTE_ROUTE_LRU
		LRURouterMethod::SendAddress(_worker, client_addr->data(), client_addr->length());
#endif

		// send result
		nRet = ZmqEx::SendFile(_worker, strResultFile->data());
		assert(nRet==true);


#ifdef _DEBUG
		printf("job done.\n");
#endif

		Sleep(100);

		Util::DeleteTempFile(strTaskFile->data());
		Util::DeleteTempFile(strResultFile->data());
	}

	return 0;
}

REGIST_DELTA_CREATOR(IWorker, CWorker);

//unsigned int CWorker::run()
//{
//	while(!_done)
//	{
//		// get task
//		String strTaskFile;
//		Util::CreateUniqueTempFile(strTaskFile);
//
//		bool nRet = ZmqEx::RecvFile(_worker, strTaskFile);
//		assert(nRet==true);
//
//		IType* ptr = TypeReflectorManager::ReflectFile2Object( strTaskFile );
//		IType_DCTask* taskPtr = Query<IType_DCTask*>(ptr);
//		assert(taskPtr!=NULL);
//
//		if (taskPtr==NULL)
//		{
//			int nRet = ZmqEx::Send(_worker, "Type reflection error.");
//			assert(nRet>0);
//
//			Util::DeleteTempFile(strTaskFile);
//
//			Sleep(100);
//			continue;
//		}
//
//		//test terminate
//		//if(id%2==0) Sleep(10000000);
//
//		taskPtr->Do();
//
//		String strResultFile;
//		Util::CreateUniqueTempFile(strResultFile);
//
//		taskPtr->Result2File(strResultFile);
//		nRet = ZmqEx::SendFile(_worker, strResultFile);
//		assert(nRet==true);
//
//
//#ifdef _DEBUG
//		printf("job done.\n");
//#endif
//
//		Sleep(100);
//
//		Util::DeleteTempFile(strTaskFile);
//		Util::DeleteTempFile(strResultFile);
//	}
//
//	return 0;
//}

//unsigned int CWorker::run()
//{
//	while(!_done)
//	{
//		// get task
//		String strTaskFile;
//		Util::CreateUniqueTempFile(strTaskFile);
//
//		bool nRet = ZmqEx::RecvFile(_worker, strTaskFile);
//		assert(nRet==true);
//
//		IType* ptr = TypeReflectorManager::ReflectFile2Object( strTaskFile );
//		IType_DCTask* taskPtr = Query<IType_DCTask*>(ptr);
//		assert(taskPtr!=NULL);
//
//		if (taskPtr==NULL)
//		{
//			int nRet = ZmqEx::Send(_worker, "Type reflection error.");
//			assert(nRet>0);
//
//			Util::DeleteTempFile(strTaskFile);
//
//			Sleep(100);
//			continue;
//		}
//
//		//test terminate
//		//if(id%2==0) Sleep(10000000);
//
//		taskPtr->Do();
//
//		String strResultFile;
//		Util::CreateUniqueTempFile(strResultFile);
//
//		taskPtr->Result2File(strResultFile);
//		nRet = ZmqEx::SendFile(_worker, strResultFile);
//		assert(nRet==true);
//
//#ifdef _DEBUG
//		printf("job done.\n");
//#endif
//
//		Sleep(100);
//
//		Util::DeleteTempFile(strTaskFile);
//		Util::DeleteTempFile(strResultFile);
//	}
//
//	return 0;
//
//
//	//while ( !_done ) 
//	//{
//	//	if(0)
//	//	{
//	//		//  wait for request
//	//		char clientAdress[32];
//	//		char temp[20];
//
//	//		size_t recvSize;
//
//	//		// address
//	//		////recvSize = zmq_recv(_worker, clientAdress, 32, 0);
//	//		//int mq;
//	//		// recvSize = zmq_recv(_worker, &mq, sizeof(mq), 0);
//
//	//		//// empty
//	//		//recvSize = zmq_recv(_worker, temp, 20, 0);
//	//		//assert(recvSize==0);
//
//	//		// data
//	//		char* data;
//	//		recvSize = ZmqEx::Recv(_worker, data);
//
//	//		//printf ("Received request: [%s]\n", clientAdress);
//	//		printf(data);
//
//	//		//  do something
//	//		Sleep(2000);
//
//	//		// send result
//	//		//recvSize = zmq_send(_worker, clientAdress, strlen(clientAdress), ZMQ_SNDMORE);
//	//		//assert(recvSize==strlen(clientAdress));
//
//	//		//recvSize = zmq_send(_worker, "", 0, ZMQ_SNDMORE);
//	//		//assert(recvSize==0);
//
//	//		char* result="work done!";
//	//		recvSize = ZmqEx::Send(_worker, result, strlen(result));
//	//	}
//
//	//       if(1)
//	//	{
//	//		char* data;
//	//		size_t recvSize = ZmqEx::Recv(_worker, data);
//	//		//printf(data);
//
//	//		char msgSend[60];
//	//		sprintf(msgSend, "worker:%d reply to %s", id, data);
//
//	//		recvSize = ZmqEx::Send(_worker, msgSend, strlen(msgSend));
//
//	//		//  do something
//	//		Sleep(20000);			
//	//	}
//	//}
//
//	//return 0;
//}
}