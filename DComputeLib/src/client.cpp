
#include <assert.h>

#include <stdio.h>

#include <zmq.h>
#include <DCompute/zmqEx.h>
#include <DCompute/client.h>
#include <DCompute/util.h>

namespace DCompute {

	class CClient : public IClient
	{
	public:
		CClient();

		~CClient();

	public:

		void setEndpoint(const char* endpoint) { _endPoint = endpoint; }


		bool create();

		void destory();


		void setTaskFile(const char* filename) { _strTaskFile = filename; }

		void setResultFile(const char* filename) { _strResultFile = filename; }


		bool sendTask();

		bool recieveResult(int doNotWait=0);

	private:
		std::string _endPoint;

		void* _context;
		void* _client;

		std::string _strTaskFile;
		std::string _strResultFile;
	};

CClient::CClient() : 
_context(0),
_client(0)
{
	_endPoint = cex::DeltaInstance<IDComputeConfig>().getClientEndPoint();
}

CClient::~CClient()
{
	destory();
}

bool CClient::create()
{
	_context = zmq_init(1);

	_client = zmq_socket (_context, ZMQ_REQ);
	int rc = zmq_connect(_client, _endPoint.data());
	assert(rc==0);
	//int erro_code = zmq_errno();

	return rc==0;
}

void CClient::destory()
{
	if ( _context!= 0 )
	{
		zmq_close(_client);
		zmq_term(_context);
	}

	_client=0;
	_context=0;
}


bool CClient::sendTask()
{
	bool nRet = ZmqEx::SendFile(_client, _strTaskFile.data() );
	assert(nRet==true);
	return nRet;
}

bool CClient::recieveResult(int doNotWait)
{
	return ZmqEx::Recv2File(_client, _strResultFile.data(), doNotWait);

	/*void* socket = _client;
	String& fileName = _strResultFile;

	size_t length;
	int recvSize = zmq_recv (socket, &length, sizeof(length), doNotWait);
	if(recvSize==-1) return false;
	assert(recvSize==sizeof(length));

	FILE* file = fopen(fileName.data(), "wb");
	if ( file == NULL ) return false;

	char pbuffer[ZMQ_SEND_ONCE_MAX];
	size_t nWriteSize=0;

	while (nWriteSize<length)
	{
		recvSize = zmq_recv (socket, pbuffer, ZMQ_SEND_ONCE_MAX, 0);
		if(recvSize<0) break;

		fwrite(pbuffer, 1, recvSize, file);

		nWriteSize += recvSize;
	}

	fclose(file);

	return true;*/
}

REGIST_DELTA_CREATOR(IClient, CClient);

////////////////////////////////////////////////////////////
// class CClientThread
class CClientThread : public detail::TThreadProxy<IClientThread>
{
public:
public:
	CClientThread();
	~CClientThread();

public:

	bool create();

	void destory();

	virtual void join()
	{
		_done = true;
		
		destory();

		__super::join();
	}


	void setTaskFile(const char* filename) { _strTaskFile = filename; }

	const char* getResultFile() const;

private:

	virtual unsigned int run();

private:

	void* _context;
	void* _client;

	std::string _strTaskFile;
	std::string _strResultFile;
};

CClientThread::CClientThread() : 
_context(0),
_client(0)
{
}

CClientThread::~CClientThread()
{
	destory();

	Util::DeleteTempFile( _strTaskFile.data() );
	Util::DeleteTempFile( _strResultFile.data() );
}

bool CClientThread::create()
{
	_context = zmq_init(1);

	_client = zmq_socket (_context, ZMQ_REQ);
	int rc = zmq_connect(_client, cex::DeltaInstance<IDComputeConfig>().getClientEndPoint());
	assert(rc==0);
	//int erro_code = zmq_errno();

	_strResultFile.assign(Util::CreateUniqueTempFile()->data());

	return rc==0;
}

void CClientThread::destory()
{
	if ( _context!= 0 )
	{
		zmq_close(_client);
		zmq_term(_context);
	}

	_client=0;
	_context=0;
}


const char* CClientThread::getResultFile() const
{
	//assert(!isRunning());
	assert(_done);
	return _strResultFile.data();
}

unsigned int CClientThread::run()
{
	// send task
	bool nRet = ZmqEx::SendFile(_client, _strTaskFile.data() );
	assert(nRet==true);

	// recieve result
	nRet = ZmqEx::Recv2File(_client, _strResultFile.data());
	assert(nRet==true);

	_done = true;

	return 0;

	//while ( !_done ) 
	//{
	//	if(0)
	//	{
	//		// send result
	//		char* result= "I'm client!";
	//		int recvSize = ZmqEx::Send(_client, result, strlen(result));

	//		//  do something
	//		Sleep(2000);

	//		//  wait for reply
	//		char* data;
	//		recvSize = ZmqEx::Recv(_client, data);
	//		printf(data);
	//	}
	//	
	//	if(1)
	//	{
	//		size_t kk = Util::GetWorkerSize();

	//		char msgSend[30];
	//		sprintf(msgSend, "client:%d. ", id);

	//		int recvSize = ZmqEx::Send(_client, msgSend, strlen(msgSend));

	//		//  wait for reply
	//		char* data;
	//		recvSize = ZmqEx::Recv(_client, data);

	//		char msgRecv[60];
	//		sprintf(msgRecv, "client:%d recived: %s. \n", id, data);
	//		printf(msgRecv);

	//		Sleep(20000);//  do something
	//	}

	//	
	//}

	//return 0;
}

REGIST_DELTA_CREATOR(IClientThread, CClientThread);

}