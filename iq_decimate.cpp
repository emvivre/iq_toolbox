/*
  ===========================================================================

  Copyright (C) 2018 Emvivre

  This file is part of IQ_DECIMATE.

  IQ_DECIMATE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  IQ_DECIMATE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with IQ_DECIMATE.  If not, see <http://www.gnu.org/licenses/>.

  ===========================================================================
*/

#include <iostream>
#include <complex>
#include "fir.h"

static const double PI = 4 * std::atan(1);

template <class T>
void decimate_scalar( const unsigned int sample_rate, const unsigned int cutoff_frequency, FILE* fd_input, FILE* fd_output )
{
	const int nb_coef = 64;
	int dec_rate = int(sample_rate / cutoff_frequency);
	const unsigned int output_sample_rate = sample_rate / dec_rate;
	std::cerr << "output_sample_rate: " << output_sample_rate << "\n";
	double avg_coef[ nb_coef ];
	T* in_buff = new T[ nb_coef + sample_rate ];
	T* out_buff = new T[ output_sample_rate ];
	fir_gen( BLACKMAN_WINDOW_TYPE, nb_coef, sample_rate, cutoff_frequency, avg_coef, NULL, NULL );
	for ( int i = 0; i < nb_coef; i++ ) {
		in_buff[ i ] = 0;
	}
	while( fread( in_buff + nb_coef, sizeof(*in_buff), sample_rate, fd_input) == sample_rate ) {
		for ( unsigned int i = 0, j = 0; i < sample_rate; i += dec_rate, j++ ) {
			const T* p = in_buff + i;
			double avg = 0;
			for ( int k = 0; k < nb_coef; k++ ) {
				avg += avg_coef[ k ] * p[k];
			}
			out_buff[ j ] = avg;
		}
		for ( int i = 0; i < nb_coef; i++ ) {
			in_buff[ i ] = in_buff[ sample_rate + i ];
		}
		fwrite( out_buff, sizeof(*out_buff), output_sample_rate, fd_output );
                fflush( fd_output );
	}
	delete[] out_buff;
	delete[] in_buff;
}

template <class T>
void decimate_iq( const unsigned int sample_rate, const unsigned int cutoff_frequency, FILE* fd_input, FILE* fd_output )
{
	const int nb_coef = 64;
	int dec_rate = int(sample_rate / cutoff_frequency);
	const unsigned int output_sample_rate = sample_rate / dec_rate;
	std::cerr << "output_sample_rate: " << output_sample_rate << "\n";
	double avg_coef[ nb_coef ];
	T* in_buff = new T[ 2*(nb_coef + sample_rate) ];
	T* out_buff = new T[ 2*output_sample_rate ];
	fir_gen( BLACKMAN_WINDOW_TYPE, nb_coef, sample_rate, cutoff_frequency/2, avg_coef, NULL, NULL );
	for ( int i = 0; i < nb_coef; i++ ) {
		in_buff[ 2*i ] = 0;
		in_buff[ 2*i+1 ] = 0;
	}
	while( fread( in_buff + 2*nb_coef, 2*sizeof(*in_buff), sample_rate, fd_input) == sample_rate ) {
		for ( unsigned int i = 0, j = 0; i < sample_rate; i += dec_rate, j++ ) {
			const T* p = in_buff + 2*i;
			std::complex<double> avg(0, 0);
			for ( int k = 0; k < nb_coef; k++ ) {
				std::complex<double> c( p[2*k], p[2*k+1] );
				avg += avg_coef[ k ] * c;
			}
			out_buff[ 2*j ] = avg.real();
			out_buff[ 2*j+1 ] = avg.imag();
		}
		for ( int i = 0; i < nb_coef; i++ ) {
			in_buff[ 2*i ] = in_buff[ 2*sample_rate + 2*i ];
			in_buff[ 2*i+1 ] = in_buff[ 2*sample_rate + 2*i+1 ];
		}
		fwrite( out_buff, 2*sizeof(*out_buff), output_sample_rate, fd_output );
	}
	delete[] out_buff;
	delete[] in_buff;
}


int main(int argc, char** argv)
{
	if ( argc < 2 ) {
		std::cerr << "Usage: " << argv[0] << " <OPTIONS>\n"
			"  -s <SAMPLE_RATE>\n"
			"  -f <CUTOFF_FREQUENCY>\n"
			"  -t <SIGNAL_TYPE> : scalar | iq (default: iq)\n"
                        "  -d <DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i16)\n"
			"  -i <INPUT_CAPTURE_FILE> (default: -)\n"
			"  -o <OUTPUT_CAPTURE_FILE> (default: -)\n";
		return 1;
	}
        const std::string prog_name = argv[0];
	unsigned int sample_rate = 0;
	unsigned int cutoff_frequency = 0;
	std::string data_format = "i16";
	std::string signal_type = "iq";
	const char* input_capture_file = "-";
	const char* output_capture_file = "-";
	for ( int i = 1; i < argc; i += 2 ) {
		std::string arg = argv[i];
		if ( arg == "-s" ) {
			sample_rate = atof( argv[i+1] );
		} else if ( arg == "-f" ) {
			cutoff_frequency = atof( argv[i+1] );
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
	if ( cutoff_frequency == 0 ) {
		std::cerr << prog_name << " : ERROR: please set a valid cutoff frequency !\n";
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
		if      ( data_format == "i8"  ) { decimate_scalar<char>( sample_rate, cutoff_frequency, fd_input, fd_output); }
		else if ( data_format == "i16" ) { decimate_scalar<short>( sample_rate, cutoff_frequency, fd_input, fd_output); }
		else if ( data_format == "i32" ) { decimate_scalar<int>( sample_rate, cutoff_frequency, fd_input, fd_output); }
		else if ( data_format == "f32" ) { decimate_scalar<float>( sample_rate, cutoff_frequency, fd_input, fd_output); }
		else if ( data_format == "f64" ) { decimate_scalar<double>( sample_rate, cutoff_frequency, fd_input, fd_output); }
	} else {
		if      ( data_format == "i8"  ) { decimate_iq<char>( sample_rate, cutoff_frequency, fd_input, fd_output); }
		else if ( data_format == "i16" ) { decimate_iq<short>( sample_rate, cutoff_frequency, fd_input, fd_output); }
		else if ( data_format == "i32" ) { decimate_iq<int>( sample_rate, cutoff_frequency, fd_input, fd_output); }
		else if ( data_format == "f32" ) { decimate_iq<float>( sample_rate, cutoff_frequency, fd_input, fd_output); }
		else if ( data_format == "f64" ) { decimate_iq<double>( sample_rate, cutoff_frequency, fd_input, fd_output); }
	}
	return 0;
}
