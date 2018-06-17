iq_progs := iq_conv iq_deemphasis iq_demodfreq iq_mix iq_modfreq iq_normalize iq_phasis iq_preemphasis
iq_progs := $(iq_progs:%=bin/%)

CXXFLAGS := -Wall -Werror -O3

all: $(iq_progs) bin/iq_decimate

bin/iq_decimate: iq_decimate.cpp
	make -C fir/
	g++ -o $@ $< fir/fir.o -Ifir/ $(CXXFLAGS)

$(iq_progs): bin/iq_%: iq_%.cpp
	g++ -o $@ $< $(CXXFLAGS)

clean:
	make -C fir/ clean
	rm -f $(iq_progs) bin/iq_decimate
