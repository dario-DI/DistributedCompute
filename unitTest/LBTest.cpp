// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <fstream>

#include <zmq.h>
#include <DCompute/zmqEx.h>
#include <DCompute/joberServer.h>
#include <DCompute/lb.h>
#include <DCompute/util.h>

using namespace DCompute;

class Client : public detail::TThreadProxy<cex::Interface>
{
public:
	Client()
	{
		id=0;

		_context = zmq_init(1);

		char clientAdress[30];
		sprintf(clientAdress, "tcp://127.0.0.1:%d", DCOMPUTE_JOB_CLIENT_PORT);

		_client = zmq_socket (_context, ZMQ_REQ);
		int rc = zmq_connect(_client, cex::DeltaInstance<IDComputeConfig>().getClientEndPoint());
		//int rc = zmq_connect(_client, clientAdress);
		assert(rc==0);
	}

	~Client()
	{
		zmq_close(_client);
		zmq_term(_context);
	}

	virtual void join()
	{
		_done = true;

		zmq_close(_client);
		zmq_term(_context);

		__super::join();
	}

	void* _context;
	void* _client;
	int id;

protected:

	UINT run()
	{
		//size_t kk = Util::GetWorkerSize();

		while(!_done)
		{
			char msgSend[30];
			sprintf(msgSend, "client:%d", id);

			int recvSize = ZmqEx::Send(_client, msgSend, strlen(msgSend));

			//  wait for reply
			std::string data = ZmqEx::Recv(_client)->data();

			size_t npos = data.find(msgSend);
			assert(npos<data.length());

			char msgRecv[50];
			sprintf(msgRecv, "client%d: recieved: %s", id, data.data());
			printf(msgRecv);

			id+=10;

			Sleep(2000);
			_done=true;

		}
		
		return 0;
	}
};

class Worker : public detail::TThreadProxy<cex::Interface>
{
public:
	Worker()
	{
		id = 0;

		_context = zmq_init(1);

		char workerAdress[30];
		sprintf(workerAdress, "tcp://127.0.0.1:%d", DCOMPUTE_JOB_WORKER_PORT);

		_worker = zmq_socket (_context, ZMQ_REP);
		int rc = zmq_connect(_worker, cex::DeltaInstance<IDComputeConfig>().getWorkerEndPoint());
		//int rc = zmq_connect(_worker, workerAdress);
		assert(rc==0);
		//int jj = zmq_errno();

		//Sleep(1000);
	}

	~Worker()
	{
		zmq_close(_worker);
		zmq_term(_context);
	}

	virtual void join()
	{
		_done = true;

		zmq_close(_worker);
		zmq_term(_context);
		_worker = 0;
		_context = 0;

		__super::join();
	}

	void* _context;
	void* _worker;
	int id;

protected:

	UINT run()
	{
		//char strID[60];
		//sprintf(strID, "%d", id);
		//Util::RegistWork( String(strID) );

		int counter=0;
		while(!_done)
		{
			++counter;

			auto pData = ZmqEx::Recv(_worker);
			if (pData == nullptr)
			{
				continue;
			}
			std::string data = pData->data();

			char msgSend[60];
			sprintf(msgSend, "worker:%d->%s\n", id, data.data());

			int recvSize = ZmqEx::Send(_worker, msgSend, strlen(msgSend));
			assert(recvSize==strlen(msgSend));
		}

		return 0;
	}
};

#define WORKERSIZE 1
#define CLIENTSIZE 10

CEX_TEST(LBTest)
{
	auto server = cex::DeltaCreateRef<ILBRouter>();
	server->create();
	server->start();

	if(0)
	{
		Worker worker[WORKERSIZE];
		Client client[WORKERSIZE];

		for (int i=0; i<WORKERSIZE; ++i)
		{
			worker[i].id = i;
			client[i].id = i;
		}

		for (int i=0; i<WORKERSIZE; ++i)
		{
			worker[i].start();
			client[i].start();
		}

		Sleep(1000);
	}

	if(1)
	{
		Worker worker[WORKERSIZE];
		Client client[CLIENTSIZE];

		for (int i=0; i<WORKERSIZE; ++i)
		{
			worker[i].id = i;
			worker[i].start();
		}

		for (int i=0; i<CLIENTSIZE; ++i)
		{
			client[i].id=i;
			client[i].start();
		}

		//Sleep(3000);

		//for (int i=CLIENTSIZE; i<CLIENTSIZE*2; ++i)
		//{
		//	client[i].id=i;
		//	client[i].start();
		//}

		Sleep(5000);

		for (int i=0; i<WORKERSIZE; ++i)
		{
			worker[i].join();
		}
	}

	server->join();
}

