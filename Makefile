CPP=g++ -c -O2 -ggdb
objects=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
headers=$(wildcard *.h)

all: graphanalysis

graphanalysis: $(objects)
	@echo Linking $@
	@g++ -o $@ $(objects) -lboost_filesystem -lboost_regex

%.o: %.cpp $(headers)
	@echo Compiling $<
	@$(CPP) -c $< -o $@

clean:
	rm graphanalysis $(objects)

distclean: clean
	rm callgrind.out.*
