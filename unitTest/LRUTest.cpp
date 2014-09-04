// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <fstream>

#include <zmq.h>
//#include <DCompute/zmqEx.h>
#include <DCompute/thread.h>
#include "DCompute/util.h"

#include <DCompute/lru.h>
using namespace DCompute;

namespace Device{

class Client : public CThreadBase
{
public:
	Client()
	{
		id=0;

		_context = zmq_init(1);

		_client = zmq_socket (_context, ZMQ_REQ);
		int rc = zmq_connect(_client, CDComputeConfig::Instance()->clientEndPoint.data());
		//int rc = zmq_connect(_client, "tcp://127.0.0.1:5559");
		assert(rc==0);
	}

	~Client()
	{
		zmq_close(_client);
		zmq_term(_context);
	}

	void* _context;
	void* _client;
	int id;

protected:

	UINT run()
	{
		//  �������󲢻�ȡӦ����Ϣ
		zmq_send(_client, "HELLO", 5, 0);

		char reply[100];
		int size = zmq_recv(_client, reply, 100, 0);
		if(size==-1)
		{
			int error_code = zmq_errno();
			assert(false);
			return -1;
		}

		reply[size]=0;
		assert(strcmp(reply,"OK")==0);
		printf ("Client recieved: %s\n", reply);

		return 0;
	}
};

class Worker : public CThreadBase
{
public:
	Worker()
	{
		id = 0;

		_context = zmq_init(1);

		_worker = zmq_socket (_context, ZMQ_REQ);
		int rc = zmq_connect(_worker, CDComputeConfig::Instance()->workerEndPoint.data());
		//int rc = zmq_connect (_worker, "tcp://127.0.0.1:5560");
		assert(rc==0);

		//Sleep(1000);
	}

	~Worker()
	{
		setDone();
		zmq_close(_worker);
		zmq_term(_context);
	}

	void* _context;
	void* _worker;
	int id;

protected:

	UINT run()
	{
		//  ���ߴ���worker�Ѿ�׼����
		//zmq_send(_worker, "READY", 5, 0);
		CLRURouter::SendReady(_worker);

		while (!_done) 
		{
			//  ����Ϣ�п�֮֡ǰ���������ݣ��ŷ⣩����������
			//  �����п�֮֡ǰֻ��һ֡���������и��ࡣ
			char address[1000];
			int address_size = zmq_recv(_worker, address, 1000, 0);
			if (address_size==-1)
			{
				int error_code = zmq_errno();
				assert(false);
				return -1;
			}
			char empty[100];
			int size = zmq_recv(_worker, empty, 100, 0);
			assert (size == 0);

			//  ��ȡ���󣬲����ͻ�Ӧ
			char request[100];
			size = zmq_recv(_worker, request, 100, 0);
			request[size]=0;
			printf ("Worker recieved: %s\n", request);


			zmq_send(_worker, address, address_size, ZMQ_SNDMORE);
			zmq_send(_worker, "", 0, ZMQ_SNDMORE);
			zmq_send(_worker, "OK", 2, 0);

			return 0;
		}

		return 0;
	}
};

}

#define WORKERSIZE 1
#define CLIENTSIZE 1

void LRUTest(void)
{
	CLRURouter router;
	router.create();
	router.start();

	Device::Worker worker[WORKERSIZE];
	Device::Client client[CLIENTSIZE];

	for (int i=0; i<WORKERSIZE; ++i)
	{
		worker[i].start();
	}

	for (int i=0; i<CLIENTSIZE; ++i)
	{
		client[i].start();
	}

	Sleep(5000);


	for (int i=0; i<WORKERSIZE; ++i)
	{
		worker[i].stop();
	}

	for (int i=0; i<CLIENTSIZE; ++i)
	{
		client[i].stop();
	}

	router.stop();
}


//  ���Ӳ�����ʹ��һ���ɴ洢�κ����͵�����ʵ��
#define DEQUEUE(q) memmove (&(q)[0], &(q)[1], sizeof (q) - sizeof (q [0]))

