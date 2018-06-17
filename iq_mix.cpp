/*
  ===========================================================================

  Copyright (C) 2018 Emvivre

  This file is part of IQ_MIX.

  IQ_MIX is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  IQ_MIX is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with IQ_MIX.  If not, see <http://www.gnu.org/licenses/>.

  ===========================================================================
*/

#include <iostream>
#include <complex>

static const double PI = 4 * std::atan(1);

template <class T>
void mix( const unsigned int sample_rate, const int frequency_mixing, FILE* fd_input, FILE* fd_output )
{
	T* in_buff = new T[ 2*sample_rate ];
	T* out_buff = new T[ 2*sample_rate ];
	while( fread( in_buff, 2*sizeof(*in_buff), sample_rate, fd_input) == sample_rate ) {
	        for ( unsigned int i = 0; i < sample_rate; i++ ) {
			std::complex<double> c( in_buff[2*i], in_buff[2*i+1] );
			c *= std::polar<double>( 1, - 2 * PI * frequency_mixing * i * 1. / sample_rate );
			out_buff[ 2*i ] = c.real();
			out_buff[ 2*i+1 ] = c.imag();
		}
		fwrite( out_buff, 2*sizeof(*out_buff), sample_rate, fd_output );
                fflush( fd_output );
	}
	delete[] out_buff;
	delete[] in_buff;
}

int main(int argc, char** argv)
{
	if ( argc < 2 ) {
		std::cerr << "Usage: " << argv[0] << " <OPTIONS>\n"
			"  -s <SAMPLE_RATE>\n"
			"  -d <DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i16)\n"
			"  -m <FREQUENCY_MIXING> (default: 0)\n"
			"  -i <INPUT_CAPTURE_FILE> (default: -)\n"
			"  -o <OUTPUT_CAPTURE_FILE> (default: -)\n";
		return 1;
	}
        const std::string prog_name = argv[0];
	unsigned int sample_rate = 0;
	int frequency_mixing = 0;
	std::string data_format = "i16";
	const char* input_capture_file = "-";
	const char* output_capture_file = "-";
	for ( int i = 1; i < argc; i += 2 ) {
		std::string arg = argv[i];
		if ( arg == "-s" ) {
			sample_rate = atof( argv[i+1] );
		} else if ( arg == "-m")  {
			frequency_mixing = atof( argv[i+1] );
		} else if ( arg == "-d" ) {
			data_format = argv[i+1];
		} else if ( arg == "-i" ) {
			input_capture_file = argv[i+1];
		} else if ( arg == "-o" ) {
			output_capture_file = argv[i+1];
		}
	}
	if ( sample_rate == 0 ) {
		std::cerr << prog_name << " : ERROR: please set a valid sample rate !\n";
		return 1;
	}
        if ( data_format != "i8" &&
	     data_format != "i16" &&
	     data_format != "i32" &&
	     data_format != "f32" &&
	     data_format != "f64" ) {
		std::cerr << prog_name << " : ERROR: please set a valid data format !\n";
		return 1;
	}
	FILE* fd_input = stdin;
	if ( input_capture_file != std::string("-") ) {
		fd_input = fopen( input_capture_file, "rb" );
		if ( fd_input == NULL ) {
			std::cerr << prog_name << " : ";
			perror("fopen()");
			return 1;
		}
	}
	FILE* fd_output = stdout;
	if ( output_capture_file != std::string("-") ) {
		fd_output = fopen( output_capture_file, "w+b" );
		if ( fd_output == NULL ) {
			std::cerr << prog_name << " : ";
			perror("fopen()");
			return 1;
		}
	}
	if      ( data_format == "i8"  ) { mix<char>( sample_rate, frequency_mixing, fd_input, fd_output); }
	else if ( data_format == "i16" ) { mix<short>( sample_rate, frequency_mixing, fd_input, fd_output); }
	else if ( data_format == "i32" ) { mix<int>( sample_rate, frequency_mixing, fd_input, fd_output); }
	else if ( data_format == "f32" ) { mix<float>( sample_rate, frequency_mixing, fd_input, fd_output); }
	else if ( data_format == "f64" ) { mix<double>( sample_rate, frequency_mixing, fd_input, fd_output); }
	return 0;
}
