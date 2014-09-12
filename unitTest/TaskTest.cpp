// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <fstream>

#include <zmq.h>
#include <DCompute/zmqEx.h>
#include <DCompute/joberServer.h>
#include <DCompute/worker.h>
#include <DCompute/client.h>
#include <DCompute/task.h>

using namespace DCompute;

class Task1
{
public:
	Task1()
	{
		result=0;
		book = "you are ok, hello!";
	}

	GF_DECL_SERIALIZABLE(Task1)

	int result;

	String book;

	void Do()
	{
		int* kk=new int;
		delete kk;

		for(size_t i=0; i<book.length(); ++i)
		{
			if (book.at(i)=='e')
			{
				++result;
			}
		}

		printf("result:%d.\n", result);
	}
};

GF_BEGIN_SERIALIZE_IMPL_NOW(Task1, 1)
//GF_SERIALIZE_BASE(BaseS)
GF_SERIALIZE_MEMBER(book)
GF_END_SERIALIZE_IMPL

GF_CLASS_VERSION(Task1, 1)
GF_TYPE_REFLECTION_DCTASK(Task1)

using namespace DCompute;

#define WORKERSIZE 4
#define TASKSIZE 40

CEX_TEST(TaskTest)
{
	int cpu = Util::DetectNumberOfProcessor();

	CJoberServer server;
	server.create();
	server.start();

	{
		CWorker worker[WORKERSIZE];

		for (int i=0; i<WORKERSIZE; ++i)
		{
			worker[i].id = i;
			worker[i].create();
			worker[i].start();
		}

		size_t workerSize = Util::GetWorkerSize();
		assert(workerSize==WORKERSIZE);

		Task1 tasks[TASKSIZE];

		DoMultiTask(tasks, TASKSIZE, 5000);

		int allResult=0;
		for (int i=0;i<TASKSIZE;++i)
		{
			allResult+=tasks[i].result;
		}

		printf("result reduced: %d.\n", allResult);

		assert(allResult==2*TASKSIZE);

		for (int i=0; i<WORKERSIZE; ++i)
		{
			worker[i].stop();
		}
	}

	Sleep(3000);

	//Sleep(1000000000);

	server.stop();
}