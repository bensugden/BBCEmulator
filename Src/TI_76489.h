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
	void				SetVolume( u8 nVolume );
private:
	static const int	c_playerFrequency = 44100;
	static const int	c_streamSize = c_playerFrequency / 50;
	static const int	c_numBuffers = 4;

	double				m_cycles;
	u8					m_storedRegisterValue;
	XAudio2				m_audioPlayer;

	u8					m_masterVolume;
	double				m_frequency[ 4 ];
	u8					m_volume[ 4 ];
	double				m_cycleCounter[ 4 ];
	bool				m_on[ 4 ];
	u8					m_noiseFB;
	u8					m_streams[ c_numBuffers ][ c_streamSize ];
	u8					m_currentBuffer;
	int					m_currentBufferIndex;
	double				m_skipCount;
};

//-------------------------------------------------------------------------------------------------

