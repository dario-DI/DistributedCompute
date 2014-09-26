/// \file deltaReflection.h Copyright (C)
/// \brief 
///  providing any type reflection by string key, or even by unsigned int key(string hash code).
///  providing easy using interface creating like COM that can be safely using in different modules.
///  providing singleton pattern implementation, which is an interface that mapping its implement.
///
/// \note:
/// \author: DI
/// \time: 2011/9/20 11:02
#ifndef _CEX_DELTAREFLECTION_H_
#define _CEX_DELTAREFLECTION_H_

#pragma region example_usage
/*example usage

// 0 using an interface LIKE COM
class IMyInterface : public Interface
{
public:
	virtual void doSomething()=0;
}

class IMyImplement : public IMyInterface
{
public:
	void doSomething() {}
private:
	std::string str;
}

// register Interface and the implementation in .cpp file
REGIST_DELTA_CREATOR(IMyInterface, IMyImplement)

// how to use
std::shared_ptr<IMyInterface> refPtr = DeltaCreateRef<IMyInterface>();
refPtr->doSomething();

// or using by other method
IMyInterface* obj = DeltaCreate<IMyInterface>();
obj->doSomething();
DeltaDestory(obj);

// 1 reflect a function
{
void Foo(int k)
{
	// do something
}

// register it in .cpp file
REGIST_DELTA( "MyLib", "Foo", &Foo);

// using that function
typedef void (*MethodType)(int);
MethodType func = DeltaCast<MethodType>("MyLib", "Foo");
func(2);

// 2 register a singleton class
class IMyInstance : public Interface
{
public:
virtual void foo()=0;
};

class InstanceImpl : public IMynstance
{
public:
void foo() {}
}

// register it in .cpp file
REGIST_DELTA_INSTANCE(IMyInstance, InstanceImpl)

// using the Instance 
IMyInstance& instance = cex::DeltaInstance<IInstance>();
instance.foo();

*/
#pragma endregion


#pragma once
#include <map>
#include <string>
#include <stdexcept>
#include <vector>
#include <memory>
#include <assert.h>
#include <iostream>
#include <boost/any.hpp>

#include <cex/config>
#include <cex/MetaProgram.hpp>


// ���е�ע��key�����꣬Լ����DELTA_REGKEY_��ͷ������ͳһ�����鿴
// ����Լ�����ֶ�����ַ���key�����Ѵ�д��ʾ��

// ��������ע������
#define DELTA_REGKEY_SYS_INSTANCE "SYS0"

// �����������ڹ���������
#define DELTA_REGKEY_SYS_REGREFPTR "SYS1"

// ���󴴽���ע������
#define DELTA_REGKEY_SYS_TYPE_CREATOR "SYS2"

namespace cex
{
	template<typename KeyType_,
		typename ValueType_,
		typename uniqueType_=int>
	class KeyValueRegister
	{
	public:
		typedef KeyType_ KeyType;
		typedef ValueType_ ValueType;

		typedef std::map<KeyType, ValueType> TYPE_MAP;

		void Regist(const KeyType& key, const ValueType& value)
		{
			_map[key]=value;
		}

		void UnRegist(const KeyType& key)
		{
			TYPE_MAP::iterator itr = _map.begin();
			for ( ; itr != _map.end(); ++itr )
			{
				if ( itr->first == key )
				{
					_map.erase( itr );
					return;
				}
			}
		}

		typename ValueType& Get(const KeyType& key)
		{
			TYPE_MAP::iterator itr = _map.find( key );
			if ( itr != _map.end() )
			{
				return itr->second;
			}

			//assert(false);
			//std::cout << "DeltaReflection.h: KeyValueRegister: invalid key: " << key << std::endl;
			throw( std::invalid_argument( "DeltaReflection.h: KeyValueRegister: invalid key." ) );
		}

		typename const ValueType& Get(const KeyType& key) const
		{
			TYPE_MAP::iterator itr = _map.find( key );
			if ( itr != _map.end() )
			{
				return itr->second;
			}

			//assert(false);
			//std::cout << "DeltaReflection.h: KeyValueRegister: invalid key: " << key << std::endl;
			throw( std::invalid_argument( "DeltaReflection.h: KeyValueRegister: invalid key." ) );
		}

		typename TYPE_MAP& Map() { return _map; }

		typename const TYPE_MAP& Map() const { return _map; }

	private:
		TYPE_MAP _map;
	};

	/// delta register
	struct UniqueTypeofDeltaRegister {};
	typedef KeyValueRegister<size_t, boost::any, UniqueTypeofDeltaRegister> UIntDeltaRegister;
	CEX_API  UIntDeltaRegister& __stdcall UIntDeltaRegisterSingleton();
	typedef KeyValueRegister<std::string, boost::any, UniqueTypeofDeltaRegister> StringDeltaRegister;
	CEX_API StringDeltaRegister& __stdcall StringDeltaRegisterSingleton();

