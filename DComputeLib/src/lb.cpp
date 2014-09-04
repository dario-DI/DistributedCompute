#include "stdafx.h"
#include <assert.h>

#include <vector>
#include <algorithm>

#include <zmq.h>
#include <DCompute/zmqEx.h>
#include <DCompute/lb.h>

#include <DCompute/util.h>

namespace DCompute {

	CLBRouter::CLBRouter()
	{
	}

	CLBRouter::~CLBRouter()
	{
		destory();
	}

	void CLBRouter::create()
	{
		_context = zmq_init(1);

		String address = CDComputeConfig::Instance()->joberAddress;

		char routerAdress[30];
		sprintf(routerAdress, "tcp://%s:%d", address.data(), DCOMPUTE_JOB_CLIENT_PORT);
		char dealerAdress[30];
		sprintf(dealerAdress, "tcp://%s:%d", address.data(), DCOMPUTE_JOB_WORKER_PORT);

		_frontend = zmq_socket (_context, ZMQ_ROUTER);
		int rc = zmq_bind (_frontend, routerAdress);
		assert(rc==0);

		_backend = zmq_socket (_context, ZMQ_DEALER);
		rc = zmq_bind (_backend, dealerAdress);
		assert(rc==0);
	}

	void CLBRouter::destory()
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

	UINT CLBRouter::run()
	{
		//int ret = zmq_device(ZMQ_QUEUE, _frontend, _backend);
		//	return ret;

		zmq_msg_t msg;
		int rc = zmq_msg_init(&msg);
		if (rc != 0)
			return -1;

		//  The algorithm below assumes ratio of requests and replies processed
		//  under full load to be 1:1.

		//  TODO: The current implementation drops messages when
		//  any of the pipes becomes full.

		int more;
		size_t moresz;
		zmq_pollitem_t items [] = {
			{ _frontend, 0, ZMQ_POLLIN, 0 },
			{ _backend,  0, ZMQ_POLLIN, 0 }
		};

		while ( !_done ) {
			//  Wait while there are either requests or replies to process.
			rc = zmq_poll (&items [0], 2, -1);
			if (rc < 0)
				return -1;

			//  Process a request.
			if (items [0].revents & ZMQ_POLLIN) {
				while (true) {
					rc = zmq_recvmsg(_frontend, &msg, 0);
					if (rc < 0)
						return -1;

					moresz = sizeof more;
					rc = zmq_getsockopt(_frontend, ZMQ_RCVMORE, &more, &moresz);
					if (rc < 0)
						return -1;

					rc = zmq_sendmsg(_backend,&msg, more? ZMQ_SNDMORE: 0);
					if (rc < 0)
						return -1;
					if (more == 0)
						break;
				}
			}
			//  Process a reply.
			if (items [1].revents & ZMQ_POLLIN) {
				while (true) {
					rc = zmq_recvmsg(_backend,&msg, 0);
					if (rc < 0)
						return -1;

					moresz = sizeof more;
					rc = zmq_getsockopt(_backend, ZMQ_RCVMORE, &more, &moresz);
					if (rc < 0)
						return -1;

					rc = zmq_sendmsg(_frontend, &msg, more? ZMQ_SNDMORE: 0);
					if (rc < 0)
						return -1;
					if (more == 0)
						break;
				}
			}

		}
		return 0;
	}
}