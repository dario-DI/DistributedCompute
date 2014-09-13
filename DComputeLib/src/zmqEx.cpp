
#include <assert.h>

#include <zmq.h>
#include <DCompute/zmqEx.h>
#include <DCompute/util.h>
#include <io.h>

#ifdef ENABLE_MSG_ASSERT
#define MSG_ASSERT(condition) MsgAssert(condition)
#else
#define MSG_ASSERT(condition) assert(condition)
#endif



namespace DCompute {

	static void MsgAssert(bool condition)
	{
		assert(condition);
		if ( !condition )
		{
			int error_code = zmq_errno();
			printf("error:%d", error_code);
		}
	}

	std::shared_ptr<cex::IString> ZmqEx::Recv(void *socket, int flags)
	{
		// read message length
		size_t len;
		int size = zmq_recv (socket, &len, sizeof(len), flags);	
		if (size==-1) return nullptr;
		MSG_ASSERT(size==sizeof(len));
	
		// read message
		char* buf = new char[len+1];
		buf[len] = 0;

		size = zmq_recv (socket, buf, len, 0);
		assert(size==len);

		//if(0)
		//{
		//	int64_t more;
		//	size_t more_size = sizeof(more);
		//	zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
		//	assert(!more);
		//}

		std::shared_ptr<cex::IString> revStr = cex::DeltaCreateRef<cex::IString>();
		revStr->assign(buf);

		delete buf;

		return revStr;
	}

	bool ZmqEx::Recv2File(void *socket, const char* filename, int flags)
	{
		size_t length;
		int recvSize = zmq_recv (socket, &length, sizeof(length), flags);
		if (recvSize==-1) return false; // no wait mode
		MSG_ASSERT(recvSize==sizeof(length));

		FILE* file = nullptr;
		fopen_s(&file, filename, "wb");
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

		return true;
	}

	int ZmqEx::Send (void *socket, const char* data, size_t len) 
	{
		// send message size first
		int size = zmq_send (socket, &len, sizeof(len), ZMQ_SNDMORE);
		if ( size<0 ) return size;
		MSG_ASSERT(size==sizeof(len));

		// send message
		size = zmq_send (socket, data, len, 0);

		return size;
	}

	bool ZmqEx::SendFile(void* socket, const char* filename)
	{
		FILE* file;
		fopen_s(&file, filename, "rb");
		if ( file == NULL )
		{
			assert(false);
			return false;
		}

		size_t length = _filelength( _fileno(file) );

		int sendSize = zmq_send (socket, &length, sizeof(length), ZMQ_SNDMORE);
		assert(sendSize==sizeof(length));

		char pbuffer[ZMQ_SEND_ONCE_MAX];
		size_t nReadSize=0;

		while ( !feof(file) )
		{
			nReadSize = fread(pbuffer, 1, ZMQ_SEND_ONCE_MAX, file);

			if ( nReadSize < ZMQ_SEND_ONCE_MAX ) break;

			sendSize = zmq_send (socket, pbuffer, nReadSize, ZMQ_SNDMORE);
			assert(sendSize==nReadSize);
		}

		sendSize = zmq_send (socket, pbuffer, nReadSize, 0);
		assert(sendSize==nReadSize);

		fclose(file);

		return true;
	}

	static int SendLarge(void* socket, char* data, size_t len)
	{
		// send message size first
		int sendSize = zmq_send (socket, &len, sizeof(len), ZMQ_SNDMORE);
		assert(sendSize==sizeof(len));

		char* pData = data;
		size_t nMessageLen = len;

		// send multi-part messages
		while (nMessageLen-ZMQ_SEND_ONCE_MAX>0)
		{
			int nOnceSendSize = ZMQ_SEND_ONCE_MAX;

			// send message
			sendSize = zmq_send (socket, pData, ZMQ_SEND_ONCE_MAX, ZMQ_SNDMORE);
			assert(sendSize==ZMQ_SEND_ONCE_MAX);

			nMessageLen -= ZMQ_SEND_ONCE_MAX;
			pData = pData + ZMQ_SEND_ONCE_MAX;
		}

		// send the last part message
		sendSize = zmq_send (socket, pData, nMessageLen, 0);
		assert(sendSize==nMessageLen);

		return sendSize;
	}

}