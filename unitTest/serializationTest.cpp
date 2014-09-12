// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//#define TIME_CHECK_ALLWAYS
//#include <TDE/TimeCheck.h>
//#include <TDE/TDE_Includes.h>

#include <fstream>
#include <sstream>

#include <DCompute/serializationInterface.h>

struct Stop
{
	Stop() : kk(2) {}
	int kk;

	friend class boost::serialization::access;
	template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
	{
		ar & kk;
	}

};

class BaseS
{
public:
	BaseS() : m(7)
	{
	}

	virtual ~BaseS()
	{
	}

	friend class boost::serialization::access;
	template<class Archive>
		void serialize( Archive & ar, const unsigned int version )
	{
		ar & m;
	}

	virtual void kkk() {}

	int m;
};


class Foo : public BaseS
{
public:
	GF_DECL_SERIALIZABLE(Foo)

		Foo() : m_nNum1(2), m_dNum2(3.0), m_strInfo("infos") 
	{
		stops[0] = new Stop;
		stops[1] = new Stop;

		//vtStop.push_back( new Stop );
		//vtStop.push_back( stops[0] );
		//vtStop.push_back( stops[1] );
	}

	~Foo() 
	{
		for (size_t i=0; i<2; ++i)
		{
			if ( stops[i] != NULL ) delete stops[i];
		}
	}

	//friend class boost::serialization::access;
	//template<class Archive>
	//	void serialize( Archive & ar, const unsigned int version )
	//{
	//	ar & m_nNum1;
	//	ar & m_dNum2;
	//	ar & m_strInfo;
	//}

	//数据成员
	int m_nNum1;
	double m_dNum2;
	std::string m_strInfo;

	Stop* stops[2];

	std::vector<Stop*> vtStop;
private:

	
	int www;
};

//BOOST_CLASS_VERSION(Stop, 1)
//BOOST_CLASS_VERSION(Base, 1)
//BOOST_CLASS_VERSION(Foo, 1)
//BOOST_CLASS_EXPORT_GUID(Stop, "Stop")
//BOOST_CLASS_EXPORT_GUID(Base, "Base")
//BOOST_CLASS_EXPORT_GUID(Foo, "Foo")

BOOST_CLASS_EXPORT(Stop)
BOOST_CLASS_EXPORT(BaseS)

//BOOST_CLASS_TRACKING(Stop, boost::serialization::track_never)
//BOOST_CLASS_TRACKING(Foo, boost::serialization::track_never)
//BOOST_CLASS_TRACKING(Base, boost::serialization::track_never)

//BOOST_SERIALIZATION_SPLIT_FREE(Foo)

//BOOST_CLASS_TRACKING(Foo, boost::serialization::track_never)


GF_BEGIN_SERIALIZE_IMPL_NOW(Foo, 8)
GF_SERIALIZE_BASE(BaseS)
GF_SERIALIZE_MEMBER(m_nNum1)
GF_SERIALIZE_MEMBER(m_dNum2)
GF_SERIALIZE_MEMBER(m_strInfo)
GF_SERIALIZE_MEMBER(stops)
GF_SERIALIZE_MEMBER(vtStop)
GF_END_SERIALIZE_IMPL

GF_BEGIN_SERIALIZE_IMPL_OLD(Foo, 7)
GF_SERIALIZE_BASE(BaseS)
GF_SERIALIZE_MEMBER(m_nNum1)
GF_SERIALIZE_MEMBER(m_dNum2)
GF_SERIALIZE_MEMBER(m_strInfo)
GF_SERIALIZE_MEMBER(stops)
GF_SERIALIZE_MEMBER(vtStop)	
GF_END_SERIALIZE_IMPL

GF_CLASS_VERSION(Foo, 8)

//// version
//BOOST_CLASS_EXPORT(Foo)
//BOOST_CLASS_VERSION(Foo,8)
//template<class Archive>
//void serialize(Archive & ar, Foo & g, const unsigned int version){
//	GF_QerySerializerAtVersion<8<99, Archive, Foo>(ar,g,version);
//}
//
//// version 7
//template<class Archive>
//void GF_serializer(Archive & ar, Foo& g, const GF_version<7>& v )
//{
//	ar&boost::serialization::base_object<BaseS>(g);
//	ar&g.m_nNum1;
//	ar&g.m_dNum2;
//	ar&g.m_strInfo;
//	ar&g.stops;
//	ar&g.vtStop;
//}
//
//// version 8
//template<class Archive>
//void GF_serializer(Archive & ar, Foo& g, const GF_version<8>& v )
//{
//	ar&boost::serialization::base_object<BaseS>(g);
//	ar&g.m_nNum1;
//	ar&g.m_dNum2;
//	ar&g.m_strInfo;
//	ar&g.stops;
//	ar&g.vtStop;
//}

