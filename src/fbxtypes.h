#pragma once

namespace fbx
{
	//
	struct CStaticName
	{
		char    raw[32];
	};

	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef unsigned long long u64;
	typedef signed long long i64;

	static_assert(sizeof(u8) == 1, "u8 is not 1 byte");
	static_assert(sizeof(u32) == 4, "u32 is not 4 bytes");
	static_assert(sizeof(u64) == 8, "u64 is not 8 bytes");
	static_assert(sizeof(i64) == 8, "i64 is not 8 bytes");

	struct OFBVector2
	{
		double x;
		double y;
	};

	struct OFBVector3
	{
		double x;
		double y;
		double z;

		double& operator [] (int index)
		{
			return *(&x + index);
		}
		double operator [] (int index) const
		{
			return *(&x + index);
		}
	};

	struct OFBVector4
	{
		double x;
		double y;
		double z;
		double w;

		double& operator [] (int index)
		{
			return *(&x + index);
		}
		double operator [] (int index) const
		{
			return *(&x + index);
		}
	};

	struct OFBColor
	{
		double r;
		double g;
		double b;
	};

	struct OFBMatrix
	{
		double m[16];

		double& operator [] (int index)
		{
			return m[index];
		}
		double operator [] (int index) const
		{
			return m[index];
		}

		double& operator () (int col, int row)
		{
			return m[col * 4 + row];
		}
		double operator () (int col, int row) const
		{
			return m[col * 4 + row];
		}
	};

	// Common independant definitions

	typedef signed char			kByte;         /* signed 8-bit integer         */
	typedef unsigned char		kUByte;        /* unsigned 8-bit integer       */
	typedef signed char			kSChar;
	typedef unsigned char		kUChar;
	typedef unsigned int		kUInt;
	typedef signed short		kShort;
	typedef unsigned short		kUShort;

	typedef double				kVector2[2];   /* 2 x 64-bit real              */
	typedef double				kVector3[3];   /* 3 x 64-bit real              */
	typedef double				kVector4[4];   /* 4 x 64-bit real              */
	typedef double				kMatrix[16];


	typedef signed long long	kLongLong;
#define K_LONGLONG(x)		(x##LL)
	typedef unsigned long long	kULongLong;
#define K_ULONGLONG(x)		(x##ULL)

#define K_LONGLONG_MIN		K_LONGLONG(0x8000000000000000)
#define K_LONGLONG_MAX		K_LONGLONG(0x7fffffffffffffff)
#define K_ULONGLONG_MIN		K_ULONGLONG(0)
#define K_ULONGLONG_MAX		K_ULONGLONG(0xffffffffffffffff)
}
