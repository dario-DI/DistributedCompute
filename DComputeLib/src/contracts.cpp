
#include <assert.h>
#include <string>
#include <DCompute/contracts.h>
#include <DCompute/typeWrappers.h>
#include <DCompute/joberServer.h>

#include <DCompute/serializationInterface.h>

namespace DCompute { namespace Contract {

	WorkerInfo::WorkerInfo() : registType(registType), result(0)
	{
	}

	void WorkerInfo::Do( void )
	{
		switch (registType)
		{
		case regist:
			{
				std::vector<std::string>& workers=detail::CWokerCounter::Instance()->workers;
				workers.push_back(id);
				result = workers.size();
				printf("regist worker: %s.\n", id.data());
			}
			break;
		case unregist:
			{
				std::vector<std::string>& workers=detail::CWokerCounter::Instance()->workers;
				std::vector<std::string>::iterator itr = std::find(workers.begin(), workers.end(), id);
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
				result = (int)detail::CWokerCounter::Instance()->workers.size();
			}
			break;
		default:
			assert(false);
			break;
		}
	}

	//GF_BEGIN_SERIALIZE_IMPL_NOW(WorkerInfo, 1)
	//GF_SERIALIZE_MEMBER(registType)
	//GF_SERIALIZE_MEMBER(id)
	//GF_END_SERIALIZE_IMPL

	REGIST_DELTA_CREATOR(WorkerInfo, TDCTaskProxy<WorkerInfo>);
} 

}

//GF_CLASS_VERSION(DCompute::Contract::WorkerInfo, 1)