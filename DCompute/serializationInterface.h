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

/// ���������л�
#define GF_DECL_SERIALIZABLE(T) \
	template<class Archive, const unsigned int N>friend \
	void GF_serializer(Archive&, T&, const typename GF_version<N>&);

///// ������ǰ���л��汾��
//#define GF_SERIALIZE_VERSION(T, N) \
//	BOOST_CLASS_EXPORT(T)\
//	BOOST_CLASS_VERSION(T,N)\
//	template<class Archive>\
//	void serialize(Archive & ar, T & g, const unsigned int version){\
//	GF_QerySerializerAtVersion<N<99, Archive, T>(ar,g,version);	} 

/// ��汾������
#define GF_CLASS_VERSION(T, N) \
	BOOST_CLASS_VERSION(T,N)

/// ��汾������.
#define GF_CLASS_VERSION2(T, N) \
	BOOST_CLASS_EXPORT(T)\
	BOOST_CLASS_VERSION(T,N)

/// ������ǰ���л��汾��
#define GF_SERIALIZE_VERSION(T, N) \
	template<class Archive>\
	void serialize(Archive & ar, T & g, const unsigned int version){\
	GF_QerySerializerAtVersion<N<99, Archive, T>(ar,g,version);	} 

/// ��ǰ�汾ʵ�����л���ʼ����
#define GF_BEGIN_SERIALIZE_IMPL_NOW(T, N) \
	GF_DEFAULT_SERIALIZER(T) \
	GF_SERIALIZE_VERSION(T, N) \
	template<class Archive>\
	void GF_serializer(Archive & ar, T& g, const GF_version<N>& v ){

/// ��ʷ�汾ʵ�����л���ʼ����
#define GF_BEGIN_SERIALIZE_IMPL_OLD(T, N) \
	template<class Archive>\
	void GF_serializer(Archive & ar, T& g, const GF_version<N>& v ){

/// ʵ�����л���������
#define GF_END_SERIALIZE_IMPL }

/// �����ݴ����л�
//#define GF_SERIALIZE_ERROR_TOLERANT

/// ���л�����
#ifndef GF_SERIALIZE_ERROR_TOLERANT
#define GF_SERIALIZE_BASE(B) ar&boost::serialization::base_object<B>(g);
#else
#define GF_SERIALIZE_BASE(B) \
	try { ar&boost::serialization::base_object<B>(g); }\
	catch(std::exception e){ assert(false);TRACE1("%s", e.what()); }
#endif

/// ���л���Ա����
#ifndef GF_SERIALIZE_ERROR_TOLERANT
#define GF_SERIALIZE_MEMBER(M) ar&g.M;
#else
#define GF_SERIALIZE_MEMBER(M) \
	try { ar&g.M; }catch(std::exception e){ assert(false);TRACE1("%s", e.what()); }
#endif

/// ���л�����������
#define GF_SERIALIZE_BINARY(M, size) ar&boost::serialization::make_binary_object(g.M, size);

/// ����汾��
template<const unsigned int N>
struct GF_version
{
	static const unsigned int value=N;
	//enum {value=N}; //if don't allow in-class initialization of static integral constant members

};

///// ���������л�ģ��
//template<class Archive, class T, const unsigned int N>
//void GF_serializer(Archive & ar, T& g, const typename GF_version<N>& v )
//{
//	unsigned int version =N;assert(false); // no such version of serialization
//}

#define GF_DEFAULT_SERIALIZER(T) \
template<class Archive, const unsigned int N>\
void GF_serializer(Archive & ar, T& g, const typename GF_version<N>& v ) {\
	unsigned int version =N;assert(false); }

/// �汾case��
#define GF_SERIALIZE_VERSION_CASE(N) \
	case N: GF_serializer(ar, g, GF_version<N>());break;

/// �汾��0~99��case���õ�ʵ��
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

/// �汾��0~999��case���õ�ʵ��
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

//BOOST_SERIALIZATION_SPLIT_MEMBER() /// �����л���Ϊsave/load.����ʽ
//BOOST_SERIALIZATION_SPLIT_FREE(class) /// �����л���Ϊsave/load. ������ʽ
//if (ar::is_loading) /// if you do not want split

#undef GF_SERIALIZE_VERSION_CASE

#endif
