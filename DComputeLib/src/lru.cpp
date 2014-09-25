
#include <assert.h>

#include <sstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <zmq.h>

#include <DCompute/lru.h>
#include <DCompute/zmqEx.h>
#include <DCompute/util.h>
#include <cex/deltareflection.h>

namespace DCompute {

	class CLRURouter : public detail::TRouterThread<ILRURouter>
	{
	public:
		CLRURouter();

		virtual ~CLRURouter();

		virtual void create();

	protected:

		virtual unsigned int run();

	public:

		static std::shared_ptr<cex::IString> ReciveAddress(void* socket);

		static int SendAddress(void* socket, const char* addr);

		static int SendReady(void* socket);

	};

	CLRURouter::CLRURouter()
	{
	}

	CLRURouter::~CLRURouter()
	{
	}

	void CLRURouter::create()
	{
		_context = zmq_init(1);

		auto address = cex::DeltaInstance<IDComputeConfig>().getJoberAddress();

		std::ostringstream oss;

		oss << "tcp://" << address << ":" << DCOMPUTE_JOB_CLIENT_PORT;
		_frontend = zmq_socket (_context, ZMQ_ROUTER);
		int rc = zmq_bind (_frontend, oss.str().c_str());
		assert(rc==0);

		oss.clear();
		oss.str("");
		oss << "tcp://" << address << ":" << DCOMPUTE_JOB_WORKER_PORT;
		_backend = zmq_socket (_context, ZMQ_ROUTER);
		rc = zmq_bind (_backend, oss.str().c_str());
		assert(rc==0);
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

	std::shared_ptr<cex::IString> LRURouterMethod::ReciveAddress(void* socket)
	{
		auto addr = cex::DeltaCreateRef<cex::IString>();

		char worker_addr[30];
		int size = zmq_recv (socket, worker_addr, 100, 0);
		if (size<=0)
		{
			return nullptr;
		}
		//assert(size>0 && size<30);
		addr->assign(worker_addr, size);

		char empty[2];
		int empty_size = zmq_recv(socket, empty, 2, 0);
		assert (empty_size == 0);

		return addr;
	}

	int LRURouterMethod::SendAddress(void* socket, const char* addr, int size)
	{
		int rc = zmq_send(socket,addr, size, ZMQ_SNDMORE);
		assert(rc>=0);
		rc = zmq_send (socket, "", 0, ZMQ_SNDMORE);
		assert(rc>=0);

		return 0;
	}

	int LRURouterMethod::SendReady(void* socket)
	{
		int rc = zmq_send(socket, "READY", 5, ZMQ_SNDMORE);
		assert(rc>=0);
		rc = zmq_send (socket, "", 0, 0);
		assert(rc>=0);

		return 0;
	}

	//#define DEQUEUE(q) memmove (&(q)[0], &(q)[1], sizeof (q) - sizeof (q [0]))

	unsigned int CLRURouter::run()
	{
		zmq_msg_t msg;
		int rc = zmq_msg_init(&msg);
		if (rc != 0)
			return -1;

		zmq_pollitem_t items [] = {
			{ _backend, 0, ZMQ_POLLIN, 0 },
			{ _frontend,  0, ZMQ_POLLIN, 0 }
		};

		std::queue<std::string> worker_queue;

		while (!_done) 
		{
			rc = zmq_poll (&items[0], 2, -1);
			if (rc < 0)
			{
				if (_done) return 0;
				assert(false);
				return -1;
			}

			if (items [0].revents & ZMQ_POLLIN) 
			{
				std::shared_ptr<cex::IString> worker_addr = LRURouterMethod::ReciveAddress(_backend);
				worker_queue.push(std::string(worker_addr->data(),worker_addr->length())); 

				std::shared_ptr<cex::IString> client_addr = LRURouterMethod::ReciveAddress(_backend);

				if ( client_addr->compare("READY") != 0 ) 
				{
					LRURouterMethod::SendAddress(_frontend,  client_addr->data(), client_addr->length());

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

				LRURouterMethod::SendAddress(_backend,  worker_queue.front().data(), worker_queue.front().length());

				rc = detail::ForwardMessage(_frontend, _backend, msg);
				if (rc==-1) return rc;

				worker_queue.pop();
			}
		}

		return 0;
	}

	REGIST_DELTA_CREATOR(ILRURouter, CLRURouter);
}