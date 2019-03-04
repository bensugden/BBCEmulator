//-------------------------------------------------------------------------------------------------
//
// Texas Instruments 76489
// Sound generator
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

class TI_76489
{
public:
	TI_76489();

	void				UpdateSoundRegister( u8 value );
	void				Tick( int nCyclesElapsed );

private:
	static const int	c_playerFrequency = 40000;
	static const int	c_streamSize = c_playerFrequency / 50;
	static const int	c_numBuffers = 4;

	int					m_cycles;
	u8					m_storedRegisterValue;
	XAudio2				m_audioPlayer;

	u32					m_frequency[ 4 ];
	u8					m_volume[ 4 ];
	u32					m_cycleCounter[ 4 ];
	bool				m_on[ 4 ];
	u8					m_noiseFB;
	u8					m_streams[ c_numBuffers ][ c_streamSize ];
	u8					m_currentBuffer;
	int					m_currentBufferIndex;
	int					m_skipCount;
};

//-------------------------------------------------------------------------------------------------

