// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <fstream>

#include <sstream>
#include <zmq.h>
//#include <DCompute/zmqEx.h>
#include "DCompute/util.h"

#include <DCompute/lru.h>
#include <cex/deltareflection.h>
using namespace DCompute;

namespace Device{

class Client : public detail::TThreadProxy<cex::Interface>
{
public:
	Client()
	{
		id=0;

		_context = zmq_init(1);

		_client = zmq_socket (_context, ZMQ_REQ);
		int rc = zmq_connect(_client, cex::DeltaInstance<IDComputeConfig>().getClientEndPoint());
		//int rc = zmq_connect(_client, "tcp://127.0.0.1:5559");
		assert(rc==0);
	}

	~Client()
	{
		join();
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
		//  发送请求并获取应答信息
		zmq_send(_client, "HELLO", 6, 0);

		char reply[100];
		int size = zmq_recv(_client, reply, 100, 0);
		if(size==-1)
		{
			int error_code = zmq_errno();
			if (_done) return 0;
			assert(false);
			return -1;
		}

		reply[size]=0;
		assert(strcmp(reply,"OK")==0);
		printf ("Client recieved: %s\n", reply);

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

		_worker = zmq_socket (_context, ZMQ_REQ);
		int rc = zmq_connect(_worker, cex::DeltaInstance<IDComputeConfig>().getWorkerEndPoint());
		//int rc = zmq_connect (_worker, "tcp://127.0.0.1:5560");
		assert(rc==0);

		//Sleep(1000);
	}

	~Worker()
	{
		join();
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
		//  告诉代理worker已经准备好
		//zmq_send(_worker, "READY", 5, 0);
		LRURouterMethod::SendReady(_worker);

		while (!_done) 
		{
			//  将消息中空帧之前的所有内容（信封）保存起来，
			//  本例中空帧之前只有一帧，但可以有更多。
			char address[1000];
			int address_size = zmq_recv(_worker, address, 1000, 0);
			//auto address = LRURouterMethod::ReciveAddress(_worker);
			if (address_size==0)
			{
				int error_code = zmq_errno();
				if (_done) return 0;
				assert(false);
				return -1;
			}
			char empty[100];
			int size = zmq_recv(_worker, empty, 100, 0);
			assert (size == 0);

			//  获取请求，并发送回应
			char request[100];
			size = zmq_recv(_worker, request, 100, 0);
			request[size]=0;
			printf ("Worker recieved: %s\n", request);


			zmq_send(_worker, address, address_size, ZMQ_SNDMORE);
			zmq_send(_worker, "", 0, ZMQ_SNDMORE);
			zmq_send(_worker, "OK", 3, 0);

			return 0;
		}

		return 0;
	}
};

}

#define WORKERSIZE 1
#define CLIENTSIZE 1

CEX_TEST(LRUTest)
{
	auto router = cex::DeltaCreateRef<ILRURouter>();
	router->create();
	router->start();

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
		worker[i].join();
	}

	for (int i=0; i<CLIENTSIZE; ++i)
	{
		client[i].join();
	}

	router->join();

	system("pause");
}

//  出队操作，使用一个可存储任何类型的数组实现
#define DEQUEUE(q) memmove (&(q)[0], &(q)[1], sizeof (q) - sizeof (q [0]))

CEX_TEST_OFF(LRUTest)
{
	std::string address = cex::DeltaInstance<IDComputeConfig>().getJoberAddress();

	std::ostringstream oss;

	oss.clear();
	oss << "tcp://" << address << ":" << DCOMPUTE_JOB_CLIENT_PORT;
	std::string frontendAdress = oss.str();

	oss.clear();
	oss.str("");
	oss << "tcp://" << address << ":" << DCOMPUTE_JOB_WORKER_PORT;
	std::string backendAdress = oss.str();

	//  准备0MQ上下文和套接字
	void *context = zmq_init (1);
	void *frontend = zmq_socket (context, ZMQ_ROUTER);
	void *backend  = zmq_socket (context, ZMQ_ROUTER);
	zmq_bind (frontend, frontendAdress.data());
	zmq_bind (backend,  backendAdress.data());

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

	//Sleep(1000);

	//  LRU逻辑
	//  - 一直从backend中获取消息；当有超过一个worker空闲时才从frontend获取消息。
	//  - 当woker回应时，会将该worker标记为已准备好，并转发woker的回应给client
	//  - 如果client发送了请求，就将该请求转发给下一个worker

	//  存放可用worker的队列
	int available_workers = 0;
	char *worker_queue [10];

	int client_nbr = CLIENTSIZE;

	while (1) {
		zmq_pollitem_t items [] = {
			{ backend,  0, ZMQ_POLLIN, 0 },
			{ frontend, 0, ZMQ_POLLIN, 0 }
		};
		zmq_poll (items, 2, -1);

		//  处理backend中worker的队列
		if (items [0].revents & ZMQ_POLLIN) {
			//  将worker的地址入队
			char worker_addr[100];
			int worker_addr_size = zmq_recv (backend, worker_addr, 100, 0);
			if (worker_addr_size==0) continue;
			assert (available_workers < WORKERSIZE);
			worker_queue [available_workers++] = worker_addr;

			//  跳过空帧

			char empty[100];
			int size = zmq_recv(backend, empty, 100, 0);
			assert (size == 0);

			// 第三帧是“READY”或是一个client的地址
			char client_addr[100];
			int client_addr_size = zmq_recv(backend, client_addr, 100, 0);
			client_addr[client_addr_size]=0;

			size = zmq_recv (backend, empty, 100, 0);
			assert (size == 0);

			//  如果是一个应答消息，则转发给client
			if (strcmp (client_addr, "READY") != 0) {
				char reply[100];
				int reply_size = zmq_recv (backend, reply, 100, 0);

				zmq_send(frontend, client_addr, client_addr_size, ZMQ_SNDMORE);
				zmq_send (frontend, "", 0, ZMQ_SNDMORE);
				zmq_send     (frontend, reply, reply_size, 0);

				//if (--client_nbr == 0)
					//break;      //  处理N条消息后退出
			}
		}
		if (items [1].revents & ZMQ_POLLIN) {
			
			if(available_workers<=0)
			{
				Sleep(100);
				continue;
			}

			//  获取下一个client的请求，交给空闲的worker处理
			//  client请求的消息格式是：[client地址][空帧][请求内容]
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

			//  将该worker的地址出队
			DEQUEUE (worker_queue);
			available_workers--;
		}
	}

	Sleep(3000);

	for (int i=0; i<CLIENTSIZE; ++i)
	{
		client[i].join();
	}

	for (int i=0; i<WORKERSIZE; ++i)
	{
		worker[i].join();
	}

	zmq_close (frontend);
	zmq_close (backend);
	zmq_term (context);
}