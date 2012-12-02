#include <iostream>
#include <vector>
#include <string>
#include <mpi.h>
#include <algorithm>  // for random_shuffle
#include <cassert>
#include <ctime>   // for srand to seed the random # generator in random_shuffle
#include "Card.h"

typedef vector<Card> Deck;
typedef vector<Deck> PlayerHands;

// put the MyFunctions.h under the typedef so that printDeck knows what a deck is
#include "MyFunctions.h"

//double handArray[10] = {0,0,0,0,0,0,0,0,0,0};

int main(int argc, char *argv[])
{
	//1. All processes initialize MPI at the start and obtain the number 
	//of processes and the rank of the current process.
	if( MPI_Init(&argc, &argv) == MPI_SUCCESS )
	{
		int numProcs, rank;
		MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		//starts the main loop for EVERY process
		createPokerHands( numProcs, rank );
		
		//12. All processes finalize MPI.
		MPI_Finalize();
	}
}