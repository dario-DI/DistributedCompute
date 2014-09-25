/// \file TimeCheck.h Copyright (C) Sharewin Inc.
/// \brief 耗时统计功能
///
///
/// \note:
/// \author: DI
/// \time: 2010-12-9

#if 0

/**
函数(作用域)整体耗时统计

方法：TIMECHECK, TIMECHECK1

说明：
在TIMECHECK作用域内的函数体,
程序关闭后，TimeCheck.txt 将按耗时多少的先后次序，列表显示所有函数。

注意：耗时统计仅在_DEBUG，且没有定义DISABLE_TIME_CHECK时，有效。
*/

//#define DISABLE_TIME_CHECK // 取消测试宏
//#define TIME_CHECK_ALLWAYS // 总是使用测试宏
#include <cex/TimeCheck.h>

void fun( para )
{
	TIMECHECK;
	
	// your code
	...
}

//也可以在某个作用域内使用，在此作用域总体耗时,也将打印在文件中。

void fun( para )
{
	for ( ... )
	{
		TIMECHECK;

		for ( ... )
		{
			//	为函数名增加标签,以区别于同一个函数中不同的TimeCheck
			TIMECHECK1( -2rdFor );
		}

		// your code
		...
	}	
}

#endif

#pragma once
#include <iosfwd>
#include <string>

#include <vector>
#include <cex/Config>

namespace cex
{
#ifdef TIMECHECK
#error redefined TIMECHECK
#endif

#if (defined(_DEBUG) && !defined(DISABLE_TIME_CHECK)) || defined(TIME_CHECK_ALLWAYS)

#define TIMECHECK \
	static cex::FuncTimeAsm timeCheckObjectX( CString(__FUNCTION__) ); \
	cex::FuncTimeAsmPxy timeCheckObjectXPxy( &timeCheckObjectX ); 
#define TIMECHECK1( name ) \
	static cex::FuncTimeAsm timeCheckObjectX( CString(__FUNCTION__)+CString("_" #name)); \
	cex::FuncTimeAsmPxy timeCheckObjectXPxy( &timeCheckObjectX );

#else

#define TIMECHECK
#define TIMECHECK1( name )

#endif

	class FuncTimeAsmPxy;

	class CEX_IMPORT FuncTimeAsm
	{
	public:
		FuncTimeAsm( const char* name );
		~FuncTimeAsm();
	public:
	
		void WriteFile(std::fstream& ar);

		DWORD GetAssemble() { return _assembleTime; }

		void AddTick(DWORD tick);

	private:
		DWORD _assembleTime;

		DWORD _onceTimeMax;

		DWORD _onceTimeMin;

		DWORD _callNumber;

		char* _name;
	};

	class CEX_IMPORT FuncTimeAsmPxy
	{
	public:
		FuncTimeAsmPxy( FuncTimeAsm* timeAsm );

		~FuncTimeAsmPxy();

	private:
		FuncTimeAsm* _timeAsm;

		DWORD _onceTimeBegin;
	};

	class CEX_IMPORT TimeCheckManage
	{
	public:
		~TimeCheckManage();

		static TimeCheckManage* instance();

		void AddFuncTimeAsm( FuncTimeAsm* timeAsm );

		void WriteFile();

	private:
		TimeCheckManage();

		typedef std::vector<FuncTimeAsm*> FuncAsmList;
		FuncAsmList _vtFuncTimeAsm;

	};
}
