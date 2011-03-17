.PHONY: statetrace

statetrace: statetrace-native

statetrace-native: statetrace.cc tracechild.cc tracechild_arch.cc printer.cc printer.hh refcnt.hh regstate.hh tracechild.hh
	g++ statetrace.cc tracechild.cc tracechild_arch.cc printer.cc -I ./ -I ./arch/ -O3 --static -o statetrace

statetrace-sparc: statetrace.cc tracechild.cc tracechild_arch.cc printer.cc printer.hh refcnt.hh regstate.hh tracechild.hh
	sparc64-unknown-linux-gnu-g++ statetrace.cc tracechild.cc tracechild_arch.cc printer.cc -g -I ./ -I ./arch/ -O3 --static -o statetrace
