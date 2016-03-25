DIR=/usr/local
LIBS= \
   -lgecodeflatzinc  -lgecodedriver \
   -lgecodegist      -lgecodesearch \
   -lgecodeminimodel -lgecodeset    \
   -lgecodefloat     -lgecodeint    \
   -lgecodekernel    -lgecodesupport

all: box-wrapping.cpp
	g++ -I$(DIR)/include -c box-wrapping.cpp
	g++ -L$(DIR)/lib -o box-wrapping box-wrapping.o $(LIBS)

clean:
	rm box-wrapping *.o *~
