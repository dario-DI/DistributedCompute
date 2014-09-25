/// \file UnTest.h Copyright (C)
/// \brief 
///  simple unit testing framework
///
/// \note:
/// \author: DI
#ifndef _CEX_UNTEST_H_
#define _CEX_UNTEST_H_

#pragma region example_usage
/*
*/
#pragma endregion


#pragma once
#include <cex/DeltaReflection.h>

namespace cex
{
	class IUnitTestMethodRegister : public Interface
	{
	public:
		typedef void (*TestFun)();

		virtual void addCase(const std::string& caseName, const TestFun& fun)=0;

		virtual void run()=0;

		virtual void run(const std::string& caseName)=0;

		virtual void run(size_t index)=0;
	};


	CEX_API IUnitTestMethodRegister& __stdcall getOrCreateUnitTestRegIns();

	#define UnitTestRegisterIns (cex::getOrCreateUnitTestRegIns())

	class UnitTestCaseRegProxy
	{
	public:
		UnitTestCaseRegProxy(const std::string& caseName, const IUnitTestMethodRegister::TestFun& fun)
		{
			UnitTestRegisterIns.addCase(caseName, fun);
		}
	};

#define CEX_RUN_ALL_TESTS() \
	UnitTestRegisterIns.run();

#define CEX_RUN_TEST(caseNameOrIndex) \
	UnitTestRegisterIns.run(caseNameOrIndex);

#define CEX_TEST(caseName) \
	void ctx##caseName(); \
	static cex::UnitTestCaseRegProxy ctsProxy_##caseName(#caseName, &ctx##caseName);\
	void ctx##caseName()

#define CEX_TEST_OFF(caseName) static void MT_SEQUENCE_NAME()

#define CEX_ASSERT_TRUE(condition) assert(condition==true)

#define CEX_ASSERT_FALSE(condition) assert(condition==false)

#define CEX_EXPECT_EQ(v1, v2) \
	if (v1 != v2) {\
		std::cout<<"failed! Expect: "<< v1 <<", Actual: "<< v2 << std::endl;	}

#define CEX_EXCEPTION_TRUE(context) \
	try {context;} catch(...) { \
	std::count<<"failed! expecting true, but catch exception: " << #context << std::endl; }

#define CEX_EXCEPTION_FALSE(context) \
	try {context; std::count<<"failed! expecting exception, but not: " << #context << std::endl;} \
	catch(...) {}

}

#endif