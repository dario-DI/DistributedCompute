/// \file Delegate.hpp Copyright (C)
/// \brief delegate implement in c++
///
///	Delegate(bool(const string&)) myDelegate;
/// myDelegate += [](const string& v){return true;}
///
/// \note: 
/// \author: DI
/// \time: 2011/7/20 15:33
#pragma once

#include <list>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <cex/MetaProgram.hpp>

namespace cex
{
#define CDelegate(p) Delegate<p>

	/// delegate wrapper
	template<typename singnature>
	class Delegate
	{
	};

	/// delegate base
	template<typename singnature>
	class Delegate_Base
	{
	public:
		Delegate_Base() {}
		~Delegate_Base() {}

		typedef boost::function<singnature> FuncType;
		typedef std::list<FuncType> ImplList;
		typedef typename ImplList::iterator IterType;
		typedef typename ImplList::const_iterator const_IterType;

		size_t size() const
		{
			return impl_list_.size();
		}

		void operator = ( const FuncType& func )
		{
			impl_list_.clear();
			add( func );
		}

		void operator += ( const FuncType& func )
		{
			add( func );
		}

		void operator -= ( const FuncType& func )
		{
			remove( func );
		}

		void add( const FuncType& func )
		{
			if ( ! func.empty()  )
				impl_list_.push_back( func );
		}

		///	针对成员函数使用，不适用一般函数
		void remove( const FuncType& obj )
		{
			IterType itr = impl_list_.begin();

			for ( ; itr != impl_list_.end(); ++itr )
			{
				if ( GetObjPtr(*itr) == GetObjPtr(obj) )
				{
					impl_list_.erase( itr );
					return;
				}
			}

			assert(false);	//	Never find Object pointer, or it is a function pointer
		}

		///	针对成员函数使用，不适用一般函数
		static void* GetObjPtr(const FuncType& obj)
		{
			FuncType* f = (FuncType*)(obj.functor.obj_ptr);
			return f->functor.obj_ptr;
		}

	protected:

		ImplList impl_list_;
	};

/// Delegate参数为n的实现
#define MT_DELEGATE_IMPL( N ) \
	template<typename R MT_COMMA(N) MT_TPARAM(N) > \
	class Delegate<R( MT_TARG(N) )> : public Delegate_Base<R( MT_TARG(N) )>\
	{\
	public:\
		void operator= ( const FuncType& func )\
		{\
			impl_list_.clear();\
			if ( func != NULL ) impl_list_.push_back( func );\
		}\
		void operator() ( MT_PARAM(N) ) const\
		{\
			const_IterType itr = impl_list_.begin();\
			for ( ; itr != impl_list_.end(); ++itr )\
			{\
				(*itr)( MT_ARG(N) );\
			}\
		}\
	};

	/// Delegate实现
	MT_EXPRESSION_IMPL_0_7( MT_DELEGATE_IMPL )

}