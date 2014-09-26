/// \file zmqEx.h Copyright (C).
/// \brief brief-description
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/20 15:42
#pragma once

#ifndef _ZMQ_EX_H_
#define _ZMQ_EX_H_

#include <memory>
#include <DCompute/config>
#include <cex/string.h>

namespace DCompute 
{
	class DCOMPUTE_EXPORT_CLASS ZmqEx
	{
	public:
		static std::shared_ptr<cex::IString> Recv(void *socket, int flags=0 );

		static bool Recv2File(void *socket, const char* filename, int flags=0);


		static int Send (void *socket, const char* data, size_t len);

		static bool SendFile(void* socket, const char* filename);
	};

}

#endif