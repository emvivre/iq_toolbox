/*
  ===========================================================================

  Copyright (C) 2018 Emvivre

  This file is part of IQ_DEMODFREQ.

  IQ_DEMODFREQ is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  IQ_DEMODFREQ is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with IQ_DEMODFREQ.  If not, see <http://www.gnu.org/licenses/>.

  ===========================================================================
*/

#include <iostream>
#include <complex>

static const unsigned int BUFFER_LEN = 200000;
static const double PI = 4 * std::atan(1);

template <class Input, class Output>
void demodfreq_( const unsigned int sample_rate, FILE* fd_input, FILE* fd_output )
{
	Input* in_buff = new Input[ 2*BUFFER_LEN ];
	Output* out_buff = new Output[ BUFFER_LEN ];
	std::complex<double> c_prev(0,0);
	unsigned int nb_sample_read;
	while( (nb_sample_read = fread( in_buff, 2*sizeof(*in_buff), BUFFER_LEN, fd_input)) > 0 ) {
		for ( unsigned int i = 0; i < nb_sample_read; i++ ) {
			std::complex<double> c( in_buff[2*i], in_buff[2*i+1] );
		        double freq = std::arg( c * std::conj(c_prev) ) * sample_rate / (2 * PI);
			out_buff[ i ] = freq;
			c_prev = c;
		}
		fwrite( out_buff, sizeof(*out_buff), nb_sample_read, fd_output );
                fflush( fd_output );
	}
	delete[] out_buff;
	delete[] in_buff;
}

template <class T>
void demodfreq( const unsigned int sample_rate, const std::string& output_data_format, FILE* fd_input, FILE* fd_output )
{
	if      ( output_data_format == "i8"  ) { demodfreq_<T,char>( sample_rate, fd_input, fd_output ); }
	else if ( output_data_format == "i16" ) { demodfreq_<T,short>( sample_rate, fd_input, fd_output ); }
	else if ( output_data_format == "i32" ) { demodfreq_<T,int>( sample_rate, fd_input, fd_output ); }
	else if ( output_data_format == "f32" ) { demodfreq_<T,float>( sample_rate, fd_input, fd_output ); }
	else if ( output_data_format == "f64" ) { demodfreq_<T,double>( sample_rate, fd_input, fd_output) ; }
}

int main(int argc, char** argv)
{
	if ( argc < 2 ) {
		std::cerr << "Usage: " << argv[0] << " <OPTIONS>\n"
			"  -s <SAMPLE_RATE>\n"
			"  -d <INPUT_DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i16)\n"
			"  -D <OUTPUT_DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: f32)\n"
			"  -i <INPUT_CAPTURE_FILE> (default: -)\n"
			"  -o <OUTPUT_CAPTURE_FILE> (default: -)\n";
		return 1;
	}
        unsigned int sample_rate = 0;
	std::string data_format = "i16";
	std::string output_data_format = "f32";
	const char* input_capture_file = "-";
	const char* output_capture_file = "-";
	for ( int i = 1; i < argc; i += 2 ) {
		std::string arg = argv[i];
		if ( arg == "-s" ) {
                        sample_rate = atof( argv[i+1] );
                } else if ( arg == "-d" ) {
			data_format = argv[i+1];
		} else if ( arg == "-D" ) {
			output_data_format = argv[i+1];
		} else if ( arg == "-i" ) {
			input_capture_file = argv[i+1];
		} else if ( arg == "-o" ) {
			output_capture_file = argv[i+1];
		}
	}
        if ( sample_rate == 0 ) {
                std::cerr << "ERROR: please set a valid sample rate !\n";
                return 1;
        }
	if ( data_format != "i8" &&
	     data_format != "i16" &&
	     data_format != "i32" &&
	     data_format != "f32" &&
	     data_format != "f64" ) {
		std::cerr << "ERROR: please set a valid data format !\n";
		return 1;
	}
	if ( output_data_format != "i8" &&
	     output_data_format != "i16" &&
	     output_data_format != "i32" &&
	     output_data_format != "f32" &&
	     output_data_format != "f64" ) {
		std::cerr << "ERROR: please set a valid output data format !\n";
		return 1;
	}
        FILE* fd_input = stdin;
	if ( input_capture_file != std::string("-") ) {
		fd_input = fopen( input_capture_file, "rb" );
		if ( fd_input == NULL ) {
			perror("fopen()");
			return 1;
		}
	}
	FILE* fd_output = stdout;
	if ( output_capture_file != std::string("-") ) {
		fd_output = fopen( output_capture_file, "w+b" );
		if ( fd_output == NULL ) {
			perror("fopen()");
			return 1;
		}
	}
	if      ( data_format == "i8"  ) { demodfreq<char>( sample_rate, output_data_format, fd_input, fd_output); }
	else if ( data_format == "i16" ) { demodfreq<short>( sample_rate, output_data_format, fd_input, fd_output); }
	else if ( data_format == "i32" ) { demodfreq<int>( sample_rate, output_data_format, fd_input, fd_output); }
	else if ( data_format == "f32" ) { demodfreq<float>( sample_rate, output_data_format, fd_input, fd_output); }
	else if ( data_format == "f64" ) { demodfreq<double>( sample_rate, output_data_format, fd_input, fd_output); }
	return 0;
}
