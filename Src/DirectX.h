#pragma once


//-------------------------------------------------------------------------------------------------

namespace GFXSystem
{
	//-------------------------------------------------------------------------------------------------

	struct FrameBufferInfo
	{
		u32* m_pData;
		u32  m_width;
		u32  m_height;
		u32  m_pixelStride;
		u32  m_pitch;
	};

	//-------------------------------------------------------------------------------------------------

	HRESULT			Init( HWND hwnd );
	void			Shutdown();

	void			Render();
	void			SetAnisotropicFiltering( bool on );

	FrameBufferInfo LockFrameBuffer( u32 width, u32 height );
	void			UnlockFrameBuffer( );
}


//-------------------------------------------------------------------------------------------------
