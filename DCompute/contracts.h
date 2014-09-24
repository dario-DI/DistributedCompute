/// \file contracts.h Copyright (C).
/// \brief data contract
///
///
/// \note:
/// \author: DI
/// \time: 2012/9/3 16:25
#pragma once

#ifndef _DCOMPUTE_CONTRACTS_H_
#define _DCOMPUTE_CONTRACTS_H_

#include <cex/DeltaReflection.h>
#include <cex/String.h>
#include <DCompute/config>
#include <DCompute/typeWrappers.h>

namespace DCompute 
{
	namespace Contract
	{
		class WorkerInfo : public cex::Interface
		{
		public:
			WorkerInfo();

			enum RegistType
			{
				regist=0, 
				unregist,
				getWorkerNumber
			};

			RegistType registType;
			int result;
			std::string id;


			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & registType;
				ar & id;
			}

			void Do();
		};
	}

	
}

#endif