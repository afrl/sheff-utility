#pragma once

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include <atomic>
#include <iostream>
#include <vector>

namespace utility
{

	/// This describes all the variables needed for the MPIUtils.
	/// One instance of this will be created for MPIUtils as a static variable.
	struct MPIUtilsState
	{
		// Indicates if the class is intialized yet
		std::atomic<bool> initialized;
		// Local communicator for all processes that initialize this object
		// with the same "color"
		int local_comm;
		// Process rank in MPI_COMM_WORLD
		int global_rank;
		// Process rank in local_comm
		int local_rank;
		// Number of groups determined by MPI
		int num_groups;
		// This groups key to identify it
		int local_group_key;
		// A vector holding the global ranks for root each group (local_rank 0)
		std::vector<int> root_ranks;

		MPIUtilsState() { reset(); };

		void reset() {
			initialized = false;

			#ifdef HAVE_MPI
			local_comm = MPI_COMM_NULL;
			#endif

			global_rank = 0;
			local_rank = 0;
			num_groups = 0;
			local_group_key = 0;
			root_ranks.clear();
		}
	};

	class MPIUtils
	{
	public:
		/// Returns true if there are more than 1 rank in the MPI_COMM_WORLD.
		/// Returns false otherwise.  This can be called whether MPIUtils::init has
		/// been called or not.
		/// \attention If MPIUtils::init has not been called before this call, null
		/// command line arguments will be given to MPI for initialization.  Thus it
		/// will ignore any MPI specific parameters you provide on the command line.
		static bool is_distributed();

		/// Initializes MPI and creates a MPI communicator for the processes that
		/// call this method.  The communicators will be split up by group key.
		/// Every process with the same key will be placed in teh same comm.
		static void init(int *argc, char ***argv, const int group_key = 0, const bool quiet = true);

		/// Creates a barrier such that each process (in the respective communicator)
		/// will wait until every process gets to.
		static void barrier(const bool use_local_comm = true);

		/// This method will gather a vector from each process to the root process
		/// of the communicator specified.  If use_local_comm is used, then the data
		/// among each group of ranks with the same group_key when init was called
		/// will be sent to the root rank of the respective group.
		/// Specialization for fundamental types (i.e. sizeof(T) would yield the number of bytes needed)
		template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
		static std::vector<T> gather_vectors(const std::vector<T>& data_from_this_proc, const bool use_local_comm = true);

		/// Sends data from root of one group to the root of another group.  This allows
		/// two programs to communicate with each other using this interface.
		/// Note: does a blocking send
		/// Note: only applicable to fundamental types.
		template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
		static void send_to_group(const std::vector<T> data, int target_group_key);

		/// The receive call that complements send_to_group
		template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
		static void receive_from_group(std::vector<T>& data, int target_group_key);

		/// An accessor method to get global rank
		static const int get_global_rank() { return get_state().global_rank; }

		/// An accessor method to get local rank
		static const int get_local_rank() { return get_state().local_rank; }

		/// Returns the local communicator for the processors that call init with the same group_key
		static const int get_local_comm() { return get_state().local_comm; }

		/// Returns the number of ranks in the local communicator
		static const int get_num_local_ranks();

		static void finalize();

		// Prohibit construction of this class
		MPIUtils() = delete;
		MPIUtils(MPIUtils const&) = delete;
		void operator=(MPIUtils const&) = delete;

	private:

		static MPIUtilsState& get_state() {
			static MPIUtilsState state;
			return state;
		}
	};

