LIB=lib/libverilog.so
INC=-I./include
CXX=g++

default: main

main: libs
	$(CXX) $(INC) src/main.cpp $(LIB) -o generate_code.out

libs:
	$(CXX) src/verilog.cpp src/multiplier.cpp -fPIC -shared -o lib/libverilog.so $(INC)

all: libs main

example: libs
	$(CXX) $(INC) src/example.cpp $(LIB) -o example_code.out


clean:
	rm -rf lib/*
	rm -rf generated_codes/*.v
	rm -rf *.out