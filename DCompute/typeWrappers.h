/// \file typeWrappers.h Copyright (C).
/// \brief brief-description
///
///
/// \note:
/// \author: DI
/// \time: 2012/8/17 10:10
#pragma once

#ifndef _DCOMPUTE_TYPERWAPPERS_H_
#define _DCOMPUTE_TYPERWAPPERS_H_

#include <fstream>
#include <sstream>
#include <cex/String.h>
#include <cex/DeltaReflection.h>
#include <DCompute/serializationInterface.h>


#define GF_REGIST_SERIALIABLE_CREATOR(type) \
	REGIST_DELTA_CREATOR(type, TISerializableProxy<type>)

#define GF_REGIST_DCTASKABLE_CREATOR(type) \
	REGIST_DELTA_CREATOR(type, TDCTaskProxy<type>)

#ifdef _DEBUG
//#define SERIALIZER_TYPE boost::archive::text_oarchive
//#define DSERIALIZER_TYPE boost::archive::text_iarchive
#define SERIALIZER_TYPE boost::archive::binary_oarchive
#define DSERIALIZER_TYPE boost::archive::binary_iarchive
#else
#define SERIALIZER_TYPE boost::archive::binary_oarchive
#define DSERIALIZER_TYPE boost::archive::binary_iarchive
#endif

namespace DCompute 
{
	////////////////////////////////////////////////////////////////////
	/// serialization able interface
	class ISerializable
	{
	public:
		virtual ~ISerializable()=0 {}

		virtual const char* IType_name() const=0;

		virtual bool serializeObj(SERIALIZER_TYPE& ar)=0;


		virtual std::shared_ptr<cex::IString> object2String()=0;

		virtual void string2Object(const char* str)=0;


		virtual void object2File(const char* filename)=0;

		virtual void file2Object(const char* filename)=0;
	};

	template<typename Base>
	class TISerializableProxy : virtual public ISerializable, public Base
	{
	public:
		virtual const char* IType_name() const
		{
			return typeid(Base).name();
		}

		virtual bool serializeObj(SERIALIZER_TYPE& ar)
		{
			ar & BOOST_SERIALIZATION_NVP(getBase());
			return true;
		}

		virtual std::shared_ptr<cex::IString> object2String()
		{
			return detail::MakeObject2String(getBase());
		}

		virtual void string2Object(const char* str)
		{
			detail::MakeString2Object(getBase(), str);
		}

		virtual void object2File(const char* filename)
		{
			detail::MakeObject2File(getBase(), filename);
		}

		virtual void file2Object(const char* filename)
		{
			detail::MakeFile2Object(getBase(), filename);
		}

	protected:
		Base& getBase()
		{
			return *dynamic_cast<Base*>(this);
		}
	};

	////////////////////////////////////////////////////////////////////
	/// 分布式计算任务基类
	class IDCTask
	{
	public:
		virtual ~IDCTask()=0 {}

		virtual void Do()=0;

		virtual std::shared_ptr<cex::IString> result2String()=0;

		virtual void string2Result(const char* str)=0;


		virtual void result2File(const char* filename)=0;

		virtual void file2Result(const char* filename)=0;
	};

	template<typename Base>
	class TDCTaskProxy : virtual public IDCTask, public TISerializableProxy<Base>
	{
	public:
		virtual void Do()
		{
			Base::Do();
		}

		virtual std::shared_ptr<cex::IString> result2String()
		{
			return detail::MakeObject2String(this->result);
		}

		virtual void string2Result(const char* str)
		{
			detail::MakeString2Object(this->result, str);
		}

		virtual void result2File(const char* filename)
		{
			detail::MakeObject2File(this->result, filename);
		}

		virtual void file2Result(const char* filename)
		{
			detail::MakeFile2Object(this->result, filename);
		}
	};

	namespace detail
	{
		/// 映射对象到字符串
		template<typename T>
		std::shared_ptr<cex::IString> MakeObject2String(const T& data)
		{
			std::ostringstream sout;
			//SERIALIZER_TYPE ar_out(sout);
			boost::archive::text_oarchive ar_out(sout);

			std::string typeName( typeid(T).name() );
			ar_out << BOOST_SERIALIZATION_NVP( typeName );

			ar_out << BOOST_SERIALIZATION_NVP(data);

			std::shared_ptr<cex::IString> str = cex::DeltaCreateRef<cex::IString>();
			str->assign(sout.str().data());

			return str;
		}

		/// 从字符串反射对象
		template<typename T>
		void MakeString2Object(T& data, const char* str)
		{
			assert(str!=NULL);

			std::istringstream sin(str);
			//DSERIALIZER_TYPE ar_in(sin);
			boost::archive::text_iarchive ar_in(sin);

			std::string typeName;
			ar_in >> BOOST_SERIALIZATION_NVP(typeName);

			assert( typeName==typeid(T).name() );

			ar_in >> BOOST_SERIALIZATION_NVP(data);
		}

		/// 映射对象到文件
		template<typename T>
		void MakeObject2File(const T& data, const char* filename)
		{
			using namespace std;

			ofstream fout(filename, ios::out|ios::binary/*|ios::trunc|ios::_Nocreate*/);
			SERIALIZER_TYPE ar_out(fout);

			std::string typeName( typeid(T).name() );
			ar_out << BOOST_SERIALIZATION_NVP( typeName );

			ar_out << BOOST_SERIALIZATION_NVP(data);

			fout.close();
		}

		/// 从文件反射对象
		template<typename T>
		void MakeFile2Object(T& data, const char* filename)
		{
			using namespace std;

			ifstream fin(filename, ios::in|ios::binary/*|ios::trunc|ios::_Nocreate*/);
			DSERIALIZER_TYPE ar_in(fin);

			std::string typeName;
			ar_in >> BOOST_SERIALIZATION_NVP(typeName);

			assert( typeName==typeid(T).name() );

			ar_in >> BOOST_SERIALIZATION_NVP(data);

			fin.close();
		}
	}

	inline std::shared_ptr<cex::Interface> ReflectString2Object(const char* str)
	{
		std::istringstream sin(str);
		DSERIALIZER_TYPE ar_in(sin);

		std::string typeName;
		ar_in >> BOOST_SERIALIZATION_NVP(typeName);

		std::shared_ptr<cex::Interface> obj = cex::DeltaCreateRef(typeName.data());
		ISerializable* serializablePtr = cex::DeltaQueryInterface<ISerializable>(obj.get());

		if ( serializablePtr==NULL )
		{
			assert(false);	// the type is not based of ISerializable. 
							// REGIST type using TISerializableProxy<type> of TDCTaskProxy<type>
			return NULL;
		}

		serializablePtr->string2Object(str);

		return obj;
	}

	inline std::shared_ptr<cex::Interface> ReflectFile2Object( const char* filename )
	{
		using namespace std;

		ifstream fin(filename, ios::in|ios::binary/*|ios::trunc|ios::_Nocreate*/);
		DSERIALIZER_TYPE ar_in(fin);

		std::string typeName;
		ar_in >> BOOST_SERIALIZATION_NVP(typeName);

		fin.close();

		std::shared_ptr<cex::Interface> obj = cex::DeltaCreateRef(typeName.data());
		ISerializable* serializablePtr = cex::DeltaQueryInterface<ISerializable>(obj.get());

		if ( serializablePtr==NULL )
		{
			assert(false);	// the type is not based of ISerializable. 
							// REGIST type using TISerializableProxy<type> of TDCTaskProxy<type>
			return NULL;
		}

		serializablePtr->file2Object(filename);

		return obj;
	}

}

#endif