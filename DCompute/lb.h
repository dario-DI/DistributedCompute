/// \file lb.h Copyright (C).
/// \brief brief-description
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/31 17:21
#pragma once

#ifndef _DCOMPUTE_LBROUTER_H_
#define _DCOMPUTE_LBROUTER_H_

#include <DCompute/joberServer.h>

namespace DCompute 
{
	class CLBRouter : public CRouterBase
	{
	public:
		CLBRouter();

		virtual ~CLBRouter();

		virtual void create();

		virtual void destory();

		virtual unsigned int run();
	};
}

#endif