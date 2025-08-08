#include <cstdlib> // For atoi, rand, srand
#include <ctime>   // For time
#include <mpi.h>

void walker_process();
void controller_process();

int domain_size;
int max_steps;
int world_rank;
int world_size;

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes and the rank of this process
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc != 3)
    {
        if (world_rank == 0)
        {
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <domain_size> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    domain_size = atoi(argv[1]);
    max_steps = atoi(argv[2]);

    if (world_rank == 0)
    {
        // Rank 0 is the controller
        controller_process();
    }
    else
    {
        // All other ranks are walkers
        walker_process();
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}

// Function for each walker
void walker_process()
{
    std::srand(std::time(nullptr) + world_rank);

    int position = 0;  // Starting at the center
    int steps = 0;

    // Walk loop
    while (steps < max_steps) {
        int move;
        
        if (std::rand() % 2 == 0)
            move = -1;
        else
            move = 1;
        position += move;
        steps++;
        
        if (position <= -domain_size || position >= domain_size)
            break;
    }

    // Send steps to controller
    MPI_Send(&steps, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    
    // Print walker's own message
    std::cout << "Rank " << world_rank << ": Walker finished in " << steps << "." << std::endl;
}

// Function for controller to receive messages
void controller_process()
{
    int num_walkers = world_size - 1;
    int received_count = 0;

    // Receive all messages from walkers
    while (received_count < num_walkers) {
        int steps_taken;        
        MPI_Status status;    

        MPI_Recv(&steps_taken, 1, MPI_INT,
                 MPI_ANY_SOURCE, 0,
                 MPI_COMM_WORLD, &status);
        
        received_count++;
    }

    // Print completion message with exact expected format
    std::cout << "All " << num_walkers << " walkers have finished" << std::endl;
}
