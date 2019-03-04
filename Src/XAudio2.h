//-------------------------------------------------------------------------------------------------
//
// XAudio2 Sound Player
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

class XAudio2
{
public: 
	XAudio2( u32 nSamplesPerSecond = 44100 );
	~XAudio2();

	void Stream( u32 nNumBytes, u8* pData, u8 max_buffers );

private:
	HRESULT PlayPCM( struct IXAudio2* pXaudio2, LPCWSTR szFilename );

	struct IXAudio2MasteringVoice*	m_pMasteringVoice;
	struct IXAudio2SourceVoice*		m_pSourceVoice;
	struct IXAudio2*				m_pXAudio2;
};

//-------------------------------------------------------------------------------------------------
