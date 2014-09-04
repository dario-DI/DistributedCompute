// ClientTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _LIB_PATH_ "F:\\SvnHome\\Common\\CommonLib"
#include <DCompute/DCompute_Includes.h>
#include <DCompute/task.h>
#include <DCompute/util.h>

#include "..\dataContract.h"

#define TASKSIZE 10

using namespace DCompute;

int _tmain(int argc, _TCHAR* argv[])
{
	int cpu = Util::DetectNumberOfProcessor();
	printf("cpu:%d.\n", cpu);

	size_t workerSize = Util::GetWorkerSize();
	printf("worker size:%d.\n", workerSize);

	MyTask1 tasks[TASKSIZE];

	DoMultiTask(tasks, TASKSIZE, 1000);

	int allResult=0;
	for (int i=0;i<TASKSIZE;++i)
	{
		allResult+=tasks[i].result;
	}

	printf("reduce:%d", allResult);

	assert(allResult==2*TASKSIZE);

	Sleep(1000000);

	return 0;
}

