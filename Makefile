TARGET = daisy

DSP_LIB = \
biquad.cpp \
iir_filter.cpp \
iir_peak.cpp.cpp \
util.cpp

METRONOME_LIB = \
onset_detection.cpp \
phase_extraction.cpp \
tempo_extraction.cpp

CPP_SOURCES = main.cpp DSP_LIB METRONOME_LIB

LIBDAISY_DIR = ../../libDaisy
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
