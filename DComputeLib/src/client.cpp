#include "stdafx.h"
#include <assert.h>

#include <stdio.h>

#include <zmq.h>
#include <DCompute/zmqEx.h>
#include <DCompute/client.h>
#include <DCompute/util.h>

namespace DCompute {

CClient::CClient() : 
_context(0),
_client(0)
{
	_endPoint = CDComputeConfig::Instance()->clientEndPoint.data();
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
	bool nRet = ZmqEx::SendFile(_client, _strTaskFile );
	assert(nRet==true);
	return nRet;
}

bool CClient::recieveResult(int doNotWait)
{
	return ZmqEx::RecvFile(_client, _strResultFile, doNotWait);

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

////////////////////////////////////////////////////////////
// class CClientThread
CClientThread::CClientThread() : 
_context(0),
_client(0)
{
}

CClientThread::~CClientThread()
{
	setDone();
	if (isRunning()) Sleep(1000);
	stop();

	destory();

	Util::DeleteTempFile( _strTaskFile );
	Util::DeleteTempFile( _strResultFile );
}

bool CClientThread::create()
{
	_context = zmq_init(1);

	_client = zmq_socket (_context, ZMQ_REQ);
	int rc = zmq_connect(_client, CDComputeConfig::Instance()->clientEndPoint.data());
	assert(rc==0);
	//int erro_code = zmq_errno();

	Util::CreateUniqueTempFile(_strResultFile);

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


String CClientThread::getResultFile() const
{
	assert(!isRunning());
	return _strResultFile;
}

UINT CClientThread::run()
{
	// send task
	bool nRet = ZmqEx::SendFile(_client, _strTaskFile );
	assert(nRet==true);

	// recieve result
	nRet = ZmqEx::RecvFile(_client, _strResultFile);
	assert(nRet==true);

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

}