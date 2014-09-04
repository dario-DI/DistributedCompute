#include "stdAfx.h"

#include <assert.h>
#include <process.h>

#include <DCompute/thread.h>

namespace DCompute{

	//	class CThreadBase
	CThreadBase::CThreadBase() :_threadHandle(0),_threadID(0),_done(false)
	{
	}

	CThreadBase::~CThreadBase()
	{
		setDone();
		stop();
	}

	void CThreadBase::start(int nPriority)
	{
		if ( isRunning() ) return;

		// Create the thread.
		_threadHandle = (HANDLE)_beginthreadex( NULL, 0/*1024*/, &ThreadFunc, this, 0, &_threadID );
		::SetThreadPriority(_threadHandle, nPriority);
	}

	unsigned __stdcall CThreadBase::ThreadFunc(void* handler)
	{
		CThreadBase* pHandler = (CThreadBase*) handler;

		int result = pHandler->run();

		pHandler->_threadHandle = NULL;

		return result;
	}

	bool CThreadBase::isRunning() const
	{
		return _threadHandle != NULL;

		//DWORD ExitCode;
		//GetExitCodeThread(_threadHandle,&ExitCode);


		//if(ExitCode==STILL_ACTIVE)
		//{
		//	return true; 
		//}
		//else
		//{
		//	return false;
		//}
	}

	bool CThreadBase::stop(DWORD waitMiliseconds)
	{
		if ( _threadHandle != NULL )
		{
			HANDLE hThread = _threadHandle;

			_threadHandle = NULL;

			try
			{
				if (WaitForSingleObject(hThread, waitMiliseconds) != WAIT_OBJECT_0)
				{
					TerminateThread(hThread, -1);
				}

				CloseHandle(hThread);
			}
			catch(...)
			{
				//	exception
				assert(false); //( "CThreadBase Stop unexception" );
			}
		}

		return 0;
	}


	//////////////////////////////////////////////////////////////////////////
	// class CMutex
	//CMutex::CMutex()
	//{
	//	//_handle = CreateMutex(NULL, FALSE, "Mutex_Only_One_Instance_Allowed");
	//	//assert(NULL != _handle);
	//}

	//CMutex::~CMutex()
	//{

	//}

	//void CMutex::lock()
	//{

	//}

	//void CMutex::unLock()
	//{

	//}

}