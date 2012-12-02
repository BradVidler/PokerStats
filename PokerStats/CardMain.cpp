#include <iostream>
#include <vector>
#include <string>
#include <mpi.h>
#include <algorithm>  // for random_shuffle
#include <cassert>
#include <ctime>   // for srand to seed the random # generator in random_shuffle
#include "Card.h"

typedef vector<Card> Deck;
// PlayerHands is a vector of the vector<Card>
typedef vector<Deck> PlayerHands;
// put the MyFunctions.h under the typedef so that printDeck knows what a deck is
#include "MyFunctions.h"

double handArray[10] = {0,0,0,0,0,0,0,0,0,0};

void terminateSlaves( int numProcs )
{
	int msgBuff = 0;

	for(int p = 1; p < numProcs; p++)
		MPI_Send(&msgBuff, 1, MPI_INT, p, TAG_QUIT, MPI_COMM_WORLD);
}

//receive the hand frequencies from each process
void receiveHands( int& activeCount )
{
	//cout <<"Receive Hands! -> " << endl;
	static MPI_Request request;
	static int recvFlag;
	MPI_Status status;
	// Message passing variables
	int msgBuff[MSG_SIZE];
	double dubBuff[10];
	bool handDone = false;

	//loop through each process and get the hand frequencies back
	for (int i = 0; i < activeCount; i++)
	{
		handDone = false;
		cout << "Sending request for hand data to process " << i+1 << endl;
		MPI_Send(&msgBuff, 1, MPI_INT, i+1, TAG_HAND, MPI_COMM_WORLD);
		if( !request && activeCount > 0 )
		{
			MPI_Irecv(&dubBuff, 10, MPI_DOUBLE, MPI_ANY_SOURCE, TAG_HAND, MPI_COMM_WORLD, &request);
		}
		do
		{
			//cout << "Done Waiting!" << endl;
			if( request )
			{
				// Already listening for a message
			
				// Test to see if message has been received
				MPI_Test( &request, &recvFlag, &status );
				//cout <<"Master testing a message! -> " << status.MPI_TAG << endl;
				if( recvFlag )
				{
					// Message received
					//cout <<"Master Received a message! -> " << status.MPI_TAG << endl;
					if( status.MPI_TAG == TAG_HAND )
					{
						handDone = true;

						cout << "Entering hand loop" << endl;
						//do what we have to do with the data. AKA tally up
						for(int m = 0; m < 10; m++)
						{
							//only prints the data for now
							cout << dubBuff[m] << endl;
						}
					}

					// Reset the request handle
					request = 0;
				}
			}
		}while (handDone == false);
	}
	terminateSlaves( activeCount+1 );
}

//check if any of the slaves send a "done" message, telling us to gather the hand frequencies
void checkMessagesFromSlaves( int& activeCount )
{
	static MPI_Request request;
	static int recvFlag;
	MPI_Status status;
	int msgBuff[MSG_SIZE];
	double dubBuff[10]; //holds the hand frequencies

	if( request )
	{
		// Already listening for a message

		// Test to see if message has been received
		MPI_Test( &request, &recvFlag, &status );

		if( recvFlag )
		{
			// Message received
			if( status.MPI_TAG == TAG_DONE )
			{
				request = 0;
				cout <<"Master Received done tag! -> " << status.MPI_TAG << endl;
				receiveHands(activeCount);
				activeCount = 0;
			}

			// Reset the request handle
			request = 0;
		}
	}

	if( !request && activeCount > 0 )
		// Start listening again
		MPI_Irecv(&msgBuff, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
}

void processMaster(int numProcs)
{
	int activeCount = numProcs - 1;

	//Wait for message saying all work is done
	// Checks for a message from each slave 
	while( activeCount > 0 )
	{
		checkMessagesFromSlaves( activeCount );
	}
}

int main(int argc, char *argv[])
{
	if( MPI_Init(&argc, &argv) == MPI_SUCCESS )
	{
		int numProcs, rank;
		MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		//DELETE THIS LATER
		//numProcs = 2;

		if( numProcs < 2 )
		{
			cout << "This program must be launched with at least two processes!" << endl;
		}
		else
		{

			//if( rank == 0 )
			//	processMaster( numProcs );
			//else
			//	createDeckAndGetHand( numProcs );
		}

		MPI_Finalize();
	}

	
}