	inline bool MPIUtils::is_distributed()
	{
	#ifdef HAVE_MPI
		MPIUtilsState& state = fea_io::MPIUtils::get_state();

		if (state.initialized == true)
		{
			return state.global_rank > 1;
		}
		else
		{
			MPI_Init(nullptr, nullptr);
			int global_rank;
			MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);
			return global_rank > 1;
		}
	#else
		return false;
	#endif
	}

	inline void MPIUtils::init(int * argc, char *** argv, const int group_key, const bool quiet)
	{
	#ifdef HAVE_MPI
		MPIUtilsState& state = fea_io::MPIUtils::get_state();

		// If the object has already been initialized, just return
		if (state.initialized == true)
			return;

		// Set the flag in the base class that indicates if the class is initialized
		state.initialized = true;

		// Initialize MPI, if it isn't already
		int is_initialized = 0;
		MPI_Initialized(&is_initialized);
		if (!is_initialized)
			MPI_Init(argc, argv);

		// Get rank
		MPI_Comm_rank(MPI_COMM_WORLD, &state.global_rank);

		// Get group handle for all_process_group from Comm_World
		// Each process started by the mpiexec must make the split call
		// Each program will have a "color" as it is called (called the
		// group key in this context), which is an identifying integer.
		// Every process with the same "color" will be put in the same group.
		// As a result, each program can have its own communicator.
		// Ordering will be
		state.local_group_key = group_key;
		MPI_Comm_split(MPI_COMM_WORLD, group_key, state.global_rank, &state.local_comm);

		// Get the resulting group for the communicator
		MPI_Group local_group;
		MPI_Comm_group(state.local_comm, &local_group);

		// Now we can get the rank of the process in its group.
		// This is more useful as each program will have operations
		// for its group based on its rank.
		MPI_Group_rank(local_group, &state.local_rank);

		// Free the group since we have the rank now
		MPI_Group_free(&local_group);

		// Figure out how many groups there are by taking a
		// sum of numbers from every process, with the root
		// (rank 0) of each group submitting 1 and the rest 0
		int group_contrib = 0;
		if (state.local_rank == 0)
			group_contrib = 1;
		MPI_Allreduce(&group_contrib, &state.num_groups, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

		// Output the number of groups
		if (state.global_rank == 0 && quiet == false)
			std::cout << state.num_groups << " groups were created by MPI_Comm_split." << std::endl;

		// Now lets get the ranks of the root processes of the other groups
		for (int i = 0; i < state.num_groups; i++) {
			// A summation is done to to find the root global rank of each each group.
			// If i is not the current group key or the local rank is not 0,
			// then just contribute 0 to the sum.  Otherwise contribute the
			// global_rank of the process.
			int rank_term = 0;
			if (i == group_key && state.local_rank == 0)
				rank_term = state.global_rank;

			// Do the reduce, and save result
			int result = 0;
			MPI_Allreduce(&rank_term, &result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
			state.root_ranks.push_back(result);
		}

		// Output the ranks of the root of each group
		if (state.global_rank == 0 && quiet == false) {
			std::cout << "Global ranks of the root of each group:" << std::endl;
			for (unsigned int i = 0; i < state.root_ranks.size(); i++)
				std::cout << "[" << i << "] " << state.root_ranks[i] << std::endl;
		}
	#endif
	}

	inline void MPIUtils::barrier(const bool use_local_comm)
	{
	#ifdef HAVE_MPI
		const MPIUtilsState& state = fea_io::MPIUtils::get_state();

		// Select which comm to use for all MPI operations in this call
		MPI_Comm comm = use_local_comm ? state.local_comm : MPI_COMM_WORLD;
		MPI_Barrier(comm);
	#endif
	}

	template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
	inline std::vector<T> MPIUtils::gather_vectors(const std::vector<T>& data_from_this_proc, const bool use_local_comm)
	{
	#ifdef HAVE_MPI
		const MPIUtilsState& state = fea_io::MPIUtils::get_state();

		// Select which comm to use for all MPI operations in this call
		MPI_Comm comm = use_local_comm ? state.local_comm : MPI_COMM_WORLD;
		// Get information about the MPI communicator
		int comm_size;
		MPI_Comm_size(comm, &comm_size);
		int comm_rank;
		MPI_Comm_rank(comm, &comm_rank);

		const int size_of_type = sizeof(T);
		const int num_values_for_this_proc = static_cast<int>(data_from_this_proc.size());
		const int num_bytes_for_this_proc = num_values_for_this_proc * size_of_type;

		if (comm_rank == 0) {
			// Figure out the size of the data on each rank that will be communciated
			std::vector<int> num_values_for_each_proc(comm_size);
			MPI_Gather(&num_values_for_this_proc, 1, MPI_INT,
				&num_values_for_each_proc[0], 1, MPI_INT,
				0, comm);

			// Compute the number of bytes to expect from each rank and
			// the offset in the buffer for each rank.
			std::vector<int> recv_counts(comm_size);
			std::vector<int> recv_offsets(comm_size + 1);
			int offset = 0;
			for (auto i = 0; i < comm_size; i++) {
				recv_counts[i] = num_values_for_each_proc[i] * size_of_type;
				recv_offsets[i] = offset;
				offset += recv_counts[i];
			}
			recv_offsets.back() = offset;

			// Allocate a buffer to put the data in
			int num_total_values_needed = 0;
			for (auto num_values : num_values_for_each_proc)
				num_total_values_needed += num_values;
			std::vector<T> recv_buffer(num_total_values_needed);

			// Note: num_bytes_for_this_proc, recv_counts, and recv_offsets must all be
			// in terms of the bytes (not number of values)
			MPI_Gatherv(&data_from_this_proc[0], num_bytes_for_this_proc, MPI_CHAR,
				&recv_buffer[0], &recv_counts[0], &recv_offsets[0], MPI_CHAR,
				0, comm);

			return recv_buffer;
		}
		else {
			// Figure out the size of the data on each rank that will be communciated
			MPI_Gather(&num_values_for_this_proc, 1, MPI_INT,
				nullptr, 1, MPI_INT,
				0, comm);

			// Send the data to the root
			MPI_Gatherv(&data_from_this_proc[0], num_bytes_for_this_proc, MPI_CHAR,
				nullptr, nullptr, nullptr, MPI_CHAR,
				0, comm);

			// Just return an empty container for all by the root rank
			return std::vector<T>();
		}
	#else
		return data_from_this_proc;
	#endif
	}

	template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
	inline void MPIUtils::send_to_group(const std::vector<T> data, int target_group_key)
	{
	#ifdef HAVE_MPI
		const MPIUtilsState& state = fea_io::MPIUtils::get_state();

		// Only let the local root send information
		if (state.local_rank != 0)
			return;

		// Send as a char array.  It is up to the other program to receive with the correct
		// templated type.
		MPI_Send(&data[0], data.size() * sizeof(T), MPI_CHAR, state.root_ranks[target_group_key], 0, MPI_COMM_WORLD);
	#endif
	}

	template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
	inline void MPIUtils::receive_from_group(std::vector<T>& data, int target_group_key)
	{
	#ifdef HAVE_MPI
		const MPIUtilsState& state = fea_io::MPIUtils::get_state();

		// Only let the local root send information
		if (state.local_rank != 0)
			return;

		// Probe the message to see the size of the data it is about to receive
		MPI_Status status;
		MPI_Probe(state.root_ranks[target_group_key], 0, MPI_COMM_WORLD, &status);

		// Get the count from the status
		int num_bytes = 0;
		MPI_Get_count(&status, MPI_CHAR, &num_bytes);
		int num_values = num_bytes / sizeof(T);
		data.resize(num_values);

		// Do the receive
		MPI_Recv(&data[0], data.size() * sizeof(T), MPI_CHAR, state.root_ranks[target_group_key], 0, MPI_COMM_WORLD, &status);
	#endif
	}

	inline const int MPIUtils::get_num_local_ranks()
	{
	#ifdef HAVE_MPI
		const MPIUtilsState& state = fea_io::MPIUtils::get_state();
		int comm_size;
		MPI_Comm_size(state.local_comm, &comm_size);
		return comm_size;
	#else
		return 0;
	#endif
	}

	inline void MPIUtils::finalize()
	{
	#ifdef HAVE_MPI
		MPIUtilsState& state = fea_io::MPIUtils::get_state();
		state.reset();

		MPI_Finalize();
	#endif
	}
}