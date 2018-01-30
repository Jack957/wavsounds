// source from http://www.cplusplus.com/forum/beginner/166954/

#include <cmath>
#include <fstream>
#include <iostream>
using namespace std;

namespace little_endian_io
{
  template <typename Word>
  std::ostream& write_word( std::ostream& outs, Word value, unsigned size = sizeof( Word ) )
  {
    for (; size; --size, value >>= 8)
      outs.put( static_cast <char> (value & 0xFF) );
    return outs;
  }
}
using namespace little_endian_io;

int main()
{
  ofstream f( "example48khz.wav", ios::binary );
  int sampleRate = 48000;
  int chnum = 2;
  int bitspersample = 16;
  double seconds = 5; // period of this sound
  
  // Write the file headers
  f << "RIFF----WAVEfmt ";     // (chunk size to be filled in later) Chunk ID = "RIFF" WAVEID = "fmt "
  write_word( f,     16, 4 );  // no extension data 	//chunk size
  write_word( f,      1, 2 );  // PCM - integer samples //Format code 1:PCM
  write_word( f,  chnum, 2 );  // two channels (stereo file) //Number of interleaved channels
  write_word( f, sampleRate, 4 );  // samples per second (Hz)	// Sample rate
  write_word( f, sampleRate*16*chnum/8, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8   //data rate in bytes
  write_word( f,      4, 2 );  // data block size (size of two integer samples, one for each channel, in bytes)
  write_word( f, bitspersample, 2 );  // number of bits per sample (use a multiple of 8)

  // Write the data chunk header
  size_t data_chunk_pos = f.tellp();
  f << "data----";  // (chunk size to be filled in later)
  
  // Write the audio samples
  // (We'll generate a single C4 note with a sine wave, fading from left to right)
  const double two_pi = 6.283185307179586476925286766559;
  const double max_amplitude = 32760;  // "volume"

  double hz        = sampleRate;    // samples per second
  double frequency = 261.626;  // middle C
  
  int N = hz * seconds;  // total number of samples
  for (int n = 0; n < N; n++)
  {
    double amplitude = (double)n / N * max_amplitude;
    double value     = sin( (two_pi * n * frequency) / hz );
    write_word( f, (int)(                 amplitude  * value), 2 );//left channel <<
    write_word( f, (int)((max_amplitude - amplitude) * value), 2 );//right channle >>
  }
  
  // (We'll need the final file size to fix the chunk sizes above)
  size_t file_length = f.tellp();

  // Fix the data chunk header to contain the data size
  f.seekp( data_chunk_pos + 4 );
  write_word( f, file_length - data_chunk_pos + 8 );

  // Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
  f.seekp( 0 + 4 );
  write_word( f, file_length - 8, 4 ); 
}
