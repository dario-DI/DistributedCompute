
#include <assert.h>

#include <sstream>
#include <vector>
#include <algorithm>

#include <zmq.h>
#include <DCompute/zmqEx.h>
#include <DCompute/lb.h>

#include <DCompute/util.h>

namespace DCompute {

	class CLBRouter : public detail::TRouterThread<ILBRouter>
	{
	public:
		CLBRouter();

		virtual ~CLBRouter();

		virtual void create();

	protected:
		virtual unsigned int run();
	};

	CLBRouter::CLBRouter()
	{
	}

	CLBRouter::~CLBRouter()
	{
	}

	void CLBRouter::create()
	{
		_context = zmq_init(1);

		auto address = cex::DeltaInstance<IDComputeConfig>().getJoberAddress();

		std::ostringstream oss;

		oss.clear();
		oss << "tcp://" << address << ":" << DCOMPUTE_JOB_CLIENT_PORT;
		std::string routerAdress = oss.str();

		oss.clear();
		oss.str("");
		oss << "tcp://" << address << ":" << DCOMPUTE_JOB_WORKER_PORT;
		std::string dealerAdress = oss.str();;


		_frontend = zmq_socket (_context, ZMQ_ROUTER);
		int rc = zmq_bind (_frontend, routerAdress.data());
		assert(rc==0);

		_backend = zmq_socket (_context, ZMQ_DEALER);
		rc = zmq_bind (_backend, dealerAdress.data());
		assert(rc==0);
	}

	unsigned int CLBRouter::run()
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
			{
				if (_done) return 0;
				return -1;
			}

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

	REGIST_DELTA_CREATOR(ILBRouter, CLBRouter);
}