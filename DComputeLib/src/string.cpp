#include "stdafx.h"
#include <assert.h>

#include <DCompute/string.h>
#include <algorithm>

#include <boost/iostreams/stream.hpp>

namespace DCompute {


	String::String() : _data(0), _size(0)
	{
		
	}

	String::String(const String& other) : _data(0), _size(0)
	{
		assign(other._data, other._size);
	}

	String::String( const char* str ) : _data(0), _size(0)
	{
		assign(str);
	}

	String::String( size_t size, char value ) : _data(0), _size(0)
	{
		alloc(size);
	
		for (size_t i=0; i<_size; ++i)
		{
			_data[i]=value;
		}
	}

	String::~String()
	{
		clear();
	}

	void String::alloc(size_t size)
	{
		_size = size;
		_data = new char[size+1];
		_data[_size]=0;
	}

	void String::attach(char* data, size_t size)
	{
		clear();

		_data = data;
		_size = size;
	}

	String& String::operator= (const String& other)
	{
		assign(other._data, other._size);
		return *this;
	}

	String& String::operator= (const char* str)
	{
		assign(str);
		return *this;
	}

	bool String::operator== (const String& other)
	{
		return operator==(other.data());
	}

	bool String::operator== (const char* other)
	{
		if(_data==other) return true; // null object

		if(_data==NULL||other==NULL) return false;

		return strcmp(_data, other)==0;
	}

	const char* String::assign( const char* data, size_t len )
	{
		if (_size >= len && _size>0 )
		{
			memcpy(_data, data, len);
			_data[len]=0;
			_size=len;
		}
		else
		{
			clear();
			alloc(len);
			memcpy(_data, data, len);
		}

		return _data;
	}

	const char* String::assign( const char* data )
	{
		size_t len=0;
		if ( data!=NULL ) len= strlen(data);
		return assign(data, len);

		//boost::iostreams::basic_array_sink<char> sr(_data, _size);
		//boost::iostreams::stream< boost::iostreams::basic_array_sink<char> > source(sr); 
	}

	void String::clear()
	{
		if ( _data != 0 )
		{
			delete [] _data;
			_data=NULL;
		}
		_size=0;
	}

	void String::resize(size_t size)
	{
		if ( _size >= size && _size>0 )
		{
			_size = size;
			_data[_size]=0;
		}
		else
		{
			char* tmp = new char[size+1];
			tmp[size]=0;

			memcpy(tmp, _data, _size);

			attach(tmp, size);
		}
	}

	size_t String::find( char s ) const
	{
		char* ptr = std::find(_data, _data+_size, s);
		if (ptr==NULL) return _size;
		return ptr-_data;
	}

	size_t String::find( const char* str ) const
	{
		size_t strLen = strlen(str);

		size_t counter=0;

		while (counter<_size)
		{
			size_t n=0;

			while(n<strLen)
			{
				if ( str[n]!=_data[counter+n] ) break;
				++n;
			}

			if ( n==strLen )
			{
				return counter; // the same
			}
			else
			{
				++counter;
			}			
		}

		return counter;
	}

	String String::substr( size_t start, size_t counter )
	{
		if ( start >= _size ) start=_size-1;
		if ( start+counter >= _size ) counter=_size-1-start;

		String sub;
		sub.assign(_data+start, counter);

		return sub;
	}

}