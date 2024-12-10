CPLEXPATH = /opt/ibm/ILOG/CPLEX_Studio129

CPLEXFLAGS=-O3 -m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD -I$(CPLEXPATH)/cplex/include -I$(CPLEXPATH)/concert/include  -L$(CPLEXPATH)/cplex/lib/x86-64_linux/static_pic -lilocplex -lcplex -L$(CPLEXPATH)/concert/lib/x86-64_linux/static_pic -lconcert -lm -pthread -std=c++0x -ldl

CFLAGS=-std=c++11 -static-libstdc++ -static-libgcc -Wall

all:
	g++ main.cpp -o run $(CPLEXFLAGS) $(CFLAGS) -O0 -g

clean:
	rm -f *.o
