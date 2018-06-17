#!/usr/bin/python
# -*- coding: utf-8 -*-
# -*- mode: Python -*-

"""
  ===========================================================================

  Copyright (C) 2018 Emvivre

  This file is part of IQ_PSD.

  IQ_PSD is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  IQ_PSD is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with IQ_PSD.  If not, see <http://www.gnu.org/licenses/>.

  ===========================================================================
*/
"""

import numpy as np
import matplotlib
import scipy
import sys

INPUT_DATA_FORMAT_TO_NUMPY_DATA_FORMAT = {
    'i8': 'int8',
    'i16': 'int16',
    'i32': 'int32',
    'f32': 'float32',
    'f64': 'float64'
}

prog_name = sys.argv[0]
sample_rate = -1
signal_type = 'iq'
data_format = 'i16'
output_png = ''
input_capture_file = '-'

if len(sys.argv) < 3:
    print('Usage: %s <OPTIONS>\n'
          '  -s <SAMPLE_RATE>\n'
          '  -t <SIGNAL_TYPE> : scalar | iq (default: iq)\n'
          '  -d <DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i16)\n'
          '  -o <OUTPUT_PNG/PDF> (default: unused, screen display)\n'
          '  -i <INPUT_CAPTURE_FILE> (default: -)'
          % sys.argv[0])
    quit(1)
for i in range(1,len(sys.argv),2):
    if sys.argv[i] == '-s':
        sample_rate = int(float(sys.argv[i+1]))
    elif sys.argv[i] == '-t':
        signal_type = sys.argv[i+1]
    elif sys.argv[i] == '-d':
        data_format = sys.argv[i+1]
    elif sys.argv[i] == '-o':
        output_png = sys.argv[i+1]
    elif sys.argv[i] == '-i':
        input_capture_file = sys.argv[i+1]

if sample_rate == -1:
    print('%s : ERROR: please set a valid sample_rate !' % prog_name)
    quit(1)
if signal_type not in ['scalar', 'iq']:
    print('%s : ERROR: please set a valid signal type !\n' % prog_name)
    quit(1)
if data_format not in ['i8', 'i16', 'i32', 'f32', 'f64']:
    print('%s : ERROR: please set a valid data format !' % prog_name)
    quit(1)
if output_png != '':
    matplotlib.use('Agg')

import matplotlib.pyplot as plt

Fs=sample_rate
dtype = INPUT_DATA_FORMAT_TO_NUMPY_DATA_FORMAT[ data_format ]
if input_capture_file != '-':
    samples = np.fromfile(input_capture_file, dtype=dtype)
else:
    samples = np.fromstring(sys.stdin.read(), dtype=dtype)
if signal_type == 'iq':
    samples = samples[0::2] + 1j*samples[1::2]
    x = np.array(samples).astype("complex64")
    plt.psd(x, Fs=Fs)
else:
    x = np.array(samples).astype("float64")
    plt.psd(x, Fs=Fs)
if output_png != '':
    plt.savefig(output_png)
else:
    plt.show()
plt.close()
