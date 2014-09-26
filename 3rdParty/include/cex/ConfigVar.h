/// \file ConfigVar.h Copyright (C).
/// \brief configurable variable by configure file. 
///
/// insteading of .ini files, it can be using variables for reading and writing
///
/// \note:
/// \author: DI
/// \time: 2011/8/30 9:46
#ifndef _CEX_CONFIGVAR_H_
#define _CEX_CONFIGVAR_H_

#pragma region example_usage
/*

/// get variable
if ( GET_bool(ns::shader_use) )
{
	// do something
}

if ( GET_int( great::overview_width ) > 256 ) {}

float fLight1_diffuse = GET_float( light1_diffuse );
std::string str = GET_string( ns::label2 );

/// modify all variable in namespace of seismic by UI
cex::ShowDlg::ShowDlgVarEdit( "ns::abc" );

/// set variable, and writing it into configure file
SET_var( great::overview_width,  150 );

///-------------------------------------------------------

/// regist variable in .cpp file
BEGIN_REGIST_VAR_TABLE_V( ns, 1 )
	REGIST_bool(abc, true,			"comment abc")
	REGIST_int(abc2, 256,			"comment abc2")
	REGIST_string(abc3,  "abc3",	"comment abc3")
END_REGIST_VAR_TABLE

*/
#pragma endregion

#pragma once
#include <cex/Config>

#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cex/DeltaReflection.h>


#if defined(REGIST_bool) || defined(REGIST_int) || defined(REGIST_float) || defined(REGIST_string) || \
	defined(GET_bool) || defined(GET_int) || defined(GET_float) || defined(GET_string) || \
	defined(SET_var)

#error the macro redefined, please modify these macro

#endif

#define VERSION_LINK_IN_VAR_TABLE_REGIST "@"

