FLAGS = -W -Wall -Wextra -pedantic -std=c++11 -O3
INCLUDES = -I../.. -I../advection -I$$HOME/include -L$$HOME/lib -lboost_program_options -lzoltan
CXX = mpic++ $(FLAGS)

PROGRAMS = neighbor_list_length game_of_life advection

HEADERS = \
	../../dccrg.hpp \
	../../dccrg_types.hpp \
	../../dccrg_topology.hpp \
	../../dccrg_length.hpp \
	../../dccrg_cartesian_geometry.hpp \
	../advection/adapter.hpp \
	../advection/cell.hpp \
	../advection/initialize.hpp \
	../advection/save.hpp \
	../advection/solve.hpp \
	../game_of_life/cell.hpp \
	../game_of_life/initialize.hpp \
	../game_of_life/save.hpp \
	../game_of_life/solve.hpp

all: $(PROGRAMS)

neighbor_list_length: neighbor_list_length.cpp $(HEADERS) Makefile
	$(CXX) -DDEBUG neighbor_list_length.cpp $(INCLUDES) -o neighbor_list_length

game_of_life: game_of_life.cpp $(HEADERS) Makefile
	$(CXX) -DDEBUG game_of_life.cpp $(INCLUDES) -o game_of_life

advection: advection.cpp $(HEADERS) ../advection/cell.hpp ../advection/save.hpp Makefile
	$(CXX) advection.cpp $(INCLUDES) -o advection

d: data
data:
	rm -f *.dc *.vtk *.visit

c: clean
clean: data
	rm -f $(PROGRAMS)

