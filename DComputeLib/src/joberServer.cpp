#include "stdafx.h"
#include <assert.h>

#include <vector>
#include <algorithm>

#include <zmq.h>
#include <DCompute/zmqEx.h>
#include <DCompute/joberServer.h>
#include <DCompute/typeWrappers.h>

#include <DCompute/util.h>
#include <DCompute/lb.h>
#include <DCompute/lru.h>

namespace DCompute {

	///////////////////////////////////////////////////////////
	// class CWokerCounter
	/*class CWokerCounter
	{
	public:
		~CWokerCounter(){}

		std::vector<String> workers;

	private:
		CWokerCounter() {}

		static CWokerCounter* Instance();

		friend class CReplyServer;
	};*/

	CWokerCounter* CWokerCounter::Instance()
	{
		static CWokerCounter theInstance;
		return &theInstance;
	}

	///////////////////////////////////////////////
	// class CReplyServer
	CReplyServer::CReplyServer() : 
		_context(0),
		_reply(0)
	{
	}

	CReplyServer::~CReplyServer()
	{
		destory();
	}

	void CReplyServer::create()
	{
		_context = zmq_init(1);

		String address = CDComputeConfig::Instance()->joberAddress;

		char replyAdress[30];
		sprintf(replyAdress, "tcp://%s:%d", address.data(), DCOMPUTE_JOB_REPLY_PORT);

		_reply = zmq_socket (_context,	ZMQ_REP);
		int rc = zmq_bind (_reply, replyAdress);
		assert(rc==0);
	}

	void CReplyServer::destory()
	{
		if ( _context!=0 )
		{
			zmq_close(_reply);
			zmq_term(_context);
		}

		_reply=0;
		_context=0;
	}

	UINT CReplyServer::run()
	{
		while (!_done) 
		{
			String strTaskFile;
			Util::CreateUniqueTempFile(strTaskFile);

			bool nRet = ZmqEx::RecvFile(_reply, strTaskFile);
			assert(nRet==true);

			IType* ptr = TypeReflectorManager::ReflectFile2Object( strTaskFile );
			IType_DCTask* taskPtr = Query<IType_DCTask*>(ptr);
			assert(taskPtr!=NULL);

			if (taskPtr==NULL)
			{
				int nRet = ZmqEx::Send(_reply, "Type reflection error.");
				assert(nRet>0);

				Util::DeleteTempFile(strTaskFile);

				Sleep(100);
				continue;
			}

			taskPtr->Do();

			String strResultFile;
			Util::CreateUniqueTempFile(strResultFile);

			taskPtr->Result2File(strResultFile);
			nRet = ZmqEx::SendFile(_reply, strResultFile);
			assert(nRet==true);


#ifdef _DEBUG
			printf("reply server job done.\n");
#endif
			Sleep(100);

			Util::DeleteTempFile(strTaskFile);
			Util::DeleteTempFile(strResultFile);
		}

		return 0;
	}

	//UINT CReplyServer::run()
	//{
	//	while (!_done) 
	//	{
	//		String msg;
	//		int recvSize = ZmqEx::Recv(_reply, msg);

	//		size_t msgSize = msg.length();
	//		size_t findPos=msgSize; 

	//		if ( (findPos=msg.find(DC_REQ_REGIST_WORKER)) <msgSize )
	//		{
	//			String id = msg.substr(0, findPos);
	//			CWokerCounter::Instance()->workers.push_back(id);

	//			printf("regist: %s.\n", id.data());

	//			ZmqEx::Send(_reply, DC_REP_OK, strlen(DC_REP_OK));
	//		}
	//		else if ( (findPos=msg.find(DC_REQ_UNREGIST_WORKER)) <msgSize )
	//		{
	//			String id = msg.substr(0, findPos);

	//			std::vector<String>& workers=CWokerCounter::Instance()->workers;
	//			std::vector<String>::iterator itr = std::find(workers.begin(), workers.end(), id);
	//			if (itr != workers.end() )
	//			{
	//				workers.erase(itr);
	//			}

	//			ZmqEx::Send(_reply, DC_REP_OK, strlen(DC_REP_OK));
	//		}
	//		else if( (findPos=msg.find(DC_REQ_WORKER_NUM)) <msgSize )
	//		{
	//			int size = (int)CWokerCounter::Instance()->workers.size();

	//			char message[30];
	//			sprintf(message, "%d", size);

	//			ZmqEx::Send(_reply, message, strlen(message));
	//		}
	//		else
	//		{
	//			ZmqEx::Send(_reply, DC_REP_VALID, strlen(DC_REP_VALID));
	//		}

	//		Sleep(1000);
	//	}

	//	return 0;
	//}

	////////////////////////////////////////////////////////////////////////////////
	// class CJoberServer
	CJoberServer::CJoberServer()
	{
#if DCOMPUTE_ROUTE_TYPE == DCOMPUTE_ROUTE_LRU
		_router = new CLRURouter();
#else
		_router = new CLBRouter();
#endif
	}

	CJoberServer::~CJoberServer()
	{
		_reply.setDone();
		_router->setDone();

		if(_router!=NULL)
		{
			delete _router;
			_router=NULL;
		}
	}

	void CJoberServer::create()
	{
		_router->create();
		_reply.create();
	}

	bool CJoberServer::start()
	{
		_router->start();
		_reply.start();

		return true;
	}

	bool CJoberServer::stop()
	{
		_reply.setDone();
		_router->setDone();

		Sleep(1000);

		_reply.stop();
		_router->stop();

		return true;
	}
}