#define BEGIN_REGIST_VAR_TABLE_V( domain_name, version ) \
	BEGIN_REGIST_VAR_TABLE(cex::ConfigDomainType(#domain_name)+"::", cex::ConfigDomainType(#domain_name)+VERSION_LINK_IN_VAR_TABLE_REGIST+#version )

	/// domain_name: ���ļ�����ConfigVarRW�У����ļ���ȡ��д��ʱ�����á�ÿһ��var_name_space��Ӧһ���ļ���
	/// var_name_space: var_name_space������domain_name����var_name_space��""ʱ��ʹ��GLOBAL_VAR_DOMAIN��Ϊdomain_name��
	///					��ʹ�ð汾��ʱ��var_name_space����汾��һ�����domain_name��
	///	var_name:	var_name_space����������ں�Ϊһ������:"var_name_space::var_name"����Ϊһ���������ַ���ʹ�á�
#define BEGIN_REGIST_VAR_TABLE(var_name_space, config_file_name) namespace{ \
class CRegistVarProxy { public: CRegistVarProxy(); }; \
	static CRegistVarProxy s_CRegistVarProxy; \
	CRegistVarProxy::CRegistVarProxy() { \
	cex::ConfigDomainType strVarNameSpace = var_name_space; \
	cex::ConfigDomainType domainName = config_file_name; \
	cex::IConfigVarRW& pRW = CONFIGVARRW_REGISTER_INS; \
	bool bEnableRegistDomainInfo = !pRW.IsConfigureFileExist( domainName );

#define CONFIGVARRW_REGISTER_INS cex::DeltaInstance<cex::IConfigVarRW>()
#define BOOL_VAR_REGISTER_INS cex::DeltaInstance<cex::ConfigVarBoolean>()
#define FLOAT_VAR_REGISTER_INS cex::DeltaInstance<cex::ConfigVarFloat>()
#define STRING_VAR_REGISTER_INS cex::DeltaInstance<cex::ConfigVarString>()

#define VAR_IN_NAME_SPACE( var ) strVarNameSpace+#var

#define	REGIST_bool( var, value, info ) \
	BOOL_VAR_REGISTER_INS.RegistVar( domainName, VAR_IN_NAME_SPACE(var), value, info );

#define	REGIST_int( var, value, info ) \
	FLOAT_VAR_REGISTER_INS.RegistVar( domainName, VAR_IN_NAME_SPACE(var), (int)value, info );

#define	REGIST_float( var, value, info ) \
	FLOAT_VAR_REGISTER_INS.RegistVar( domainName, VAR_IN_NAME_SPACE(var), value, info );

#define	REGIST_string( var, value, info ) \
	STRING_VAR_REGISTER_INS.RegistVar( domainName, VAR_IN_NAME_SPACE(var), value, info );

#define END_REGIST_VAR_TABLE \
	if ( bEnableRegistDomainInfo ) { pRW.SaveVar2File(domainName); } } }

	//#define VAR_USE_NAMESPACE(name_space) \
	//	static char* __decl_var_name_space() { return #name_space; }
	//
	//template<typename T>
	//std::string __get_var_name_space(const T& valueName)
	//{
	//	__if_exists( __decl_var_name_space )
	//	{
	//		return __decl_var_name_space() + "::" + valueName;
	//	}
	//
	//	return valueName;
	//}
	//
	//#define VAR_OUT_NAME_SPACE( var ) __get_var_name_space(#var)
#define VAR_OUT_NAME_SPACE( var ) #var

#define GET_bool( var ) \
	BOOL_VAR_REGISTER_INS.GetVar( VAR_OUT_NAME_SPACE(var) )

#define GET_int( var ) \
	(int)FLOAT_VAR_REGISTER_INS.GetVar( VAR_OUT_NAME_SPACE(var) )

#define GET_float( var ) \
	FLOAT_VAR_REGISTER_INS.GetVar( VAR_OUT_NAME_SPACE(var) )

#define GET_string( var ) \
	STRING_VAR_REGISTER_INS.GetVar( VAR_OUT_NAME_SPACE(var) )

#define  SET_var( var, value ) \
	cex::VarRegisterUtil::SetVar( #var, value )

	////////////////////////////////////////////////////////////////////////////
struct lua_State;

namespace cex
{
	typedef std::string		ConfigDomainType;
	typedef std::string		ConfigCommentType;
	class IConfigVarRW;

	////////////////////////////////////////////
	//	class ConfigVar
	template<typename DataType>
	class IConfigVar : public Interface
	{
	public:
		typedef std::string		NameType;
		typedef typename DataType::ValueType	ValueType;
		typedef typename DataType::VarInfo		VarInfo;

		typedef std::map< NameType, ValueType > Var_Map;

		typedef DataType DataType;

	public:
		virtual void RegistVar( 
			const ConfigDomainType& domain, const NameType& name, 
			const ValueType& value, const ConfigCommentType& comment )=0;

		virtual ValueType GetVar( const NameType& varName )=0;
		
		virtual void SetVar( const NameType& name, const ValueType& value )=0;

		virtual void SetVarPure( const NameType& name, const ValueType& value )=0;

	};	


	////////////////////////////////////////////
	//	class ConfigVarRWByLua
	//	ע�⣺��ӿ���ʹ�õı�����������namespace
	class IConfigVarRW : public Interface
	{
	public:
		typedef std::string DomainType;

		typedef boost::function<bool(lua_State*)> LoadValueFunc;
		typedef std::vector<LoadValueFunc> LoadValueFuncList;

		struct VarInfo
		{
			DomainType name;
			DomainType value;
			DomainType comment;

			VarInfo() {}

			VarInfo(const DomainType& n, const DomainType& v, const DomainType& c) :
				name(n), value(v), comment(c)
			{}
		};

		typedef std::vector<VarInfo> VarInfoList;
		typedef std::map<DomainType, VarInfoList > Var_Domain_Map;

		virtual void RegisterLoadValueFunc(const LoadValueFunc& func)=0;

		virtual const LoadValueFuncList& GetLoadValueFuncList() const =0;

		// ע��: ʹ��ǰ��Ӧʹ��TryLoadVarFromFile�����ļ��л�ȡ�ı��ı���
		virtual VarInfoList& GetDomianVars(const DomainType& domain)=0;

		// ע�������Ϣ
		virtual void RegistDomain( const DomainType& domain, const DomainType& name, const DomainType& value, const DomainType& comment)=0;

		// ���±�������Ϣ.
		virtual bool UpdateVarInfo( const DomainType& varName, const DomainType& value )=0;

		// ��ȡ���а汾��Ϣ������
		virtual DomainType GetDomainNameInVersion( const DomainType& nameWithoutVersion )=0;


	public:
		virtual void SaveVar2File(const DomainType& domain)=0;

		virtual void TryLoadVarFromFile()=0;
		virtual void LoadVarFromFile(const DomainType& domain)=0;

		virtual bool IsConfigureFileExist(const DomainType& domain)=0;

	};

	/////////////////////////////////////////////////////
	//	class ConfigVarTypeBase
	template<typename valueType>
	class ConfigVarTypeBase
	{
	public:
		typedef ConfigVarTypeBase<valueType> ThisType;

		typedef std::string	VarType;
		typedef valueType	ValueType;

		//	�������ñ������Ϣ
		struct VarInfo
		{
			VarType name;
			ValueType value;
			ConfigCommentType comment;

			VarInfo() : value(0) {}

			VarInfo(const VarType& n, ValueType num, const ConfigCommentType& c ) : 
				name(n), value(num), comment(c) 
			{}

		};
	};

	/// bool�ͱ���
	class CEX_EXPORT_CLASS ConfigVarBooleanType : public ConfigVarTypeBase<bool>
	{
	public:
		static std::string Value2String(const ValueType& value);
		static bool LoadValueFunc(lua_State* L);
	};

	/// float/int�ͱ���
	class CEX_EXPORT_CLASS ConfigVarFloatType : public ConfigVarTypeBase<float>
	{
	public:
		static std::string Value2String(const ValueType& value);
		static bool LoadValueFunc(lua_State* L);
	};

	/// string�ͱ���
	class CEX_EXPORT_CLASS ConfigVarStringType : public ConfigVarTypeBase<std::string>
	{
	public:
		static std::string Value2String(const ValueType& value);
		static bool LoadValueFunc(lua_State* L);
	};

	typedef IConfigVar<ConfigVarBooleanType> ConfigVarBoolean;
	typedef IConfigVar<ConfigVarFloatType> ConfigVarFloat;
	typedef IConfigVar<ConfigVarStringType> ConfigVarString;


	class CEX_EXPORT_CLASS VarRegisterUtil
	{
	public:
		static void SetVar( const std::string& varName, bool v);

		static void SetVar( const std::string& varName, float v);

		static void SetVar( const std::string& varName, int v);

		static void SetVar( const std::string& varName, double v);

		static void SetVar( const std::string& varName, const char* v);

		static void SetVar( const std::string& varName, const std::string& v);
	};

}

#endif
