
#include <assert.h>

#include <sstream>
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

	namespace detail
	{
		CWokerCounter* CWokerCounter::Instance()
		{
			static CWokerCounter theInstance;
			return &theInstance;
		}
	}

	///////////////////////////////////////////////
	// class CReplyServer
	class CReplyServer : public detail::TThreadProxy<IReplyServer>
	{
	public:
		CReplyServer();

		~CReplyServer();

	public:

		void create();

		void destory();

		virtual void join()
		{
			_done = true;

			destory();

			__super::join();
		}

	protected:

		virtual unsigned int run();

	private:
		void* _context;
		void* _reply;
	};

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

		auto address = cex::DeltaInstance<IDComputeConfig>().getJoberAddress();
		std::ostringstream ossm;
		ossm << "tcp://" << address << ":" << DCOMPUTE_JOB_REPLY_PORT;
		std::string replyAddr = ossm.str();

		_reply = zmq_socket (_context,	ZMQ_REP);
		int rc = zmq_bind (_reply, replyAddr.data());
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

	unsigned int CReplyServer::run()
	{
		while (!_done) 
		{
			auto strTaskFile = Util::CreateUniqueTempFile();

			bool nRet = ZmqEx::Recv2File(_reply, strTaskFile->data());
			if (nRet == false)
			{
				if (_done) return 0;
				assert(false);
				return -1;
			}

			auto ptr = ReflectFile2Object( strTaskFile->data() );
			IDCTask* taskPtr = cex::DeltaQueryInterface<IDCTask>(ptr.get());
			assert(taskPtr!=NULL);

			if (taskPtr==NULL)
			{
				char* errorMsg = "Type reflection error.";
				int nRet = ZmqEx::Send(_reply, errorMsg, strlen(errorMsg));
				assert(nRet>0);

				Util::DeleteTempFile(strTaskFile->data());

				Sleep(100);
				continue;
			}

			taskPtr->Do();

			auto strResultFile = Util::CreateUniqueTempFile();

			taskPtr->result2File(strResultFile->data());
			nRet = ZmqEx::SendFile(_reply, strResultFile->data());
			assert(nRet==true);


#ifdef _DEBUG
			printf("reply server job done.\n");
#endif
			Sleep(100);

			Util::DeleteTempFile(strTaskFile->data());
			Util::DeleteTempFile(strResultFile->data());
		}

		return 0;
	}

	//unsigned int CReplyServer::run()
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
	REGIST_DELTA_CREATOR(IReplyServer, CReplyServer);

	////////////////////////////////////////////////////////////////////////////////
	// class CJoberServer
	class CJoberServer : public IJoberServer
	{
	public:
		CJoberServer();

		~CJoberServer();

	public:

		virtual void create();

		virtual void start();

		virtual void join();

	private:
		CReplyServer _reply;

		std::shared_ptr<ILRURouter> _router;
	};

	CJoberServer::CJoberServer()
	{
#if DCOMPUTE_ROUTE_TYPE == DCOMPUTE_ROUTE_LRU
		_router = cex::DeltaCreateRef<ILRURouter>();
#else
		_router = new CLBRouter();
#endif
	}

	CJoberServer::~CJoberServer()
	{
		join();
	}

	void CJoberServer::create()
	{
		_router->create();
		_reply.create();
	}

	void CJoberServer::start()
	{
		_router->start();
		_reply.start();
	}

	void CJoberServer::join()
	{
		_router->join();
		_reply.join();
	}

	REGIST_DELTA_CREATOR(IJoberServer, CJoberServer);
}