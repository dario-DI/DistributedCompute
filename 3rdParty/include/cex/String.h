/// \file string.h Copyright (C).
/// \brief brief-description
///
/// string interface. Can be using in the different  modules.
/// 
/// std::shared_ptr<IString> str = DeltaCreateRef<IString>();
///	const char* data = str->data();
/// 
/// \note:
/// \author: DI
/// \time: 2012/8/23 17:38
#pragma once

#ifndef _CEX_STRING_H_
#define _CEX_STRING_H_

#include <cex/DeltaReflection.h>


namespace cex 
{
	class IString : public Interface
	{
	public:
		virtual const char* assign(const char* data, size_t len)=0;

		virtual const char* assign(const char* data)=0;

		virtual void clear()=0;


		virtual const char* data()const=0;

		virtual const char* c_str()const=0;


		virtual size_t length()=0;

		virtual char at(size_t index)=0;


		virtual void resize(size_t size)=0;


		virtual size_t find(char s)=0;

		virtual size_t find(const char* str)=0;


		virtual int compare(const char* str)=0;

		virtual IString* append(IString* str)=0;

		virtual std::shared_ptr<IString> substr(size_t start, size_t counter)=0;

	};

}

#endif