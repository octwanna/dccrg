/*
A program for general scalability testing of dccrg.

Copyright 2011, 2012, 2013, 2014,
2015, 2016 Finnish Meteorological Institute

Dccrg is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License version 3
as published by the Free Software Foundation.

Dccrg is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with dccrg. If not, see <http://www.gnu.org/licenses/>.
*/

#include "boost/program_options.hpp"
#include "cstdlib"
#include "ctime"
#include "fstream"
#include "functional"
#include "iostream"
#include "string"
#include "vector"

#include "mpi.h"
#include "zoltan.h"

#include "../../dccrg.hpp"


using namespace std;
using namespace dccrg;


class Cell
{
public:

	vector<uint8_t> data;

	static size_t data_size;

	Cell()
	{
		this->data.resize(Cell::data_size);
	}

	std::tuple<void*, int, MPI_Datatype> get_mpi_datatype()
	{
		return std::make_tuple(this->data.data(), this->data.size(), MPI_UINT8_T);
	}
};

size_t Cell::data_size = 0;


/*!
Returns the number of bytes that have to be transmitted in given list to other processes.
*/
size_t get_traffic_size(
	const std::unordered_map<int, std::vector<std::pair<uint64_t, int> > >& lists
) {
	double communication_size = 0;
	for (const auto& item: lists) {
		communication_size += sizeof(uint8_t) * Cell::data_size * item.second.size();
	}

	return communication_size;
}


/*!
Returns the amount of time in seconds spent "solving" given cells.
*/
template<class CellData> double solve(
	const double solution_time,
	const vector<uint64_t>& cells,
	Dccrg<CellData>& grid
) {
	const double start_time = MPI_Wtime();

	for (const auto& cell: cells) {
		auto* const cell_data = grid[cell];
		if (cell_data == NULL) {
			cerr << __FILE__ << ":" << __LINE__
				<< "No data for cell " << cell << endl;
			abort();
		}

		// "solve" for given amount of time
		const double end_time = MPI_Wtime() + solution_time;
		double elapsed_time = MPI_Wtime();
		while (elapsed_time < end_time) {
			elapsed_time = MPI_Wtime();
		}
	}

	return MPI_Wtime() - start_time;
}


int main(int argc, char* argv[])
{
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		cerr << "Coudln't initialize MPI." << endl;
		abort();
	}

	MPI_Comm comm = MPI_COMM_WORLD;

	int rank = 0, comm_size = 0;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &comm_size);

	float zoltan_version;
	if (Zoltan_Initialize(argc, argv, &zoltan_version) != ZOLTAN_OK) {
	    cout << "Zoltan_Initialize failed" << endl;
	    exit(EXIT_FAILURE);
	}

	/*
	Options
	*/
	size_t data_size;
	double solution_time;
	string load_balancer;
	int timesteps, maximum_refinement_level, neighborhood_size;
	uint64_t x_length, y_length, z_length;
	boost::program_options::options_description options("Usage: program_name [options], where options are:");
	options.add_options()
		("help", "print this help message")
		("data_size",
			boost::program_options::value<size_t>(&data_size)->default_value(1),
			"Amount of data in bytes in every cell of the grid")
		("solution_time",
			boost::program_options::value<double>(&solution_time)->default_value(0.001),
			"Amount of time in secods that it takes to \"solve\" one cell")
		("load_balancer",
			boost::program_options::value<string>(&load_balancer)->default_value("HYPERGRAPH"),
			"Load balancing function to use")
		("timesteps",
			boost::program_options::value<int>(&timesteps)->default_value(10),
			"Number of times to \"solve\" cells")
		("x_length",
			boost::program_options::value<uint64_t>(&x_length)->default_value(10),
			"Create a grid with arg number of unrefined cells in the x direction")
		("y_length",
			boost::program_options::value<uint64_t>(&y_length)->default_value(10),
			"Create a grid with arg number of unrefined cells in the y direction")
		("z_length",
			boost::program_options::value<uint64_t>(&z_length)->default_value(10),
			"Create a grid with arg number of unrefined cells in the z direction")
		("maximum_refinement_level",
			boost::program_options::value<int>(&maximum_refinement_level)->default_value(-1),
			"Maximum refinement level of the grid (0 == not refined, "
			"-1 == maximum possible for given lengths)")
		("neighborhood_size",
			boost::program_options::value<int>(&neighborhood_size)->default_value(1),
			"Size of a cell's neighborhood in cells of equal size "
			"(0 means only cells sharing a face are neighbors)");

	// read options from command line
	boost::program_options::variables_map option_variables;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), option_variables);
	boost::program_options::notify(option_variables);

	// print a help message if asked
	if (option_variables.count("help") > 0) {
		if (rank == 0) {
			cout << options << endl;
		}
		MPI_Finalize();
		return EXIT_SUCCESS;
	}

	// warn if requested solution time too small
	if (solution_time < MPI_Wtick()) {
		cout << "Warning: requested solution time is less than MPI_Wtime resolution, setting solution_time to: "
			<< MPI_Wtick()
			<< endl;
		solution_time = MPI_Wtick();
	}

	Cell::data_size = data_size;

	// initialize
	Dccrg<Cell> grid;

	const std::array<uint64_t, 3> grid_length{{x_length, y_length, z_length}};
	grid.initialize(
		grid_length,
		comm,
		load_balancer.c_str(),
		neighborhood_size,
		maximum_refinement_level
	);
	grid.balance_load();

	vector<uint64_t>
		inner_cells = grid.get_local_cells_not_on_process_boundary(),
		outer_cells = grid.get_local_cells_on_process_boundary();

	double total_solution_time = 0;
	double sends_size = 0, receives_size = 0;
	for (int timestep = 0; timestep < timesteps; timestep++) {

		const auto
			&sends	= grid.get_cells_to_send(),
			&receives = grid.get_cells_to_receive();

		sends_size += get_traffic_size(sends);
		receives_size += get_traffic_size(receives);

		grid.start_remote_neighbor_copy_updates();

		total_solution_time += solve<Cell>(solution_time, inner_cells, grid);
		grid.wait_remote_neighbor_copy_update_receives();

		total_solution_time += solve<Cell>(solution_time, outer_cells, grid);
		grid.wait_remote_neighbor_copy_update_sends();
	}

	for (int process = 0; process < comm_size; process++) {
		MPI_Barrier(comm);
		if (rank == process) {
			cout << "Process " << rank
				<< ": total solution time per timestep " << total_solution_time / timesteps
				<< ", total bytes sent per timestep " << sends_size / timesteps
				<< ", total bytes received per timestep " << receives_size / timesteps
				<< endl;
		}
		MPI_Barrier(comm);
	}

	double transferred_bytes_local = sends_size, transferred_bytes = 0;
	MPI_Reduce(&transferred_bytes_local, &transferred_bytes, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
	if (rank == 0) {
		cout << "Total transferred bytes per timestep: "
			<< transferred_bytes / timesteps
			<< endl;
	}

	MPI_Finalize();

	return EXIT_SUCCESS;
}

