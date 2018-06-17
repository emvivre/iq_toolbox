/*
  ===========================================================================

  Copyright (C) 2018 Emvivre

  This file is part of IQ_NORMALIZE.

  IQ_NORMALIZE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  IQ_NORMALIZE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with IQ_NORMALIZE.  If not, see <http://www.gnu.org/licenses/>.

  ===========================================================================
*/

#include <iostream>
#include <complex>
#include <limits>

static const unsigned int BUFFER_LEN = 20000;

template <class T>
void normalize_scalar( double max_norm, FILE* fd_input, FILE* fd_output )
{
	T* in_buff = new T[ BUFFER_LEN ];
	T* out_buff = new T[ BUFFER_LEN ];
	double max = -std::numeric_limits<double>::infinity();
	unsigned int nb_sample_read;
	while( (nb_sample_read = fread( in_buff, sizeof(*in_buff), BUFFER_LEN, fd_input)) > 0 ) {
		for ( unsigned int i = 0; i < nb_sample_read; i++ ) {
			const T v = in_buff[ i ];
			double n = (v > 0) ? v : -v;
			if ( n > max ) {
				max = n;
			}
		}
		for ( unsigned int i = 0; i < nb_sample_read; i++ ) {
			out_buff[ i ] = max_norm * in_buff[ i ] / max;
		}
		fwrite( out_buff, sizeof(*out_buff), nb_sample_read, fd_output );
                fflush( fd_output );
	}
	delete[] out_buff;
	delete[] in_buff;
}

template <class T>
void normalize_iq( double max_norm, FILE* fd_input, FILE* fd_output )
{
	T* in_buff = new T[ 2*BUFFER_LEN ];
	T* out_buff = new T[ 2*BUFFER_LEN ];
	double max = -std::numeric_limits<double>::infinity();
	unsigned int nb_sample_read;
	while( (nb_sample_read = fread( in_buff, 2*sizeof(*in_buff), BUFFER_LEN, fd_input)) > 0 ) {
		for ( unsigned int i = 0; i < nb_sample_read; i++ ) {
			const std::complex<double> c (in_buff[ 2*i ], in_buff[ 2*i+1 ]);
			double n = std::abs( c );
			if ( n > max ) {
				max = n;
			}
		}
		for ( unsigned int i = 0; i < nb_sample_read; i++ ) {
			out_buff[ 2*i ] = max_norm * in_buff[ 2*i ] / max;
			out_buff[ 2*i+1 ] = max_norm * in_buff[ 2*i+1 ] / max;
		}
		fwrite( out_buff, 2*sizeof(*out_buff), nb_sample_read, fd_output );
	}
	delete[] out_buff;
	delete[] in_buff;
}

int main(int argc, char** argv)
{
	if ( argc < 2 ) {
		std::cerr << "Usage: " << argv[0] << " <OPTIONS>\n"
			"  -m <MAX_VALUE> (default: 0)\n"
			"  -t <SIGNAL_TYPE> : scalar | iq (default: iq)\n"
			"  -d <DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i16)\n"
			"  -i <INPUT_CAPTURE_FILE> (default: -)\n"
			"  -o <OUTPUT_CAPTURE_FILE> (default: -)\n";
		return 1;
	}
        const std::string prog_name = argv[0];
	double max_value = 0;
	std::string data_format = "i16";
	std::string signal_type = "iq";
	const char* input_capture_file = "-";
	const char* output_capture_file = "-";
	for ( int i = 1; i < argc; i += 2 ) {
		std::string arg = argv[i];
	        if ( arg == "-m" ) {
			max_value = atof( argv[i+1] );
		} else if ( arg == "-t" ) {
			signal_type = argv[i+1];
		} else if ( arg == "-d" ) {
			data_format = argv[i+1];
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
	if ( signal_type != "scalar" && signal_type != "iq" ) {
		std::cerr << prog_name << " : ERROR: please set a valid signal type !\n";
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
        if ( signal_type == "scalar" ) {
		if      ( data_format == "i8"  ) { normalize_scalar<char>( max_value, fd_input, fd_output); }
		else if ( data_format == "i16" ) { normalize_scalar<short>( max_value, fd_input, fd_output); }
		else if ( data_format == "i32" ) { normalize_scalar<int>( max_value, fd_input, fd_output); }
		else if ( data_format == "f32" ) { normalize_scalar<float>( max_value, fd_input, fd_output); }
		else if ( data_format == "f64" ) { normalize_scalar<double>( max_value, fd_input, fd_output); }
	} else {
		if      ( data_format == "i8"  ) { normalize_iq<char>( max_value, fd_input, fd_output); }
		else if ( data_format == "i16" ) { normalize_iq<short>( max_value, fd_input, fd_output); }
		else if ( data_format == "i32" ) { normalize_iq<int>( max_value, fd_input, fd_output); }
		else if ( data_format == "f32" ) { normalize_iq<float>( max_value, fd_input, fd_output); }
		else if ( data_format == "f64" ) { normalize_iq<double>( max_value, fd_input, fd_output); }
	}
	return 0;
}
