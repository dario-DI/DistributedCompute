// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <fstream>
#include <zmq.h>
#include <io.h>

#include <DCompute/zmqEx.h>
#include <DCompute/util.h>

using namespace DCompute;

void ZmqExTest()
{
	//  Create the infrastructure
	void *ctx = zmq_init (0);
	assert (ctx);
	void *rep = zmq_socket (ctx, ZMQ_REP);
	assert (rep);
	int rc = zmq_bind (rep, "inproc://a");
	assert (rc == 0);

	void *req = zmq_socket (ctx, ZMQ_REQ);
	assert (req);
	rc = zmq_connect (req, "inproc://a");
	assert (rc == 0);

	// test send string
	{		
		char* msgSend = "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww";
		//char* msg = new char[10];
		//sprintf (msg, "n: %d,", request_nbr);
		int msgSize = ZmqEx::Send(req, msgSend, strlen(msgSend));
		//if ( msgSize<0 ) Sleep(1000);
		assert(msgSize==strlen(msgSend));
		//delete msg;

		char* msgRecv1=NULL;
		int recvSize = ZmqEx::Recv(rep, msgRecv1);
		printf(msgRecv1);
		assert(strcmp(msgRecv1, msgSend)==0);
		ZmqEx::Free(msgRecv1);

		recvSize = ZmqEx::Send(rep, "ok");

		char* msgRecv2=NULL;
		recvSize = ZmqEx::Recv(req, msgRecv2);
		assert(strcmp(msgRecv2, "ok")==0);
		ZmqEx::Free(msgRecv2);
	}


	// test send file
	{
		String testFileName = "../bin/debug/testfile.tmp";
		bool bRet = ZmqEx::SendFile(req, testFileName);
		assert(bRet==true);

		String recvFileName;
		Util::CreateUniqueTempFile(recvFileName);
		bRet = ZmqEx::RecvFile(rep, recvFileName);
		assert(bRet==true);

		FILE* sendFile=fopen(testFileName.data(), "rb");
		FILE* recvFile=fopen(recvFileName.data(), "rb");

		size_t length1 = filelength( fileno(sendFile) );
		size_t length2 = filelength( fileno(recvFile) );
		assert(length1==length2);

		char pbufferSend[ZMQ_SEND_ONCE_MAX];
		char pbufferRecv[ZMQ_SEND_ONCE_MAX];
		while ( !feof(sendFile) )
		{
			size_t nReadSizeSend = fread(pbufferSend, 1, ZMQ_SEND_ONCE_MAX, sendFile);
			size_t nReadSizeRecv = fread(pbufferRecv, 1, ZMQ_SEND_ONCE_MAX, recvFile);
			assert(nReadSizeRecv==nReadSizeSend);

			for(size_t i=0; i<nReadSizeSend; ++i)
			{
				assert(pbufferSend[i]==pbufferRecv[i]);
			}
		}

		fclose(sendFile);
		fclose(recvFile);

		Util::DeleteTempFile(recvFileName);
	}


	//  Deallocate the infrastructure.
	rc = zmq_close (rep);
	assert (rc == 0);
	rc = zmq_close (req);
	assert (rc == 0);
	rc = zmq_term (ctx);
	assert (rc == 0);
}