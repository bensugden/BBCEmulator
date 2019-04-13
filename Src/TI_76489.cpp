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
	m_skipCount = (double)cpu.Frequency() /  (double)c_playerFrequency;
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
	m_storedRegisterValue = 0;
	m_masterVolume = 255;
	srand(12345);
}

//-------------------------------------------------------------------------------------------------

void TI_76489::Tick( int nCyclesElapsed )
{
	m_cycles += (double)nCyclesElapsed;
	while ( m_cycles >= m_skipCount )
	{
		if ( m_masterVolume == 0 )
			return;
		m_cycles -= m_skipCount;

		for ( int channel = 0;  channel < 3; channel++ )
		{
			m_cycleCounter[ channel ] += m_skipCount;
			if ( m_cycleCounter[ channel ] > m_frequency[ channel ] )
			{
				m_cycleCounter[ channel ] -= m_frequency[ channel ] ;
				m_on[ channel ] = !m_on[ channel ];
			}
		}

		//
		// Noise
		//
		if ( m_volume[ 3 ] > 0 )
		{
			double nNoiseFrequency = m_frequency[ 3 ];
			if ( nNoiseFrequency == 0 ) // this indicates we take the frequency from channel 1
			{
				nNoiseFrequency = m_frequency[ 0 ];
			}

			m_cycleCounter[ 3 ] += m_skipCount;

			if ( m_noiseFB == 1 )
			{
				//
				// White Noise
				//
				if ( m_cycleCounter[ 3 ] > nNoiseFrequency )
				{
					m_on[ 3 ] = ( rand() & 1 ) == 1;
				}
			}
			else
			{
				//
				// Periodic Noise
				//
				if ( m_on[ 3 ] )
				{
					nNoiseFrequency *= 10;
				}
				if ( m_cycleCounter[ 3 ] > nNoiseFrequency )
				{
					m_on[ 3 ] = !m_on[ 3 ];
				}
			}
		}

		int total_value = 128;
		for ( int channel = 0;  channel < 4; channel++ )
		{
			if ( m_on[ channel ] )
				total_value += m_volume[ channel ];
			else
				total_value -= m_volume[ channel ];
		}


		if ( total_value > 255 )
			total_value = 255;
		if ( total_value < 0 )
			total_value = 0;

		m_streams[ m_currentBuffer ][ m_currentBufferIndex++ ] = (u8)((total_value*m_masterVolume)>>8);

		if ( m_currentBufferIndex == c_streamSize )
		{
			m_audioPlayer.Stream( c_streamSize, m_streams[ m_currentBuffer ], c_numBuffers );
			m_currentBuffer = ( m_currentBuffer + 1 ) & ( c_numBuffers - 1 );
			m_currentBufferIndex = 0;
		}
	}
}

//-------------------------------------------------------------------------------------------------

void TI_76489::SetVolume( u8 volume )
{
	m_masterVolume = volume;
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
				m_volume[ channel ] =  15 - (value & 0xf );
				break;
			}
			case 6:
			{
				// noise control
				int noise_freq = value & 3;
				if ( m_noiseFB == 1 )
				{
					if ( noise_freq == 0 )
						m_frequency[ 3 ] = ((double)cpu.Frequency()) / ( 10000 * 2 );
					else if ( noise_freq == 1 )
						m_frequency[ 3 ] = ((double)cpu.Frequency()) / ( 5000 * 2 );
					else if ( noise_freq == 2 )
						m_frequency[ 3 ] = ((double)cpu.Frequency()) / ( 2500 * 2 );
					else
						m_frequency[ 3 ] = 0;
				}
				else
				{
					m_frequency[ 3 ] = noise_freq;
				}
				m_noiseFB = ( value >> 2 ) & 1;

				break;
			}
			case 7:
			{
				// noise volume
				static int expVol[] = { 0, 11, 14, 17, 20, 24, 28, 33, 39, 46, 54, 63, 74, 87, 102, 120 };
				m_volume[ 3 ] =  15 - (value & 0xf );
				break;
			}
		}
	}
	else
	{
		if ( m_storedRegisterValue == 0 )
			return;
		// tone frequency ( 2nd byte )
		u8 channel = 2 - ( ( m_storedRegisterValue >> 5 ) & 3 );
		double raw_frequency = ( m_storedRegisterValue & 0xf ) + ( ( value & 0x3f ) << 4 );
		double real_frequency = 4000000 / ( 32 * raw_frequency );
		double cpu_frequency = cpu.Frequency() / ( real_frequency * 2 ); // this many cycles we toggle on / off
		m_frequency[ channel ] = cpu_frequency ;
	}
}

//-------------------------------------------------------------------------------------------------


