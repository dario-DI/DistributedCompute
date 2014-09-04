/*  =====================================================================
    zhelpers.h

    Helper header file for example applications.
    =====================================================================
*/

#ifndef __ZHELPERS_H_INCLUDED__
#define __ZHELPERS_H_INCLUDED__

//  Include a bunch of headers that we will need in the examples

#include <zmq.h>

//#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>

#include <vector>

#define __WINDOWS__
typedef __int64 int64_t;

//  Version checking, and patch up missing constants to match 2.1
#if ZMQ_VERSION_MAJOR == 2
#   if ZMQ_VERSION_MINOR == 0
#       error "Please upgrade to ZeroMQ/2.1 stable for these examples"
#   endif
#elif ZMQ_VERSION_MAJOR == 3
//#   error "Please stick with ZeroMQ/2.1 stable for these examples"
#endif
#ifndef ZMQ_ROUTER
#   define ZMQ_ROUTER ZMQ_ROUTER
#endif
#ifndef ZMQ_DEALER
#   define ZMQ_DEALER ZMQ_DEALER
#endif


//  Provide random number from 0..(num-1)
#if (defined (__WINDOWS__))
#   define randof(num)  (int) ((float) (num) * rand () / (RAND_MAX + 1.0))
#else
#   define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))
#endif


//  Receive 0MQ string from socket and convert into C string
//  Caller must free returned string. Returns NULL if the context
//  is being terminated.
//static char *
//s_recv (void *socket) {
//	const int maxSize=64;
//    char* string = new char[maxSize];
//	int size = zmq_recv (socket, string, maxSize, 0);
//	if ( size == 0 )
//	{
//		delete string;
//		string=NULL;
//		return NULL;
//	}
//
//    string [size] = 0;
//    return (string);
//}

static char*
s_recv (void *socket)
{
	// read message length
	int msgSize;
	int size = zmq_recv (socket, &msgSize, sizeof(msgSize), 0);	
	assert(size==sizeof(msgSize));

	// read message
    char* buf = new char[msgSize+1];

	size = zmq_recv (socket, buf, msgSize, 0);
	assert(size==msgSize);

	buf[msgSize] = 0;

	//if(0)
	//{
	//	int64_t more;
	//	size_t more_size = sizeof(more);
	//	zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
	//	assert(!more);
	//}

	return buf;
}

static bool
s_recv_file (void *socket, char* fileName)
{
	return false;
}

//#define READ_MEMORY_SIZE 128
//static char*
//s_recv (void *socket)
//{
//	//zmq_msg_t message;
//	//zmq_msg_init (&message);
//
//	int totalSize = 128;
//	char* pDump = new char[totalSize];
//	memset(pDump,0,totalSize);
//
//	int currentPos=0;
//
//	char readMemory[READ_MEMORY_SIZE];
//	memset(readMemory, 0, READ_MEMORY_SIZE);
//
//	while (1) 
//	{
//		//  Process all parts of the message
//		int nRessize = zmq_recv (socket, readMemory, READ_MEMORY_SIZE-1, 0);
//		if ( nRessize == 0 )
//		{
//			delete pDump;
//			return NULL;
//		}
//
//		int size = (int)strlen(readMemory);
//
//		if ( currentPos+size >= totalSize )
//		{
//			char* pNewDump = new char[totalSize*2];
//			memccpy(pNewDump, pDump, 0, totalSize );
//			totalSize *= 2;
//			delete [] pDump;
//			pDump = pNewDump;
//		}
//
//		memccpy(pDump, readMemory, currentPos, size);
//		currentPos += size;
//
//		//int more = zmq_msg_get (&message, ZMQ_MORE);
//		int64_t more;
//		size_t more_size = sizeof(more);
//		zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
//
//		//zmq_msg_close (&message);
//		if (!more)
//			break;      //  Last message part
//	}
//
//	return pDump;
//}

//  Convert C string to 0MQ string and send to socket
static int
s_send (void *socket, char* data, int len) 
{
	// send message size first
	int sendSize = zmq_send (socket, &len, sizeof(len), ZMQ_SNDMORE);
	assert(sendSize==sizeof(len));

	// send message
	sendSize = zmq_send (socket, data, len, 0);

	return sendSize;
}