CEX_TEST_OFF(LBTest1)
{
	void* _context = zmq_init(1);

	int rc;

	void* _dealer = zmq_socket (_context, ZMQ_DEALER);
	rc = zmq_bind (_dealer, "tcp://127.0.0.1:5570");
	assert(rc==0);

	void* _router = zmq_socket (_context, ZMQ_ROUTER);
	rc = zmq_bind (_router, "tcp://127.0.0.1:5579");
	assert(rc==0);

	void* _worker = zmq_socket (_context, ZMQ_REP);
	rc = zmq_connect(_worker, "tcp://127.0.0.1:5570");
	assert(rc==0);

	void* _client = zmq_socket (_context, ZMQ_REQ);
	rc = zmq_connect(_client, "tcp://127.0.0.1:5579");
	assert(rc==0);

	{
		// send result
		//char* result= "I'm client!";
		//size_t recvSize = ZmqEx::Send(_client, result, strlen(result));

		rc = zmq_send (_client, "ABC", 3, ZMQ_SNDMORE);
		assert (rc == 3);
		rc = zmq_send (_client, "DEF", 3, 0);
	}

	for (int i=0; i<4; ++i)
	{
		zmq_msg_t msg;
		rc = zmq_msg_init (&msg);
		assert (rc == 0);
		rc = zmq_recvmsg (_router, &msg, 0);
		assert (rc >= 0);
		int rcvmore;
		size_t sz = sizeof (rcvmore);
		rc = zmq_getsockopt (_router, ZMQ_RCVMORE, &rcvmore, &sz);
		assert (rc == 0);
		rc = zmq_sendmsg (_dealer, &msg, rcvmore ? ZMQ_SNDMORE : 0);
		assert (rc >= 0);
		//if (rcvmore==0) break;
	}

	{
		//  wait for request
		char clientAdress[32];
		char temp[2];

		// address
		size_t recvSize = zmq_recv(_worker, clientAdress, 32, 0);

		// empty
		recvSize = zmq_recv(_worker, temp, 2, 0);
		assert(recvSize==0);

		// data
		std::string data = ZmqEx::Recv(_worker)->data();

		printf ("Received request: [%s]\n", clientAdress);
		printf(data.data());

		// send result
		recvSize = zmq_send(_worker, clientAdress, strlen(clientAdress), ZMQ_SNDMORE);
		assert(recvSize==strlen(clientAdress));

		recvSize = zmq_send(_worker, "", 0, ZMQ_SNDMORE);
		assert(recvSize==0);

		//char* result="work done!";
		//recvSize = ZmqEx::Send(_worker, result, strlen(result));
		//  Send the reply.
		rc = zmq_send (_worker, "GHI", 3, ZMQ_SNDMORE);
		assert (rc == 3);
		rc = zmq_send (_worker, "JKL", 3, 0);
		assert (rc == 3);
	}

	for (int i=0; i<4; ++i)
	{
		zmq_msg_t msg;
		rc = zmq_msg_init (&msg);
		assert (rc == 0);
		rc = zmq_recvmsg (_dealer, &msg, 0);
		assert (rc >= 0);
		int rcvmore;
		size_t sz = sizeof (rcvmore);
		rc = zmq_getsockopt (_dealer, ZMQ_RCVMORE, &rcvmore, &sz);
		assert (rc == 0);
		rc = zmq_sendmsg (_router, &msg, rcvmore ? ZMQ_SNDMORE : 0);
		assert (rc >= 0);
		if (rcvmore==0) break;
	}

	{
		//  wait for reply
		std::string data = ZmqEx::Recv(_client)->data();

		printf(data.data());		
	}

	Sleep(1000000);
}

