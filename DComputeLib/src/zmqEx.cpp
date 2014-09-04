#include "stdafx.h"
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

	int ZmqEx::Recv(void *socket, msgType& msg, int flags)
	{
		// read message length
		size_t len;
		int size = zmq_recv (socket, &len, sizeof(len), flags);	
		if (size==-1) return -1;
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

		msg=buf;

		return (int)len;
	}

	int ZmqEx::Recv(void *socket, String& msg, int flags)
	{
		char* buf;
		int size = Recv(socket, buf, flags);
		
		msg.attach(buf, size);

		return size;
	}

	void ZmqEx::Free( msgType& msg)
	{
		delete msg;
		msg=NULL;
	}

	bool ZmqEx::RecvFile(void *socket, const String& fileName, int flags)
	{
		size_t length;
		int recvSize = zmq_recv (socket, &length, sizeof(length), flags);
		if (recvSize==-1) return false; // no wait mode
		MSG_ASSERT(recvSize==sizeof(length));

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

	int ZmqEx::Send (void *socket, const String& data) 
	{
		return Send(socket, data.data(), data.length());
	}

	bool ZmqEx::SendFile(void* socket, const String& filename)
	{
		FILE* file = fopen(filename.data(), "rb");
		if ( file == NULL ) return false;

		size_t length = filelength( fileno(file) );

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