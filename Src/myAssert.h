//-------------------------------------------------------------------------------------------------

#undef assert
#ifdef WIN32
#include <string>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#ifndef _XBOX
	#ifdef _DEBUG
		#define assert_str( a, b )	{ __analysis_assume( a ); if ( !(a) ) { MessageBoxA( NULL, (std::string(#a) + std::string(b)).c_str(), __FILE__ , 0 ); DebugBreak(); } }
		#define assert_msg( a, b )	{ __analysis_assume( a ); if ( !(a) ) { MessageBoxA( NULL, (std::string(#a) + std::string(#b)).c_str(), __FILE__ , 0 ); DebugBreak(); } }
	//	#define assert( a )			{ __analysis_assume( a ); if ( !(a) ) { MessageBoxA( NULL, (std::string(#a)).c_str(), __FILE__ , 0 ); DebugBreak();  } }
		#define assert( a )			{ __analysis_assume( a ); if ( !(a) ) { DebugBreak();  } }
	#else
		#define assert_str( a, b )
		#define assert_msg( a, b )
		#define assert( a )
	#endif
#else
	#define assert_msg( a, b ){ __analysis_assume( a ); if ( !(a) ) { int i = 1 ; i /= 0; } }
	#define assert( a ){ __analysis_assume( a ); if ( !(a) ) { int i = 1 ; i /= 0; } }
#endif

#define dxassert( a ) { assert_msg( a == D3D_OK, "d3d assert failed" ); }
#else
#include <assert.h>
// need to fix this for ios
#define assert_str( a, b ) assert( a )
#define assert_msg( a, b ) assert( a )
#endif
//-------------------------------------------------------------- -----------------------------------