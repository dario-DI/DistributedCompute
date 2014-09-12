// DComputeLib.cpp : Defines the entry point for the DLL application.
//


#include <assert.h>
#include <string>

#include <DCompute/typeReflection.h>
#include <DCompute/typeWrappers.h>

namespace DCompute {

////////////////class TypeReflectorRegistry Impelement//////////////////////////
//
class TypeReflectorRegistry
{
public:
	typedef std::string KeyType;

	static TypeReflectorRegistry* instance(void);

	~TypeReflectorRegistry();

	volatile void addFactory( const KeyType& key, ITypeCreator* factory );

	ITypeCreator* getFactory( const KeyType& key );

public:

	typedef std::map<KeyType, ITypeCreator*> FactoryMap;

	FactoryMap _factoryMap;

private:
	TypeReflectorRegistry();
};

TypeReflectorRegistry* TypeReflectorRegistry::instance()
{
	static TypeReflectorRegistry s_regist;
	return &s_regist;
}

TypeReflectorRegistry::TypeReflectorRegistry()
{

}

TypeReflectorRegistry::~TypeReflectorRegistry()
{
	for (FactoryMap::iterator itr=_factoryMap.begin(); itr!=_factoryMap.end(); ++itr)
	{
		delete itr->second;
		itr->second = NULL;
	}

	_factoryMap.clear();
}

volatile void TypeReflectorRegistry::addFactory( const KeyType& key, ITypeCreator* factory )
{
	if ( factory->GetVersion() != DCOMPUTE_LIB_VERSION )
	{
		assert(false); // 版本已升级，必须重新编译
		return;
	}

#ifdef _DEBUG
	FactoryMap::iterator itr = _factoryMap.find( key );
	if ( itr != _factoryMap.end() )
	{
		assert( false );	//	相同名映射
	}
#endif

	_factoryMap[key] = factory;
}

ITypeCreator* TypeReflectorRegistry::getFactory( const KeyType& key )
{
	FactoryMap::iterator itr = _factoryMap.find( key );
	if ( itr == _factoryMap.end() )
	{
		assert(false); 
		printf("type has no register:%s.\n", key.data());
		return NULL;
	}

	return itr->second;
}

///////////////////////////////////////////////////////
// class TypeReflectorManager
volatile void TypeReflectorManager::AddCreator( const String& key, ITypeCreator* factory )
{
	TypeReflectorRegistry::instance()->addFactory(key.data(), factory);
}

ITypeCreator* TypeReflectorManager::GetCreator( const String& key )
{
	return TypeReflectorRegistry::instance()->getFactory(key.data());
}

IType* TypeReflectorManager::CreateObject( const String& key )
{
	ITypeCreator* creator = GetCreator( key );
	if ( creator == NULL ) return NULL;

	return creator->Create();
}

void TypeReflectorManager::DestoryObject( IType* ptr )
{
	if ( ptr==NULL ) return;
	ITypeCreator* creator = ptr->_creator;
	creator->Destory(ptr);
}

IType* TypeReflectorManager::ReflectString2Object(const String& str)
{
	std::istringstream sin(str.data());
	DSERIALIZER_TYPE ar_in(sin);

	String typeName;
	ar_in >> BOOST_SERIALIZATION_NVP(typeName);

	IType* ptr = TypeReflectorManager::CreateObject( typeName );
	IType_Serializable* serializablePtr = Query<IType_Serializable*>(ptr);

	if ( serializablePtr==NULL ) return NULL;

	serializablePtr->String2Object(str);

	return ptr;
}

IType* TypeReflectorManager::ReflectFile2Object( const String& filename )
{
	using namespace std;

	ifstream fin(filename.data(), ios::in|ios::binary/*|ios::trunc|ios::_Nocreate*/);
	DSERIALIZER_TYPE ar_in(fin);

	String typeName;
	ar_in >> BOOST_SERIALIZATION_NVP(typeName);

	fin.close();

	IType* ptr = TypeReflectorManager::CreateObject( typeName );
	IType_Serializable* serializablePtr = Query<IType_Serializable*>(ptr);

	if ( serializablePtr==NULL ) return NULL;

	serializablePtr->File2Object(filename);

	return ptr;
}

}