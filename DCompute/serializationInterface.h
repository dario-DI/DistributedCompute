/// \file serializationInterface.h Copyright (C).
/// \brief brief-description
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/17 10:10
#pragma once

#ifndef _DCOMPUTE_SERIALIZATIONINTERFACE_H_
#define _DCOMPUTE_SERIALIZATIONINTERFACE_H_

#pragma warning(disable:4267)

#include <assert.h>

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include <cex/MetaProgram.hpp>

/// 声明能序列化
#define GF_DECL_SERIALIZABLE(T) \
	template<class Archive, const unsigned int N>friend \
	void GF_serializer(Archive&, T&, const typename GF_version<N>&);

///// 声明当前序列化版本号
//#define GF_SERIALIZE_VERSION(T, N) \
//	BOOST_CLASS_EXPORT(T)\
//	BOOST_CLASS_VERSION(T,N)\
//	template<class Archive>\
//	void serialize(Archive & ar, T & g, const unsigned int version){\
//	GF_QerySerializerAtVersion<N<99, Archive, T>(ar,g,version);	} 

/// 类版本号声明
#define GF_CLASS_VERSION(T, N) \
	BOOST_CLASS_VERSION(T,N)

/// 类版本号声明.
#define GF_CLASS_VERSION2(T, N) \
	BOOST_CLASS_EXPORT(T)\
	BOOST_CLASS_VERSION(T,N)

/// 声明当前序列化版本号
#define GF_SERIALIZE_VERSION(T, N) \
	template<class Archive>\
	void serialize(Archive & ar, T & g, const unsigned int version){\
	GF_QerySerializerAtVersion<N<99, Archive, T>(ar,g,version);	} 

/// 当前版本实现序列化开始代码
#define GF_BEGIN_SERIALIZE_IMPL_NOW(T, N) \
	GF_DEFAULT_SERIALIZER(T) \
	GF_SERIALIZE_VERSION(T, N) \
	template<class Archive>\
	void GF_serializer(Archive & ar, T& g, const GF_version<N>& v ){

/// 历史版本实现序列化开始代码
#define GF_BEGIN_SERIALIZE_IMPL_OLD(T, N) \
	template<class Archive>\
	void GF_serializer(Archive & ar, T& g, const GF_version<N>& v ){

/// 实现序列化结束代码
#define GF_END_SERIALIZE_IMPL }

/// 允许容错序列化
//#define GF_SERIALIZE_ERROR_TOLERANT

/// 序列化基类
#ifndef GF_SERIALIZE_ERROR_TOLERANT
#define GF_SERIALIZE_BASE(B) ar&boost::serialization::base_object<B>(g);
#else
#define GF_SERIALIZE_BASE(B) \
	try { ar&boost::serialization::base_object<B>(g); }\
	catch(std::exception e){ assert(false);TRACE1("%s", e.what()); }
#endif

/// 序列化成员变量
#ifndef GF_SERIALIZE_ERROR_TOLERANT
#define GF_SERIALIZE_MEMBER(M) ar&g.M;
#else
#define GF_SERIALIZE_MEMBER(M) \
	try { ar&g.M; }catch(std::exception e){ assert(false);TRACE1("%s", e.what()); }
#endif

/// 序列化二进制数据
#define GF_SERIALIZE_BINARY(M, size) ar&boost::serialization::make_binary_object(g.M, size);

/// 抽象版本号
template<const unsigned int N>
struct GF_version
{
	static const unsigned int value=N;
	//enum {value=N}; //if don't allow in-class initialization of static integral constant members

};

///// 任意类序列化模板
//template<class Archive, class T, const unsigned int N>
//void GF_serializer(Archive & ar, T& g, const typename GF_version<N>& v )
//{
//	unsigned int version =N;assert(false); // no such version of serialization
//}

#define GF_DEFAULT_SERIALIZER(T) \
template<class Archive, const unsigned int N>\
void GF_serializer(Archive & ar, T& g, const typename GF_version<N>& v ) {\
	unsigned int version =N;assert(false); }

/// 版本case宏
#define GF_SERIALIZE_VERSION_CASE(N) \
	case N: GF_serializer(ar, g, GF_version<N>());break;

/// 版本在0~99内case调用的实现
template<bool IsIn100, class Archive, class T>
struct GF_QerySerializerAtVersion
{
	GF_QerySerializerAtVersion(Archive & ar, T& g, const unsigned int version)
	{
		switch(version)
		{
			MT_EXPRESSION_IMPL_0_99(GF_SERIALIZE_VERSION_CASE)
		default:
			assert(version<=99); // max version supported
		}
	}
};

/// 版本在0~999内case调用的实现
template<class Archive, class T>
struct GF_QerySerializerAtVersion<false, Archive, T>
{
	GF_QerySerializerAtVersion(Archive & ar, T& g, const unsigned int version)
	{
		switch(version)
		{
			MT_EXPRESSION_IMPL_0_999(GF_SERIALIZE_VERSION_CASE)
		default:
			assert(version<=999); // max version supported
		}
	}
};

//BOOST_SERIALIZATION_SPLIT_MEMBER() /// 将序列化分为save/load.侵入式
//BOOST_SERIALIZATION_SPLIT_FREE(class) /// 将序列化分为save/load. 非侵入式
//if (ar::is_loading) /// if you do not want split

#undef GF_SERIALIZE_VERSION_CASE

#endif
