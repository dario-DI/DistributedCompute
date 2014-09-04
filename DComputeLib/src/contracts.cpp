#include "stdafx.h"
#include <assert.h>

#include <DCompute/contracts.h>
#include <DCompute/typeWrappers.h>
#include <DCompute/joberServer.h>

GF_TYPE_REFLECTION_DCTASK(DCompute::Contract::WorkerInfo);

namespace DCompute { namespace Contract {

	WorkerInfo::WorkerInfo() : registType(regist), result(0)
	{
	}

	WorkerInfo::~WorkerInfo()
	{
	}

	void WorkerInfo::Do( void )
	{
		switch (registType)
		{
		case regist:
			{
				std::vector<String>& workers=CWokerCounter::Instance()->workers;
				workers.push_back(id);
				result = workers.size();
				printf("regist worker: %s.\n", id.data());
			}
			break;
		case unregist:
			{
				std::vector<String>& workers=CWokerCounter::Instance()->workers;
				std::vector<String>::iterator itr = std::find(workers.begin(), workers.end(), id);
				if (itr != workers.end() )
				{
					workers.erase(itr);
					printf("unregist worker: %s.\n", id.data());
				}
				result = workers.size();
			}
			break;
		case getWorkerNumber:
			{
				result = (int)CWokerCounter::Instance()->workers.size();
			}
			break;
		default:
			assert(false);
			break;
		}
	}

} }