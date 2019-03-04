//-------------------------------------------------------------------------------------------------
//
// Texas Instruments 76489
// Sound generator
//
//-------------------------------------------------------------------------------------------------

#include <stdafx.h>

//-------------------------------------------------------------------------------------------------

TI_76489::TI_76489()
	: m_audioPlayer( c_playerFrequency )
	, m_currentBuffer( 0 )
	, m_currentBufferIndex( 0 )
{
	m_skipCount = cpu.Frequency() / c_playerFrequency;
	m_on[ 0 ] = m_on[ 1 ] = m_on[ 2 ] =	m_on[ 3 ] = false;
	m_volume[ 0 ] = m_volume[ 1 ] =	m_volume[ 2 ] =	m_volume[ 3 ] = 0;
	m_frequency[ 0 ] = m_frequency[ 1 ] = m_frequency[ 2 ] = m_frequency[ 3 ] = m_skipCount;
	m_cycles = 0;
	m_noiseFB = 0;
	m_cycleCounter[ 0 ] = m_cycleCounter[ 1 ] = m_cycleCounter[ 2 ] = m_cycleCounter[ 3 ] = 0;
	for ( int i = 0; i < c_numBuffers; i++ )
	{
		memset( &m_streams[ i ], 128, c_streamSize );
	}
}

//-------------------------------------------------------------------------------------------------

void TI_76489::Tick( int nCyclesElapsed )
{
	m_cycles += nCyclesElapsed;
	while ( m_cycles >= m_skipCount )
	{
		m_cycles -= m_skipCount;

		int total_value = 128;
		for ( int channel = 0;  channel < 3; channel++ )
		{
			m_cycleCounter[ channel ] += m_skipCount;
			if ( m_cycleCounter[ channel ] > m_frequency[ channel ] )
			{
				m_cycleCounter[ channel ] -= m_frequency[ channel ] ;
				m_on[ channel ] = !m_on[ channel ];
			}
			if ( m_on[ channel ] )
				total_value += m_volume[ channel ];
			else
				total_value -= m_volume[ channel ];
		}
		if ( total_value > 255 )
			total_value = 255;
		if ( total_value < 0 )
			total_value = 0;
		//
		// Noise
		//
		if ( m_volume[ 3 ] > 0 )
		{
			//
			// White Noise
			//
			if ( m_noiseFB == 1 )
			{
			//	total_value -= m_volume
			}
		}

		m_streams[ m_currentBuffer ][ m_currentBufferIndex++ ] = (u8)total_value;

		if ( m_currentBufferIndex == c_streamSize )
		{
			m_audioPlayer.Stream( c_streamSize, m_streams[ m_currentBuffer ], c_numBuffers );
			m_currentBuffer = ( m_currentBuffer + 1 ) & ( c_numBuffers - 1 );
			m_currentBufferIndex = 0;
		}
	}
}

//-------------------------------------------------------------------------------------------------

void TI_76489::UpdateSoundRegister( u8 value )
{
	if ( value & 0x80 )
	{
		u8 mode = ( value >> 4 ) & 7;

		switch ( mode )
		{
			case 0:
			case 2:
			case 4:
			{
				// tone frequency ( first byte )
				m_storedRegisterValue = value;
				break;
			}
			case 1:
			case 3:
			case 5:
			{
				// tone volume
				u8 channel = 2 - ( mode >> 1 );
				static int expVol[] = { 0, 11, 14, 17, 20, 24, 28, 33, 39, 46, 54, 63, 74, 87, 102, 120 };
				m_volume[ channel ] =  15 - ( value & 0xf );
				break;
			}
			case 6:
			{
				// noise control
				m_frequency[ 3 ] = value & 3;
				m_noiseFB = ( value >> 2 ) & 1;
				break;
			}
			case 7:
			{
				// noise volume
				static int expVol[] = { 0, 11, 14, 17, 20, 24, 28, 33, 39, 46, 54, 63, 74, 87, 102, 120 };
				m_volume[ 3 ] = 15 - ( value & 0xf );
				break;
			}
		}
	}
	else
	{
		// tone frequency ( 2nd byte )
		u8 channel = 2 - ( ( m_storedRegisterValue >> 5 ) & 3 );
		u16 raw_frequency = ( m_storedRegisterValue & 7 ) + ( ( value & 0x3f ) << 4 );
		u32 real_frequency = 4000000 / ( 32 * raw_frequency );
		u32 cpu_frequency = cpu.Frequency() / ( real_frequency * 2 ); // this many cycles we toggle on / off
		m_frequency[ channel ] = cpu_frequency ;
	}
}

//-------------------------------------------------------------------------------------------------


