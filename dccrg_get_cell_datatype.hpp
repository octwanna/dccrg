/*
Functions for dccrg to obtain the MPI Datatype from cell data.

Copyright 2014, 2015, 2016 Ilja Honkonen

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


#ifndef DCCRG_GET_CELL_DATATYPE_HPP
#define DCCRG_GET_CELL_DATATYPE_HPP


#include "cstdint"
#include "tuple"
#include "type_traits"

#include "mpi.h"

#include "boost/function_types/property_tags.hpp"
#include "boost/mpl/vector.hpp"
#include "boost/tti/has_member_function.hpp"


namespace dccrg {
namespace detail {


BOOST_TTI_HAS_MEMBER_FUNCTION(get_mpi_datatype)


/*!
Returns the MPI transfer info from given cell.

Version for get_mpi_datatype(const uint64_t, ..., const int) const.
*/
template<
	class Cell_T
> typename std::enable_if<
	has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<
			const uint64_t,
			const int,
			const int,
			const bool,
			const int
		>,
		boost::function_types::const_qualified
	>::value,
	std::tuple<
		void*,
		int,
		MPI_Datatype
	>
>::type get_cell_mpi_datatype(
	const Cell_T& cell,
	const uint64_t cell_id,
	const int sender,
	const int receiver,
	const bool receiving,
	const int neighborhood_id
) {
	return cell.get_mpi_datatype(
		cell_id,
		sender,
		receiver,
		receiving,
		neighborhood_id
	);
}


/*!
Returns the MPI transfer info from given cell.

Version for get_mpi_datatype(const uint64_t, ..., const int).
*/
template<
	class Cell_T
> typename std::enable_if<
	has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<
			const uint64_t,
			const int,
			const int,
			const bool,
			const int
		>
	>::value,
	std::tuple<
		void*,
		int,
		MPI_Datatype
	>
>::type get_cell_mpi_datatype(
	Cell_T& cell,
	const uint64_t cell_id,
	const int sender,
	const int receiver,
	const bool receiving,
	const int neighborhood_id
) {
	return cell.get_mpi_datatype(
		cell_id,
		sender,
		receiver,
		receiving,
		neighborhood_id
	);
}


/*!
Returns the MPI transfer info from given cell.

Version for get_mpi_datatype() const.
Gives precedence to get_mpi_datatype which takes arguments.
*/
template<
	class Cell_T
> typename std::enable_if<
	has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<>,
		boost::function_types::const_qualified
	>::value
	and not
	has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<
			const uint64_t,
			const int,
			const int,
			const bool,
			const int
		>,
		boost::function_types::const_qualified
	>::value,
	std::tuple<
		void*,
		int,
		MPI_Datatype
	>
>::type get_cell_mpi_datatype(
	const Cell_T& cell,
	const uint64_t /*cell_id*/,
	const int /*sender*/,
	const int /*receiver*/,
	const bool /*receiving*/,
	const int /*neighborhood_id*/
) {
	return cell.get_mpi_datatype();
}


/*!
Returns the MPI transfer info from given cell.

Version for get_mpi_datatype().
Gives precedence to get_mpi_datatype which takes arguments.
*/
template<
	class Cell_T
> typename std::enable_if<
	has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<>
	>::value
	and not
	has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<
			const uint64_t,
			const int,
			const int,
			const bool,
			const int
		>
	>::value,
	std::tuple<
		void*,
		int,
		MPI_Datatype
	>
>::type get_cell_mpi_datatype(
	Cell_T& cell,
	const uint64_t /*cell_id*/,
	const int /*sender*/,
	const int /*receiver*/,
	const bool /*receiving*/,
	const int /*neighborhood_id*/
) {
	return cell.get_mpi_datatype();
}


/*!
Returns a human-readable error message if cell type doesn't have suitable get_mpi_datatype().
*/
template<
	class Cell_T
> typename std::enable_if<
	not has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<
			const uint64_t,
			const int,
			const int,
			const bool,
			const int
		>,
		boost::function_types::const_qualified
	>::value
	and not has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<
			const uint64_t,
			const int,
			const int,
			const bool,
			const int
		>
	>::value
	and not has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<>,
		boost::function_types::const_qualified
	>::value
	and not has_member_function_get_mpi_datatype<
		Cell_T,
		std::tuple<void*, int, MPI_Datatype>,
		boost::mpl::vector<>
	>::value,
	std::tuple<
		void*,
		int,
		MPI_Datatype
	>
>::type get_cell_mpi_datatype(
	Cell_T& cell,
	const uint64_t /*cell_id*/,
	const int /*sender*/,
	const int /*receiver*/,
	const bool /*receiving*/,
	const int /*neighborhood_id*/
) {
	static_assert(
		not std::is_same<Cell_T, Cell_T>::value,
		"No suitable get_mpi_datatype() member function found in cell type given to dccrg"
	);
	return std::make_tuple(nullptr, -1, MPI_DATATYPE_NULL);
}


}} // namespaces

#endif

