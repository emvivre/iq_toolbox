/*
  ===========================================================================

  Copyright (C) 2018 Emvivre

  This file is part of IQ_PHASIS.

  IQ_PHASIS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  IQ_PHASIS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with IQ_PHASIS.  If not, see <http://www.gnu.org/licenses/>.

  ===========================================================================
*/

#include <iostream>
#include <complex>

static const unsigned int BUFFER_LEN = 200000;

template <class Input, class Output>
void phasis_( FILE* fd_input, FILE* fd_output )
{
	Input* in_buff = new Input[ 2*BUFFER_LEN ];
	Output* out_buff = new Output[ 2*BUFFER_LEN ];
	std::complex<double> c_prev(0,0);
	unsigned int nb_sample_read;
	while( (nb_sample_read = fread( in_buff, 2*sizeof(*in_buff), BUFFER_LEN, fd_input)) > 0 ) {
		for ( unsigned int i = 0; i < nb_sample_read; i++ ) {
			std::complex<double> c( in_buff[2*i], in_buff[2*i+1] );
		        double phasis = std::arg( c * std::conj(c_prev) );
			out_buff[ i ] = phasis;
			c_prev = c;
		}
		fwrite( out_buff, sizeof(*out_buff), nb_sample_read, fd_output );
                fflush( fd_output );
	}
	delete[] out_buff;
	delete[] in_buff;
}

template <class T>
void phasis( const std::string& output_data_format, FILE* fd_input, FILE* fd_output )
{
	if      ( output_data_format == "i8"  ) { phasis_<T,char>( fd_input, fd_output ); }
	else if ( output_data_format == "i16" ) { phasis_<T,short>( fd_input, fd_output ); }
	else if ( output_data_format == "i32" ) { phasis_<T,int>( fd_input, fd_output ); }
	else if ( output_data_format == "f32" ) { phasis_<T,float>( fd_input, fd_output ); }
	else if ( output_data_format == "f64" ) { phasis_<T,double>( fd_input, fd_output) ; }
}

int main(int argc, char** argv)
{
	if ( argc < 2 ) {
		std::cerr << "Usage: " << argv[0] << " <OPTIONS>\n"
			"  -d <INPUT_DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i16)\n"
			"  -D <OUTPUT_DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: f32)\n"
			"  -i <INPUT_CAPTURE_FILE> (default: -)\n"
			"  -o <OUTPUT_CAPTURE_FILE> (default: -)\n";
		return 1;
	}
        const std::string prog_name = argv[0];
	std::string data_format = "i16";
	std::string output_data_format = "f32";
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
	if      ( data_format == "i8"  ) { phasis<char>( output_data_format, fd_input, fd_output); }
	else if ( data_format == "i16" ) { phasis<short>( output_data_format, fd_input, fd_output); }
	else if ( data_format == "i32" ) { phasis<int>( output_data_format, fd_input, fd_output); }
	else if ( data_format == "f32" ) { phasis<float>( output_data_format, fd_input, fd_output); }
	else if ( data_format == "f64" ) { phasis<double>( output_data_format, fd_input, fd_output); }
	return 0;
}