CEX_TEST(SerializationTest0)
{
	int saveNum = 1;

	// save
	std::ofstream fout("serialization.txt");
	boost::archive::text_oarchive ar_out(fout);


	//TIMECHECK;
	{
		Foo* pp = NULL;
		for (int i=0; i<saveNum; ++i)
		{
			//TIMECHECK1( save );
			//ar_out.template register_type<BaseS>();
			//ar_out.template register_type<Foo>();

			//if ( pp != NULL ) delete pp;
			pp = new Foo();

			Foo& foo=*pp;
			foo.m_nNum1 = 22+i;
			foo.m_dNum2 = 88;
			foo.m_strInfo = "kkk";
			foo.stops[0]->kk = 3+i;
			foo.m = 8;

			//const Foo& v = foo;
			//ar_out << v;
			ar_out << BOOST_SERIALIZATION_NVP(pp);

			// foo2
			Foo foo2/*(foo)*/;
			foo2.m_nNum1=99;
			ar_out << BOOST_SERIALIZATION_NVP(foo2);

			// ptr
			BaseS* pFoo = new Foo;
			ar_out << BOOST_SERIALIZATION_NVP(pFoo);
			delete pFoo;

			assert(foo.m_nNum1 == 22+i);
			assert(foo.m_dNum2 == 88);
			assert(foo.m_strInfo == "kkk");
			assert(foo.stops[0]->kk == 3+i);
			assert(foo.m == 8);
		}
	}

	fout.close();

	// load
	std::ifstream fin("serialization.txt");
	boost::archive::text_iarchive ar_in(fin);

	{
		for (int i=0; i<saveNum; ++i)
		{
			//TIMECHECK1( load );

			Foo* pp;
			ar_in >> BOOST_SERIALIZATION_NVP(pp);
			Foo& new_obj=*pp;

			Foo new_obj2;
			ar_in >> BOOST_SERIALIZATION_NVP(new_obj2);

			BaseS* new_pFoo;
			ar_in >> BOOST_SERIALIZATION_NVP(new_pFoo);
			Foo* new_p = dynamic_cast<Foo*>(new_pFoo);
			assert(new_p!=NULL);

			//Stop* stop = new_obj2.vtStop[1];

			assert(new_obj.m_nNum1 == 22+i);
			assert(new_obj.m_dNum2 == 88);
			assert(new_obj.m_strInfo == "kkk");
			//assert(new_obj.stops[0]->kk == 3+i);
			assert(new_obj.m == 8);
		}
	}


	fin.close();

}

CEX_TEST(SerializationTest)
{
	int saveNum = 1;

	// save
	std::ostringstream fout;
	boost::archive::binary_oarchive ar_out(fout);


	//TIMECHECK;
	{
		Foo* pp = NULL;
		for (int i=0; i<saveNum; ++i)
		{
			//TIMECHECK1( save );
			//ar_out.template register_type<BaseS>();
			//ar_out.template register_type<Foo>();

			//if ( pp != NULL ) delete pp;
			pp = new Foo();

			Foo& foo=*pp;
			foo.m_nNum1 = 22+i;
			foo.m_dNum2 = 88;
			foo.m_strInfo = "kkk";
			foo.stops[0]->kk = 3+i;
			foo.m = 8;

			//const Foo& v = foo;
			//ar_out << v;
			ar_out << BOOST_SERIALIZATION_NVP(pp);

			// foo2
			Foo foo2/*(foo)*/;
			foo2.m_nNum1=99;
			ar_out << BOOST_SERIALIZATION_NVP(foo2);

			// ptr
			BaseS* pFoo = new Foo;
			ar_out << BOOST_SERIALIZATION_NVP(pFoo);
			delete pFoo;

			assert(foo.m_nNum1 == 22+i);
			assert(foo.m_dNum2 == 88);
			assert(foo.m_strInfo == "kkk");
			assert(foo.stops[0]->kk == 3+i);
			assert(foo.m == 8);

		}
	}

	std::string& str = fout.str();

	// load
	std::istringstream fin(str);
	boost::archive::binary_iarchive ar_in(fin);

	{
		for (int i=0; i<saveNum; ++i)
		{
			//TIMECHECK1( load );

			Foo* pp;
			ar_in >> BOOST_SERIALIZATION_NVP(pp);
			Foo& new_obj=*pp;

			Foo new_obj2;
			ar_in >> BOOST_SERIALIZATION_NVP(new_obj2);

			BaseS* new_pFoo;
			ar_in >> BOOST_SERIALIZATION_NVP(new_pFoo);
			Foo* new_p = dynamic_cast<Foo*>(new_pFoo);
			assert(new_p!=NULL);

			//Stop* stop = new_obj2.vtStop[1];

			assert(new_obj.m_nNum1 == 22+i);
			assert(new_obj.m_dNum2 == 88);
			assert(new_obj.m_strInfo == "kkk");
			//assert(new_obj.stops[0]->kk == 3+i);
			assert(new_obj.m == 8);
		}
	}
}

