#include "stdafx.h"
#include <assert.h>

#include <vector>
#include <algorithm>
#include <queue>
#include <zmq.h>

#include <DCompute/lru.h>
#include <DCompute/zmqEx.h>
#include <DCompute/util.h>

namespace DCompute {

	CLRURouter::CLRURouter()
	{
	}

	CLRURouter::~CLRURouter()
	{
		destory();
	}

	void CLRURouter::create()
	{
		_context = zmq_init(1);

		String address = CDComputeConfig::Instance()->joberAddress;

		char frontendAdress[30];
		sprintf(frontendAdress, "tcp://%s:%d", address.data(), DCOMPUTE_JOB_CLIENT_PORT);
		char backendAdress[30];
		sprintf(backendAdress, "tcp://%s:%d", address.data(), DCOMPUTE_JOB_WORKER_PORT);

		_frontend = zmq_socket (_context, ZMQ_ROUTER);
		int rc = zmq_bind (_frontend, frontendAdress);
		assert(rc==0);

		_backend = zmq_socket (_context, ZMQ_ROUTER);
		rc = zmq_bind (_backend, backendAdress);
		assert(rc==0);
	}

	void CLRURouter::destory()
	{
		if ( _context!=0 )
		{
			zmq_close(_backend);
			zmq_close(_frontend);
			zmq_term(_context);
		}

		_backend=0;
		_frontend=0;
		_context=0;
	}

	namespace detail
	{
		static int ForwardMessage(void* srcSocket, void* desSocket, zmq_msg_t& msg)
		{
			int rc;
			int more;
			size_t moresz;

			while (true) 
			{
				rc = zmq_recvmsg(srcSocket, &msg, 0);
				if (rc < 0)	return -1;

				moresz = sizeof more;
				rc = zmq_getsockopt(srcSocket, ZMQ_RCVMORE, &more, &moresz);
				if (rc < 0)
					return -1;

				rc = zmq_sendmsg(desSocket, &msg, more? ZMQ_SNDMORE: 0);
				if (rc < 0)
					return -1;
				if (more == 0)
					break;
			}

			return 0;
		}

		static int ReciveEmpty(void* socket)
		{
			char empty[2];
			int size = zmq_recv(socket, empty, 2, 0);
			assert (size == 0);
			return size;
		}
	}

	int CLRURouter::ReciveAddress(void* socket, String& addr)
	{
		char worker_addr[30];
		int size = zmq_recv (socket, worker_addr, 100, 0);
		assert(size>0 && size<30);
		addr.assign(worker_addr, size);

		char empty[2];
		int empty_size = zmq_recv(socket, empty, 2, 0);
		assert (empty_size == 0);

		return size;
	}

	int CLRURouter::SendAddress(void* socket, const String& addr)
	{
		int rc = zmq_send(socket,addr.data(), addr.length(), ZMQ_SNDMORE);
		assert(rc>=0);
		rc = zmq_send (socket, "", 0, ZMQ_SNDMORE);
		assert(rc>=0);

		return 0;
	}

	int CLRURouter::SendReady(void* socket)
	{
		int rc = zmq_send(socket, "READY", 5, ZMQ_SNDMORE);
		assert(rc>=0);
		rc = zmq_send (socket, "", 0, 0);
		assert(rc>=0);

		return 0;
	}

	UINT CLRURouter::run()
	{
		zmq_msg_t msg;
		int rc = zmq_msg_init(&msg);
		if (rc != 0)
			return -1;

		zmq_pollitem_t items [] = {
			{ _backend, 0, ZMQ_POLLIN, 0 },
			{ _frontend,  0, ZMQ_POLLIN, 0 }
		};

		std::queue<String> worker_queue;

		while (!_done) 
		{
			//rc = zmq_poll (&items [0], available_workers>0? 2: 1, -1);
			rc = zmq_poll (&items [0], 2, -1);
			if (rc < 0)
			{
				assert(false);
				return -1;
			}

			if (items [0].revents & ZMQ_POLLIN) 
			{
				String worker_addr;
				int worker_addr_size = ReciveAddress(_backend, worker_addr);
				worker_queue.push(worker_addr); 

				String client_addr;
				int client_addr_size = ReciveAddress(_backend, client_addr);

				if ( !(client_addr=="READY") ) 
				{
					SendAddress(_frontend,  client_addr);

					rc = detail::ForwardMessage(_backend, _frontend, msg);
					if (rc==-1) return rc;
				}
			}

			if (items [1].revents & ZMQ_POLLIN) 
			{
				if(worker_queue.size()==0)
				{
					Sleep(100);
					continue;
				}

				SendAddress(_backend,  worker_queue.front());

				rc = detail::ForwardMessage(_frontend, _backend, msg);
				if (rc==-1) return rc;

				worker_queue.pop();
			}
		}

		return 0;
	}
}