	namespace df 
	{
		// common type(char*, wchar_t*, std::string)
		template<typename T>
		struct TTypeTraits
		{
			typedef std::string KeyType;
			typedef StringDeltaRegister RegType;
			static RegType& Instance() { return StringDeltaRegisterSingleton(); }

			static std::string Convert(const std::string& key)
			{
				return key;
			}

			static std::string Convert(const char* key)
			{
				return std::string(key);
			}

			static std::string Convert(const wchar_t* key)
			{
				std::wstring wstr(key);
				return ws2s(wstr);
			}

			static std::string ws2s(const std::wstring& ws)
			{
				std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
				setlocale(LC_ALL, "chs");
				const wchar_t* _Source = ws.c_str();
				size_t _Dsize = 2 * ws.size() + 1;
				char *_Dest = new char[_Dsize];
				memset(_Dest,0,_Dsize);
				size_t numConverted;
				wcstombs_s(&numConverted, _Dest,_Dsize, _Source,_Dsize);
				std::string result = _Dest;
				delete []_Dest;
				setlocale(LC_ALL, curLocale.c_str());
				return result;
			} 

		};

		// unsigned int type
		template<>
		struct TTypeTraits<size_t>
		{
			typedef size_t KeyType;
			typedef UIntDeltaRegister RegType;
			static RegType& Instance() { return UIntDeltaRegisterSingleton(); }

			static size_t Convert(size_t key)
			{
				return key;
			}
		};

		// int type
		template<>
		struct TTypeTraits<int>
		{
			typedef size_t KeyType;
			typedef UIntDeltaRegister RegType;
			static RegType& Instance() { return UIntDeltaRegisterSingleton(); }

			static size_t Convert(int key)
			{
				return key;
			}
		};

		// other supported key type(compatible with type of size_t or string)
		// ...

		// template cast
		template<typename DeltaType, typename KeyType>
		DeltaType TGetRegValue(
			KeyValueRegister<KeyType, boost::any, UniqueTypeofDeltaRegister>& reg,
			const KeyType& key)
		{
			try
			{
				boost::any& r = reg.Get(key);
				return boost::any_cast<DeltaType>(r);
			}
			catch (...)
			{
				//std::cout << "DeltaReflection.h: TGetRegValue, invalid key: " << key << std::endl;
				throw( std::invalid_argument( "invalid key" ) );
			}
		}

		template<typename DeltaType, typename KeyType>
		DeltaType* TGetRegValuePTR(
			KeyValueRegister<KeyType, boost::any, UniqueTypeofDeltaRegister>& reg,
			const KeyType& key)
		{
			try
			{
				boost::any& r = reg.Get(key);
				return boost::any_cast<DeltaType>(&r);
			}
			catch(...)
			{
				//std::cout << "DeltaReflection.h: TGetRegValue, invalid key: " << key << std::endl;
				throw( std::invalid_argument( "invalid key" ) );
			}
		}
	}

	// get copy value by the given key
	template<typename DeltaType, typename KeyType>
	DeltaType DeltaCast(const KeyType& key)
	{
		return df::TGetRegValue<DeltaType>(df::TTypeTraits<KeyType>::Instance(), 
			df::TTypeTraits<KeyType>::Convert(key));
	}

	// get pointer of value by the given key
	template<typename DeltaType, typename KeyType>
	DeltaType* DeltaPTRCast(const KeyType& key)
	{
		return df::TGetRegValuePTR<DeltaType>(df::TTypeTraits<KeyType>::Instance(),
			df::TTypeTraits<KeyType>::Convert(key));
	}

	template<typename DeltaType, typename libKeyType, typename valueKeyType>
	DeltaType DeltaCast(const libKeyType& libKey, const valueKeyType& valueKey)
	{
		typedef df::TTypeTraits<valueKeyType>::RegType RegType;

		try
		{
			RegType* valueReg = DeltaPTRCast<RegType>(libKey);

			if (valueReg!=NULL)
			{
				return df::TGetRegValue<DeltaType>(*valueReg, 
					df::TTypeTraits<valueKeyType>::Convert(valueKey));
			}
			else
			{
				throw( std::invalid_argument( "invalid lib key." ) );
			}			
		}
		catch(std::exception e)
		{
			std::cout << "DeltaReflection.h: DeltaCast, invalid lib key: " << libKey << ", value key: "<< valueKey << std::endl;
			throw(std::invalid_argument( "invalid lib key." ));
		}
	}

