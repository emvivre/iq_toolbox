/*
===========================================================================

  Copyright (C) 2018 Emvivre

  This file is part of IQ_MODFREQ.

  IQ_MODFREQ is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  IQ_MODFREQ is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with IQ_MODFREQ.  If not, see <http://www.gnu.org/licenses/>.

  ===========================================================================
*/

#include <iostream>
#include <complex>
#include <limits>

static const unsigned int BUFFER_LEN = 200000;

template <class Input, class Output>
void modfreq_( FILE* fd_input, FILE* fd_output )
{
	Input* in_buff = new Input[ BUFFER_LEN ];
	Output* out_buff = new Output[ 2*BUFFER_LEN ];
	Output amplitude = 0.5*std::numeric_limits<Output>::max();
        unsigned int nb_sample_read;
	double phase = 0;
	while( (nb_sample_read = fread( in_buff, sizeof(*in_buff), BUFFER_LEN, fd_input)) > 0 ) {
		for ( unsigned int i = 0; i < nb_sample_read; i++ ) {
			phase += in_buff[ i ];
			std::complex<double> c = std::polar<double>( amplitude, phase );
			out_buff[ 2*i ] = c.real();
			out_buff[ 2*i+1 ] = c.imag();
	        }
		fwrite( out_buff, 2*sizeof(*out_buff), nb_sample_read, fd_output );
                fflush( fd_output );
	}
	delete[] out_buff;
	delete[] in_buff;
}

template <class T>
void modfreq( const std::string& output_data_format, FILE* fd_input, FILE* fd_output )
{
	if      ( output_data_format == "i8"  ) { modfreq_<T,char>( fd_input, fd_output ); }
	else if ( output_data_format == "i16" ) { modfreq_<T,short>( fd_input, fd_output ); }
	else if ( output_data_format == "i32" ) { modfreq_<T,int>( fd_input, fd_output ); }
	else if ( output_data_format == "f32" ) { modfreq_<T,float>( fd_input, fd_output ); }
	else if ( output_data_format == "f64" ) { modfreq_<T,double>( fd_input, fd_output) ; }
}

int main(int argc, char** argv)
{
	if ( argc < 2 ) {
		std::cerr << "Usage: " << argv[0] << " <OPTIONS>\n"
			"  -d <INPUT_DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i16)\n"
			"  -D <OUTPUT_DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i16)\n"
			"  -i <INPUT_CAPTURE_FILE> (default: -)\n"
			"  -o <OUTPUT_CAPTURE_FILE> (default: -)\n";
		return 1;
	}
	const std::string prog_name = argv[0];
	std::string data_format = "i16";
	std::string output_data_format = "i16";
	const char* input_capture_file = "-";
	const char* output_capture_file = "-";
	for ( int i = 1; i < argc; i += 2 ) {
		std::string arg = argv[i];
		if ( arg == "-d" ) {
			data_format = argv[i+1];
		} else if ( arg == "-D" ) {
			output_data_format = argv[i+1];
		} else if ( arg == "-i" ) {
			input_capture_file = argv[i+1];
		} else if ( arg == "-o" ) {
			output_capture_file = argv[i+1];
		}
	}
	if ( data_format != "i8" &&
	     data_format != "i16" &&
	     data_format != "i32" &&
	     data_format != "f32" &&
	     data_format != "f64" ) {
		std::cerr << prog_name << " : ERROR: please set a valid data format !\n";
		return 1;
	}
        if ( output_data_format != "i8" &&
	     output_data_format != "i16" &&
	     output_data_format != "i32" &&
	     output_data_format != "f32" &&
	     output_data_format != "f64" ) {
		std::cerr << prog_name << " : ERROR: please set a valid output data format !\n";
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
        if      ( data_format == "i8"  ) { modfreq<char>( output_data_format, fd_input, fd_output ); }
	else if ( data_format == "i16" ) { modfreq<short>( output_data_format, fd_input, fd_output ); }
	else if ( data_format == "i32" ) { modfreq<int>( output_data_format, fd_input, fd_output ); }
	else if ( data_format == "f32" ) { modfreq<float>( output_data_format, fd_input, fd_output ); }
	else if ( data_format == "f64" ) { modfreq<double>( output_data_format, fd_input, fd_output ); }
	return 0;
}
