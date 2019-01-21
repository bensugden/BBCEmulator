//-----------------------------------------------------------------------------
//	Types
//-----------------------------------------------------------------------------

#pragma once

typedef signed char			s8;
typedef unsigned char		u8;

typedef signed short		s16;
typedef unsigned short		u16;

typedef signed long			s32;
typedef unsigned long		u32;
#ifdef WIN32
typedef signed __int64		s64;
typedef unsigned __int64	u64;
#endif
typedef float				f32;
typedef double				f64;

typedef long double			f80;

typedef unsigned char		byte;

#define LOBYTE_READ(w)		((w) & 0xff)
#define HIBYTE_READ(w)		((w) >> 8) & 0xff)
#define LOBYTE_WRITE(w,v)	{w &= (0xffffff00); w |= ((v) & 0xff); }
#define HIBYTE_WRITE(w,v)	{w &= (0xffff00ff); w |= (((v) & 0xff)<<8); }