CEX_TEST_OFF(LBTest0)
{
	fprintf (stderr, "test_reqrep_device running...\n");

	void *ctx = zmq_init (1);
	assert (ctx);

	//  Create a req/rep device.
	void *dealer = zmq_socket (ctx, ZMQ_DEALER);
	assert (dealer);
	int rc = zmq_bind (dealer, "tcp://127.0.0.1:5560");
	assert (rc == 0);
	void *router = zmq_socket (ctx, ZMQ_ROUTER);
	assert (router);
	rc = zmq_bind (router, "tcp://127.0.0.1:5561");
	assert (rc == 0);

	//  Create a worker.
	void *rep = zmq_socket (ctx, ZMQ_REP);
	assert (rep);
	rc = zmq_connect (rep, "tcp://127.0.0.1:5560");
	assert (rc == 0);

	//  Create a client.
	void *req = zmq_socket (ctx, ZMQ_REQ);
	assert (req);
	rc = zmq_connect (req, "tcp://127.0.0.1:5561");
	assert (rc == 0);

	//  Send a request.
	rc = zmq_send (req, "ABC", 3, ZMQ_SNDMORE);
	assert (rc == 3);
	rc = zmq_send (req, "DEF", 3, 0);
	assert (rc == 3);

	//  Pass the request through the device.
	for (int i = 0; i != 4; i++) {
		zmq_msg_t msg;
		rc = zmq_msg_init (&msg);
		assert (rc == 0);
		rc = zmq_recvmsg (router, &msg, 0);
		assert (rc >= 0);
		int rcvmore;
		size_t sz = sizeof (rcvmore);
		rc = zmq_getsockopt (router, ZMQ_RCVMORE, &rcvmore, &sz);
		assert (rc == 0);
		rc = zmq_sendmsg (dealer, &msg, rcvmore ? ZMQ_SNDMORE : 0);
		assert (rc >= 0);
	}

	//  Receive the request.
	char buff [3];
	rc = zmq_recv (rep, buff, 3, 0);
	assert (rc == 3);
	assert (memcmp (buff, "ABC", 3) == 0);
	int rcvmore;
	size_t sz = sizeof (rcvmore);
	rc = zmq_getsockopt (rep, ZMQ_RCVMORE, &rcvmore, &sz);
	assert (rc == 0);
	assert (rcvmore);
	rc = zmq_recv (rep, buff, 3, 0);
	assert (rc == 3);
	assert (memcmp (buff, "DEF", 3) == 0);
	rc = zmq_getsockopt (rep, ZMQ_RCVMORE, &rcvmore, &sz);
	assert (rc == 0);
	assert (!rcvmore);

	//  Send the reply.
	rc = zmq_send (rep, "GHI", 3, ZMQ_SNDMORE);
	assert (rc == 3);
	rc = zmq_send (rep, "JKL", 3, 0);
	assert (rc == 3);

	//  Pass the reply through the device.
	for (int i = 0; i != 4; i++) {
		zmq_msg_t msg;
		rc = zmq_msg_init (&msg);
		assert (rc == 0);
		rc = zmq_recvmsg (dealer, &msg, 0);
		assert (rc >= 0);
		int rcvmore;
		rc = zmq_getsockopt (dealer, ZMQ_RCVMORE, &rcvmore, &sz);
		assert (rc == 0);
		rc = zmq_sendmsg (router, &msg, rcvmore ? ZMQ_SNDMORE : 0);
		assert (rc >= 0);
	}

	//  Receive the reply.
	rc = zmq_recv (req, buff, 3, 0);
	assert (rc == 3);
	assert (memcmp (buff, "GHI", 3) == 0);
	rc = zmq_getsockopt (req, ZMQ_RCVMORE, &rcvmore, &sz);
	assert (rc == 0);
	assert (rcvmore);
	rc = zmq_recv (req, buff, 3, 0);
	assert (rc == 3);
	assert (memcmp (buff, "JKL", 3) == 0);
	rc = zmq_getsockopt (req, ZMQ_RCVMORE, &rcvmore, &sz);
	assert (rc == 0);
	assert (!rcvmore);

	//  Clean up.
	rc = zmq_close (req);
	assert (rc == 0);
	rc = zmq_close (rep);
	assert (rc == 0);
	rc = zmq_close (router);
	assert (rc == 0);
	rc = zmq_close (dealer);
	assert (rc == 0);
	rc = zmq_term (ctx);
	assert (rc == 0);

}