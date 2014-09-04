// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <fstream>

#include <assert.h>
#include <process.h>
#include <DCompute/thread.h>

using namespace DCompute;

class MyThread : public DCompute::CThreadBase
{
public:
	MyThread()
	{
		id=0;
	}

	~MyThread()
	{
	}

	int id;

protected:

	UINT run()
	{
		for (int i=0; i<1000; ++i)
		{
			printf("new :");

			char* kk=new char[1000];
			for (int i=0; i<1000; ++i)
			{
				kk[i]=5;
			}

			printf("deleted \n");

			delete [] kk;
		}
		return 0;
	}
};

#define THREAD_SIZE 20

void ThreadTest()
{
	MyThread thread[THREAD_SIZE];
	for (int i=0; i<THREAD_SIZE; ++i)
	{
		thread[i].start();
	}

	Sleep(1000000);
}