void LRUTest0(void)
{
	String address = CDComputeConfig::Instance()->joberAddress;

	char frontendAdress[30];
	sprintf(frontendAdress, "tcp://%s:%d", address.data(), DCOMPUTE_JOB_CLIENT_PORT);
	char backendAdress[30];
	sprintf(backendAdress, "tcp://%s:%d", address.data(), DCOMPUTE_JOB_WORKER_PORT);

	//  ׼��0MQ�����ĺ��׽���
	void *context = zmq_init (1);
	void *frontend = zmq_socket (context, ZMQ_ROUTER);
	void *backend  = zmq_socket (context, ZMQ_ROUTER);
	zmq_bind (frontend, frontendAdress);
	zmq_bind (backend,  backendAdress);

	Device::Worker worker[WORKERSIZE];
	Device::Client client[CLIENTSIZE];

	for (int i=0; i<WORKERSIZE; ++i)
	{
		worker[i].start();
	}

	for (int i=0; i<CLIENTSIZE; ++i)
	{
		client[i].start();
	}

	Sleep(1000);

	//  LRU�߼�
	//  - һֱ��backend�л�ȡ��Ϣ�����г���һ��worker����ʱ�Ŵ�frontend��ȡ��Ϣ��
	//  - ��woker��Ӧʱ���Ὣ��worker���Ϊ��׼���ã���ת��woker�Ļ�Ӧ��client
	//  - ���client���������󣬾ͽ�������ת������һ��worker

	//  ��ſ���worker�Ķ���
	int available_workers = 0;
	char *worker_queue [10];

	int client_nbr = CLIENTSIZE;

	while (1) {
		zmq_pollitem_t items [] = {
			{ backend,  0, ZMQ_POLLIN, 0 },
			{ frontend, 0, ZMQ_POLLIN, 0 }
		};
		zmq_poll (items, 2, -1);

		//  ����backend��worker�Ķ���
		if (items [0].revents & ZMQ_POLLIN) {
			//  ��worker�ĵ�ַ���
			char worker_addr[100];
			int worker_addr_size = zmq_recv (backend, worker_addr, 100, 0);
			if (worker_addr_size==0) continue;
			assert (available_workers < WORKERSIZE);
			worker_queue [available_workers++] = worker_addr;

			//  ������֡

			char empty[100];
			int size = zmq_recv(backend, empty, 100, 0);
			assert (size == 0);

			// ����֡�ǡ�READY������һ��client�ĵ�ַ
			char client_addr[100];
			int client_addr_size = zmq_recv(backend, client_addr, 100, 0);
			client_addr[client_addr_size]=0;

			size = zmq_recv (backend, empty, 100, 0);
			assert (size == 0);

			//  �����һ��Ӧ����Ϣ����ת����client
			if (strcmp (client_addr, "READY") != 0) {
				char reply[100];
				int reply_size = zmq_recv (backend, reply, 100, 0);

				zmq_send(frontend, client_addr, client_addr_size, ZMQ_SNDMORE);
				zmq_send (frontend, "", 0, ZMQ_SNDMORE);
				zmq_send     (frontend, reply, reply_size, 0);

				//if (--client_nbr == 0)
					//break;      //  ����N����Ϣ���˳�
			}
		}
		if (items [1].revents & ZMQ_POLLIN) {
			
			if(available_workers<=0)
			{
				Sleep(100);
				continue;
			}

			//  ��ȡ��һ��client�����󣬽������е�worker����
			//  client�������Ϣ��ʽ�ǣ�[client��ַ][��֡][��������]
			char client_addr[100];
			int client_addr_size = zmq_recv (frontend, client_addr, 100, 0);
			char empty[100];
			int size = zmq_recv (frontend, empty, 100, 0);
			assert (size == 0);

			char request[100];
			int request_size = zmq_recv (frontend, request, 100, 0);

			zmq_send (backend, worker_queue[0], 5, ZMQ_SNDMORE);
			zmq_send (backend, "", 0, ZMQ_SNDMORE);
			zmq_send (backend, client_addr, 5, ZMQ_SNDMORE);
			zmq_send (backend, "", 0, ZMQ_SNDMORE);
			zmq_send     (backend, request, request_size, 0);

			//  ����worker�ĵ�ַ����
			DEQUEUE (worker_queue);
			available_workers--;
		}
	}

	zmq_close (frontend);
	zmq_close (backend);
	zmq_term (context);
}