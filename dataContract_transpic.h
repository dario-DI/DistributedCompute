// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <fstream>

#include <DCompute/typeWrappers.h>
#include <DCompute/string.h>

class ImageInfoContract
{
public:
	GF_DECL_SERIALIZABLE(ImageInfoContract)

	WORD wBitCount; // 每个像素所占字节数
	LONG biWidth;	// 宽
	LONG biHeight;	// 高

	DCompute::String data; // 像素内存

	ImageInfoContract()
	{
		wBitCount=4;
		biWidth=0;
		biHeight=0;
		data=0;
	}
};

GF_BEGIN_SERIALIZE_IMPL_NOW(ImageInfoContract, 1)
	GF_SERIALIZE_MEMBER(wBitCount)
	GF_SERIALIZE_MEMBER(biWidth)
	GF_SERIALIZE_MEMBER(biHeight)
	GF_SERIALIZE_MEMBER(data)
GF_END_SERIALIZE_IMPL

GF_CLASS_VERSION(ImageInfoContract, 1)

extern void CopyScreen2Image(ImageInfoContract& image);
extern BOOL  SwitchInputDesktop();

class TransPicContract
{
public:
	TransPicContract()
	{
	}

	GF_DECL_SERIALIZABLE(TransPicContract)

	ImageInfoContract result;

	void Do()
	{
		CopyScreen2Image(result);
		printf("TransPicContract do.\n");

	}
};

GF_BEGIN_SERIALIZE_IMPL_NOW(TransPicContract, 1)
GF_END_SERIALIZE_IMPL

GF_CLASS_VERSION(TransPicContract, 1)

class MouseEventContract
{
public:
	MouseEventContract()
	{
	}

	int result;

	int x;
	int y;

	DWORD dwMouseFlags;

	void Do()
	{
		SwitchInputDesktop();
		::SetCursorPos(x,y); 
		Sleep(200);
		::mouse_event(dwMouseFlags, x, y, 0, 0);
	}
};

GF_BEGIN_SERIALIZE_IMPL_NOW(MouseEventContract, 1)
	GF_SERIALIZE_MEMBER(x)
	GF_SERIALIZE_MEMBER(y)
	GF_SERIALIZE_MEMBER(dwMouseFlags)
GF_END_SERIALIZE_IMPL

GF_CLASS_VERSION(MouseEventContract, 1)