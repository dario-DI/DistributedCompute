// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <fstream>

#include <DCompute/typeWrappers.h>
#include <DCompute/string.h>

class MyTask1
{
public:
	MyTask1()
	{
		result=0;
		book = "you are ok, hello!";
	}

	GF_DECL_SERIALIZABLE(MyTask1)

	int result;

	DCompute::String book;

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


GF_BEGIN_SERIALIZE_IMPL_NOW(MyTask1, 1)
	//GF_SERIALIZE_BASE(BaseS)
	GF_SERIALIZE_MEMBER(book)
GF_END_SERIALIZE_IMPL

GF_CLASS_VERSION(MyTask1, 1)
GF_TYPE_REFLECTION_DCTASK(MyTask1)
