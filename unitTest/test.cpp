// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _LIB_PATH_ "F:\\SvnHome\\Common\\CommonLib"
#include <DCompute/DCompute_Includes.h>

extern void StringTest();
extern void ThreadTest();
extern void SerializationTest();
extern void TypeFeflectionTest();
extern void ZmqExTest();
extern void LBTest();
extern void LRUTest();
extern void TaskTest();

int _tmain(int argc, _TCHAR* argv[])
{
	//StringTest();

	//ThreadTest();

	//SerializationTest();

	//TypeFeflectionTest();

	//ZmqExTest();

	//LBTest();

	//LRUTest();

	TaskTest();

	return 0;
}