	template<typename DeltaType, typename libKeyType, typename valueKeyType>
	DeltaType* DeltaPTRCast(const libKeyType& libKey, const valueKeyType& valueKey)
	{
		typedef df::TTypeTraits<valueKeyType>::RegType RegType;

		try
		{
			RegType* valueReg = DeltaPTRCast<RegType>(libKey);

			if (valueReg!=NULL)
			{
				return df::TGetRegValuePTR<DeltaType>(*valueReg,
					df::TTypeTraits<valueKeyType>::Convert(valueKey));
			}
			else
			{
				throw( std::invalid_argument( "invalid lib key." ) );
			}			
		}
		catch(std::exception e)
		{
			std::cout << "DeltaReflection.h: DeltaCast, invalid lib key: " << libKey << ", value key: "<< valueKey << std::endl;
			throw( std::invalid_argument( "invalid lib key." ) );
		}
		
	}

	namespace reg
	{
		template<typename DeltaType, typename KeyType>
		void Register(const KeyType& key, const DeltaType& value)
		{
			typedef df::TTypeTraits<KeyType> Traits;
#ifdef _DEBUG
			try
			{
				Traits::Instance().Get(Traits::Convert(key));
				assert(false); // the same named value has already registered.
				return;
			}
			catch(std::exception e)
			{
				// normal
			}
#endif
			Traits::Instance().Regist(Traits::Convert(key), value);
		}

		template<typename KeyType>
		void UnRegister(const KeyType& key)
		{
			typedef df::TTypeTraits<KeyType> Traits;
			Traits::Instance().UnRegist(Traits::Convert(key));
		}

		template<typename DeltaType, typename libKeyType, typename valueKeyType>
		void Register(const libKeyType& libKey, const valueKeyType& valueKey, const DeltaType& value)
		{
			typedef df::TTypeTraits<libKeyType> LibTraits;
			typedef df::TTypeTraits<valueKeyType> ValueTraits;

			// find library register
			ValueTraits::RegType* reg = NULL;
			try
			{
				reg = DeltaPTRCast<ValueTraits::RegType>(LibTraits::Convert(libKey));
			}
			catch(std::exception e)
			{
				// no register for this lib key, just register
				LibTraits::Instance().Regist(LibTraits::Convert(libKey), ValueTraits::RegType());
				reg = DeltaPTRCast<ValueTraits::RegType>(LibTraits::Convert(libKey));
			}
			
			assert(reg != NULL);

			// register value to the lib register
#ifdef _DEBUG
			try
			{
				ValueTraits::RegType::ValueType& r = reg->Get(ValueTraits::Convert(valueKey));
				assert(false); // the same named value has already registered.
				return;
			}
			catch(std::exception e)
			{
				// normal
			}
#endif
			reg->Regist(ValueTraits::Convert(valueKey), value);
		}

		template<typename libKeyType, typename valueKeyType>
		void UnRegister(const libKeyType& libKey, const valueKeyType& valueKey)
		{
			typedef df::TTypeTraits<libKeyType> LibTraits;
			typedef df::TTypeTraits<valueKeyType> ValueTraits;

			// find library register
			ValueTraits::RegType* reg = NULL;
			try
			{
				reg = DeltaPTRCast<ValueTraits::RegType>(LibTraits::Convert(libKey));
			}
			catch(std::exception e)
			{
				return;
			}

			assert(reg != NULL);

			reg->UnRegist(ValueTraits::Convert(valueKey));
		}
	}

	class DeltaRegisterProxy
	{
	public:
		template<typename DeltaType, typename KeyType>
		DeltaRegisterProxy(const KeyType& key, const DeltaType& value)
		{
			reg::Register(key, value);
		}

		template<typename DeltaType, typename LibKeyType, typename ValueKeyType>
		DeltaRegisterProxy(const LibKeyType& libKey, const ValueKeyType& valueKey, const DeltaType& value)
		{
			reg::Register(libKey, valueKey, value);
		}
	};

#pragma region refrenced_object_pointer_handler
	
	class Interface
	{
	public:
		virtual ~Interface()=0{}
	};

	class InterfaceCreator
	{
	public:
		virtual const char* IType_type_name()=0;
		virtual const char* RType_type_name()=0;

		virtual Interface* create()=0;
		virtual void destory(Interface*)=0;

		virtual std::shared_ptr<Interface> createPure()=0;
	};

	template<typename T>
	class TInterfaceCreator : public InterfaceCreator
	{
	public:
		virtual const char* IType_type_name()		{ return typeid(T).name(); }
	};

	template<typename T, typename Base>
	class TRInterfaceCreator : public TInterfaceCreator<Base>
	{
	public:
		virtual const char* RType_type_name()	{ return typeid(T).name(); }

		virtual Interface* create()
		{
			return DeltaRefPtrContainer::addObject(std::make_shared<T>());
		}

		virtual void destory(Interface* v)
		{
			DeltaRefPtrContainer::removeObject(v);
		}

