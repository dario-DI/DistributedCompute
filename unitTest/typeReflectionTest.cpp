// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <fstream>

#include <DCompute/typeReflection.h>
#include <DCompute/typeWrappers.h>

using namespace DCompute;

class MyType
{
public:
	MyType() : kk(3) {}
	virtual ~MyType() {}

	int kk;
};

GF_TYPE_REFLECTION(MyType)
//static TReflectTypeCreatorRegistProxy< DefaultReflectTypeCreator< DefaultReflectType<MyType> > > s_Reflection("Foo");

namespace aa {

class MyType2 : public MyType
{
public:
	GF_DECL_SERIALIZABLE(MyType2)

	MyType2() : kk2(4) 
	{
		result = 0;
	}

	void Do() { kk2=88; result=88; }

	int kk2;

	int result;
};


GF_BEGIN_SERIALIZE_IMPL_NOW(MyType2, 1)
//GF_SERIALIZE_BASE(BaseS)
GF_SERIALIZE_MEMBER(kk2)
GF_END_SERIALIZE_IMPL

}

GF_CLASS_VERSION(aa::MyType2, 1)
GF_TYPE_REFLECTION_DCTASK(aa::MyType2);

using namespace aa;


void TypeFeflectionTest0()
{
	{
		IType* ptr = TypeReflectorManager::CreateObject( typeid(MyType).name() );

		MyType* v = IType_cast<MyType*>(ptr);
		assert(v!=NULL);

		//MyType* tt=NULL;
		//assert( creator->IsSameType(ptr) );

		TypeReflectorManager::DestoryObject(ptr);
	}

	{
		// Deserialize test

		// save
		std::ofstream fout("MyType2serialization.txt");
		SERIALIZER_TYPE ar_out(fout);

		std::string typeName1( typeid(MyType2).name() );
		ar_out << BOOST_SERIALIZATION_NVP( typeName1 );

		MyType2 type2;
		type2.kk2 = 5;
		ar_out << BOOST_SERIALIZATION_NVP(type2);

		fout.close();

		// load
		std::ifstream fin("MyType2serialization.txt");
		DSERIALIZER_TYPE ar_in(fin);

		String typeName;
		ar_in >> BOOST_SERIALIZATION_NVP(typeName);

		fin.close();

		IType* ptr = TypeReflectorManager::CreateObject( typeName );
		IType_DCTask* taskPtr = Query<IType_DCTask*>(ptr);

		// 测试反序列化
		taskPtr->Serialize(ar_in);

		// 测试返回结果序列化
		{
			String strResult;
			taskPtr->Result2String(strResult);
			std::istringstream result_sin(strResult.data());
			boost::archive::binary_iarchive result_ar_in(result_sin);
			String name;
			result_ar_in >> name;
			int nResult;
			result_ar_in >> BOOST_SERIALIZATION_NVP(nResult);
			assert(nResult==83);
		}

		MyType2* v = IType_cast<MyType2*>(taskPtr);
		assert(v!=NULL);
		assert(v->kk2==5);

		taskPtr->Do();
		assert(v->kk2==88);

		fin.close();

		TypeReflectorManager::DestoryObject(taskPtr);
		
	}

	int kk=0;
	++kk;
}

void TypeFeflectionTest1()
{
	{
		IType* ptr = TypeReflectorManager::CreateObject( typeid(MyType).name() );

		MyType* v = IType_cast<MyType*>(ptr);
		assert(v!=NULL);

		//MyType* tt=NULL;
		//assert( creator->IsSameType(ptr) );

		TypeReflectorManager::DestoryObject(ptr);
	}

	{
		// Deserialize test
		MyType2 type2;
		type2.kk2 = 51;

		// save
		String strOfType2;
		MakeObject2String(type2, strOfType2);

		// load
		MyType2 reflectType2;
		MakeString2Object(reflectType2, strOfType2);
		assert(type2.kk2==reflectType2.kk2);

		IType* ptr = TypeReflectorManager::ReflectString2Object( strOfType2 );
		IType_DCTask* taskPtr = Query<IType_DCTask*>(ptr);
		assert(taskPtr!=NULL);

		MyType2* v = IType_cast<MyType2*>(taskPtr);
		assert(v!=NULL);
		assert(v->kk2==51);

		// 测试返回结果序列化
		{
			String strResult;
			taskPtr->Result2String(strResult);

			int oldResult = v->result;
			v->result = 4223;

			taskPtr->String2Result(strResult);

			assert(v->result==oldResult);
		}

		taskPtr->Do();
		assert(v->kk2==88);

		TypeReflectorManager::DestoryObject(taskPtr);

	}

	int kk=0;
	++kk;
}

void TypeFeflectionTest()
{
	{
		IType* ptr = TypeReflectorManager::CreateObject( typeid(MyType).name() );

		MyType* v = IType_cast<MyType*>(ptr);
		assert(v!=NULL);

		//MyType* tt=NULL;
		//assert( creator->IsSameType(ptr) );

		TypeReflectorManager::DestoryObject(ptr);
	}

	{
		// Deserialize test
		MyType2 type2;
		type2.kk2 = 0;

		// save
		String fileOfType2;
		Util::CreateUniqueTempFile(fileOfType2);
		MakeObject2File(type2, fileOfType2);

		// load
		MyType2 reflectType2;
		MakeFile2Object(reflectType2, fileOfType2);
		assert(type2.kk2==reflectType2.kk2);

		IType* ptr = TypeReflectorManager::ReflectFile2Object( fileOfType2 );
		IType_DCTask* taskPtr = Query<IType_DCTask*>(ptr);
		assert(taskPtr!=NULL);

		MyType2* v = IType_cast<MyType2*>(taskPtr);
		assert(v!=NULL);
		assert(v->kk2==0);

		// 测试返回结果序列化
		{
			String fileResult;
			Util::CreateUniqueTempFile(fileResult);
			taskPtr->Result2File(fileResult);

			int oldResult = v->result;
			v->result = 4223;

			taskPtr->File2Result(fileResult);

			assert(v->result==oldResult);

			Util::DeleteTempFile(fileResult);
		}

		taskPtr->Do();
		assert(v->kk2==88);

		TypeReflectorManager::DestoryObject(taskPtr);

		Util::DeleteTempFile(fileOfType2);

	}

	int kk=0;
	++kk;
}