/// 消息一次发送最大字节
#define ZMQ_SEND_ONCE_MAX 32 /*2048*/

static int
s_send_large(void* socket, char* data, size_t len)
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

static int
s_send_file(void* socket, char* filename)
{
	FILE* file = fopen(filename, "rb");
	if ( file == NULL ) return -1;

	return 0;
}


////  Convert C string to 0MQ string and send to socket
//static int
//s_send (void *socket, char *string) {
//    return zmq_send (socket, string, strlen(string), 0);
//}

//  Sends string as 0MQ string, as multipart non-terminal
//static int
//s_sendmore (void *socket, char *data) {
//	int len = strlen(string);
//	int msgLen = sizeof(zmq_msg_t);
//
//	char* pData = data;
//
//	while (len>0)
//	{
//		int sendSize = len-sendSize<0 ? len : msgLen;
//		
//		zmq_msg_t message;
//		zmq_msg_init_size (&message, sendSize);
//		memcpy (zmq_msg_data(&message), pData, sendSize);
//		int rc = zmq_send (socket, &message, sendSize, ZMQ_SNDMORE);
//		zmq_msg_close (&message);
//
//		len -= sendSize;
//		pData = pData + sendSize;
//	}
//
//    return zmq_send (socket, string, strlen(string), ZMQ_SNDMORE);
//}
//
////  Receives all message parts from socket, prints neatly
////
//static char*
//s_dump (void *socket)
//{
//	zmq_msg_t message;
//	zmq_msg_init (&message);
//
//	int totalSize = 128;
//	char* pDump = new char[totalSize];
//	memset(pDump,0,totalSize);
//
//	int currentPos=0;
//
//    while (1) 
//	{
//        //  Process all parts of the message
//        int nRessize = zmq_recv (socket, &message, sizeof(message), 0);
//		if ( nRessize == 0 )
//		{
//			delete pDump;
//			return NULL;
//		}
//
//		int size = (int)zmq_msg_size (&message);
//		char* data = (char*)zmq_msg_data (&message);
//
//		if ( currentPos+size >= totalSize )
//		{
//			char* pNewDump = new char[totalSize*2];
//			memccpy(pNewDump, pDump, 0, totalSize );
//			totalSize *= 2;
//			delete [] pDump;
//			pDump = pNewDump;
//		}
//
//		memccpy(pDump, data, currentPos, size);
//		currentPos += size;
//
//		//int more = zmq_msg_get (&message, ZMQ_MORE);
//		int64_t more;
//		size_t more_size = sizeof(more);
//		zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
//
//        //zmq_msg_close (&message);
//        if (!more)
//            break;      //  Last message part
//    }
//
//	return pDump;
//}

//  Set simple random printable identity on socket
//
static void
s_set_id (void *socket)
{
    char identity [10];
    sprintf (identity, "%04X-%04X", randof (0x10000), randof (0x10000));
    zmq_setsockopt (socket, ZMQ_IDENTITY, identity, strlen (identity));
}


//  Sleep for a number of milliseconds
static void
s_sleep (int msecs)
{
#if (defined (__WINDOWS__))
    Sleep (msecs);
#else
    struct timespec t;
    t.tv_sec  =  msecs / 1000;
    t.tv_nsec = (msecs % 1000) * 1000000;
    nanosleep (&t, NULL);
#endif
}

//  Return current system clock as milliseconds
static int64_t
s_clock (void)
{
#if (defined (__WINDOWS__))
    SYSTEMTIME st;
    GetSystemTime (&st);
    return (int64_t) st.wSecond * 1000 + st.wMilliseconds;
#else
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (int64_t) (tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}

//  Print formatted string to stdout, prefixed by date/time and
//  terminated with a newline.

static void
s_console (const char *format, ...)
{
    time_t curtime = time (NULL);
    struct tm *loctime = localtime (&curtime);
    char *formatted = (char*)malloc (20);
    strftime (formatted, 20, "%y-%m-%d %H:%M:%S ", loctime);
    printf ("%s", formatted);
    free (formatted);

    va_list argptr;
    va_start (argptr, format);
    vprintf (format, argptr);
    va_end (argptr);
    printf ("\n");
}

#endif  //  __ZHELPERS_H_INCLUDED__
