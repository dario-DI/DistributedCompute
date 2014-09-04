// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>

#include <DCompute/string.h>
#include <DCompute/typeReflection.h>
#include <vector>

using namespace DCompute;

void StringTest()
{
	char temp[20] = "hello world!";
	String str0 = temp;
	printf("%s\n", str0.data());

	String str1 = "hello,";
	printf("%s\n", str1.data());

	str1.resize(str1.length());
	assert(str1=="hello,");
	
	str1.resize(0);
	assert(str1.length()==0);

	String str2;
	str2.assign("world!");
	assert(str2.length()==strlen("world!"));
	printf("%s\n", str2.data());

	String str3=str2;
	assert(str3==str2);
	printf("%s\n", str3.data());

	// find
	size_t findPos = str2.find("rld!");
	assert(findPos==2);

	// save
	String str4;
	MakeObject2String(str3, str4);
	printf("%s\n", str4.data());

	// load
	String str5;
	MakeString2Object(str5, str4);
	assert(str5==str3);
	printf("%s\n", str5.data());


	//test vector
	std::vector<String> vtString;
	vtString.push_back(str1);
	vtString.push_back(str2);
	vtString.push_back(str3);
	assert(vtString[0]==str1);
	assert(vtString[1]==str2);
	assert(vtString[2]==str3);

	vtString.clear();
	str1.clear();
	str2.clear();
	str3.clear();

}