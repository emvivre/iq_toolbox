/*
  ===========================================================================

  Copyright (C) 2018 Emvivre

  This file is part of IQ_DEEMPHASIS.

  IQ_DEEMPHASIS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  IQ_DEEMPHASIS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with IQ_DEEMPHASIS.  If not, see <http://www.gnu.org/licenses/>.

  ===========================================================================
*/

#include <iostream>
#include <complex>

static const double PI = 4 * std::atan(1);

template <class T>
void deemphasis_scalar( const unsigned int sample_rate, const double a, const double b, FILE* fd_input, FILE* fd_output )
{
        T* in_buff = new T[ sample_rate ];
        T* out_buff = new T[ sample_rate ];
	double x_prev = 0;
	double y_prev = 0;
        while( fread( in_buff, sizeof(*in_buff), sample_rate, fd_input) == sample_rate ) {
                for ( unsigned int i = 0; i < sample_rate; i++ ) {
			double x = in_buff[ i ];
			double y = a * x + a * x_prev + b * y_prev;
                        out_buff[ i ] = y;
			y_prev = y;
			x_prev = x;
                }
                fwrite( out_buff, sizeof(*out_buff), sample_rate, fd_output );
                fflush( fd_output );
        }
        delete[] out_buff;
        delete[] in_buff;
}

template <class T>
void deemphasis_iq( const unsigned int sample_rate, const double a, const double b, FILE* fd_input, FILE* fd_output )
{
        T* in_buff = new T[ 2 * sample_rate ];
        T* out_buff = new T[ 2 * sample_rate ];
	std::complex< double > x_prev( 0, 0 );
	std::complex< double > y_prev( 0, 0 );
        while( fread( in_buff, 2*sizeof(*in_buff), sample_rate, fd_input) == sample_rate ) {
                for ( unsigned int i = 0; i < sample_rate; i++ ) {
			std::complex<double> x( in_buff[ 2*i ], in_buff[ 2*i+1 ] );
			std::complex<double> y = a * x + a * x_prev + b * y_prev;
			out_buff[ 2*i ] = y.real();
			out_buff[ 2*i+1 ] = y.imag();
			y_prev = y;
			x_prev = x;
                }
                fwrite( out_buff, 2*sizeof(*out_buff), sample_rate, fd_output );
        }
        delete[] out_buff;
        delete[] in_buff;
}



int main(int argc, char** argv)
{
	if ( argc < 2 ) {
		std::cerr << "Usage: " << argv[0] << " <OPTIONS>\n"
			"  -s <SAMPLE_RATE>\n"
			"  -r <RC_TIME_CONSTANT> (default: 50e-6)\n"
			"  -t <SIGNAL_TYPE> : scalar | iq (default: scalar)\n"
			"  -d <DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: f32)\n"
			"  -i <INPUT_CAPTURE_FILE> (default: -)\n"
			"  -o <OUTPUT_CAPTURE_FILE> (default: -)\n";
		return 1;
	}
        const std::string prog_name = argv[0];
        unsigned int sample_rate = 0;
	double tau = 50e-6;
        std::string data_format = "f32";
        std::string signal_type = "scalar";
        const char* input_capture_file = "-";
        const char* output_capture_file = "-";
        for ( int i = 1; i < argc; i += 2 ) {
                std::string arg = argv[i];
                if ( arg == "-s" ) {
                        sample_rate = atof( argv[i+1] );
                } else if ( arg == "-r" ) {
                        tau = atof( argv[i+1] );
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
        if ( tau == 0 ) {
                std::cerr << prog_name << " : ERROR: please set a valid rc time constant !\n";
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
	const double T = 1. / sample_rate;
	const double tau_p = T / ( 2 * std::tan( T / ( 2 * tau ) ) );
	const double a = T / (T + 2 * tau_p);
	const double b = -(T - 2 * tau_p) / (T + 2 * tau_p);
        if ( signal_type == "scalar" ) {
                if      ( data_format == "i8"  ) { deemphasis_scalar<char>( sample_rate, a, b, fd_input, fd_output); }
                else if ( data_format == "i16" ) { deemphasis_scalar<short>( sample_rate, a, b, fd_input, fd_output); }
                else if ( data_format == "i32" ) { deemphasis_scalar<int>( sample_rate, a, b, fd_input, fd_output); }
                else if ( data_format == "f32" ) { deemphasis_scalar<float>( sample_rate, a, b, fd_input, fd_output); }
                else if ( data_format == "f64" ) { deemphasis_scalar<double>( sample_rate, a, b, fd_input, fd_output); }
        } else {
                if      ( data_format == "i8"  ) { deemphasis_iq<char>( sample_rate, a, b, fd_input, fd_output); }
                else if ( data_format == "i16" ) { deemphasis_iq<short>( sample_rate, a, b, fd_input, fd_output); }
                else if ( data_format == "i32" ) { deemphasis_iq<int>( sample_rate, a, b, fd_input, fd_output); }
                else if ( data_format == "f32" ) { deemphasis_iq<float>( sample_rate, a, b, fd_input, fd_output); }
                else if ( data_format == "f64" ) { deemphasis_iq<double>( sample_rate, a, b, fd_input, fd_output); }
        }
        return 0;
}