		virtual std::shared_ptr<Interface> createPure()
		{
			return std::make_shared<T>();
		}
	};
	

	// �����������ڹ���������
	class CEX_EXPORT_CLASS DeltaRefPtrContainer
	{
	public:
		typedef std::vector<std::shared_ptr<Interface> > SysRefPtrContainerType;

		static bool IsExisted(Interface* obj);

		static Interface* addObject(std::shared_ptr<Interface> obj);

		static void removeObject(Interface* obj);
	};

	//---------------------------------------------------------------------------------------

	// ���󴴽���ע�����
	template<typename IType, typename RType=IType>
	class InterfaceCreatorRegistProxy
	{
	public:
		typedef TRInterfaceCreator<RType, IType>	RTypeCreator;

		InterfaceCreatorRegistProxy()
		{
			reg::Register(DELTA_REGKEY_SYS_TYPE_CREATOR, 
				typeid(IType).name(), 
				std::shared_ptr<InterfaceCreator>(std::make_shared<RTypeCreator>()));
		}
	};

	template<typename IType>
	InterfaceCreator* DeltaGetCreator()
	{
		return DeltaCast<std::shared_ptr<InterfaceCreator> >(
			DELTA_REGKEY_SYS_TYPE_CREATOR, typeid(IType).name()).get(); 
	}

	template<typename T>
	T* DeltaCreate()
	{
		InterfaceCreator* pCreator = DeltaGetCreator<T>();

		return dynamic_cast<T*>(pCreator->create());
	}

	template<typename T>
	void DeltaDestory(T* v)
	{
		InterfaceCreator* pCreator = DeltaGetCreator<T>();

		pCreator->destory(v);
	}

	template<typename T>
	std::shared_ptr<T> DeltaCreateRef()
	{
		InterfaceCreator* pCreator = DeltaGetCreator<T>();

		return std::dynamic_pointer_cast<T>(pCreator->createPure());
	}

	inline std::shared_ptr<Interface> DeltaCreateRef(const char* IType_type_name)
	{
		InterfaceCreator* pCreator = DeltaCast<std::shared_ptr<InterfaceCreator> >(
			DELTA_REGKEY_SYS_TYPE_CREATOR, IType_type_name).get(); 

		return std::dynamic_pointer_cast<Interface>(pCreator->createPure());
	}

	template<typename R1, typename R2>
	R1* DeltaQueryInterface(R2* ptr)
	{
		return dynamic_cast<R1*>(ptr);
	}

	template<typename R1, typename R2>
	R1* DeltaQueryInterface(const std::shared_ptr<R2>& ptr)
	{
		return dynamic_cast<R1*>(ptr.get());
	}
	//-----------------------------------------------------------------------------------------

	// ���󵥼�ע�����(��ʼ������Ĳ�ȷ��, ��̬�����ڲ���������������̬����)
	template<typename IType, typename RType>
	class DeltaObjInstanceRegistProxy
	{
	public:	
		DeltaObjInstanceRegistProxy()
		{
			reg::Register(DELTA_REGKEY_SYS_INSTANCE, 
				typeid(IType).name(), std::shared_ptr<IType>(std::make_shared<RType>()));
		};
	};

	template<typename IType>
	IType& DeltaInstance()
	{
		return *DeltaCast<std::shared_ptr<IType> >(DELTA_REGKEY_SYS_INSTANCE, typeid(IType).name()).get(); 
	}

	template<typename IType, typename RType>
	IType& DeltaGetOrCreateInstance()
	{
		try
		{
			return *cex::DeltaCast<std::shared_ptr<IType> >(DELTA_REGKEY_SYS_INSTANCE, typeid(IType).name()).get();
		}
		catch(std::exception e)
		{
			std::cout << "Creating Instance: "<<typeid(IType).name()<<std::endl;
			cex::DeltaObjInstanceRegistProxy<IType, RType> temp;
			return *cex::DeltaCast<std::shared_ptr<IType> >(DELTA_REGKEY_SYS_INSTANCE, typeid(IType).name()).get();
		}
	}

#pragma endregion 

#define REGIST_DELTA2(key, value) \
	static cex::DeltaRegisterProxy MT_SEQUENCE_NAME(key, value);

#define REGIST_DELTA(libKey, valueKey, value) \
	static cex::DeltaRegisterProxy MT_SEQUENCE_NAME(libKey, valueKey, value);

#define REGIST_DELTA_CREATOR(IType, RType) \
	static cex::InterfaceCreatorRegistProxy<IType, RType> MT_SEQUENCE_NAME;

#define REGIST_DELTA_INSTANCE(IType, RType) \
	static cex::DeltaObjInstanceRegistProxy<IType, RType> MT_SEQUENCE_NAME;

}

#endif