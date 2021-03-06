# to compile and test pick one makefile from makefiles directory,
# or write a new one for your environment, and put it here:
ENVIRONMENT_MAKEFILE = makefiles/homedir

#
# The lines below are not intended to be modified by users
#
CXXFLAGS = -std=c++11 -W -Wall -Wextra -pedantic -O3
include $(ENVIRONMENT_MAKEFILE)

# filled by project makefiles
EXECUTABLES =
TESTS =
RESULTS =
CLEAN =

default: all

DCCRG_HEADERS = \
  dccrg_cartesian_geometry.hpp \
  dccrg_get_cell_datatype.hpp \
  dccrg.hpp \
  dccrg_length.hpp \
  dccrg_mapping.hpp \
  dccrg_mpi_support.hpp \
  dccrg_no_geometry.hpp \
  dccrg_stretched_cartesian_geometry.hpp \
  dccrg_topology.hpp \
  dccrg_types.hpp

include \
  examples/project_makefile \
  tests/init/project_makefile \
  tests/indices/project_makefile \
  tests/get_cell_datatype/project_makefile \
  tests/constructors/project_makefile \
  tests/additional_cell_data/project_makefile \
  tests/mpi_support/project_makefile \
  tests/get_cells/project_makefile \
  tests/geometry/project_makefile \
  tests/load_balancing/project_makefile \
  tests/refine/project_makefile \
  tests/game_of_life/project_makefile \
  tests/restart/project_makefile \
  tests/advection/project_makefile


all: $(EXECUTABLES)

# executes all tests
t: test
test: $(TESTS)

# removes all simulation results
r: results
results: $(RESULTS)

# removes all executables and other non-source files
c: clean
clean: results $(CLEAN)
