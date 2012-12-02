enum handType{ ROYALFLUSH, STRAIGHTFLUSH, FOUROFAKIND, FULLHOUSE, FLUSH, STRAIGHT, THREEOFAKIND, TWOPAIR, PAIR, HIGHCARD };

#include <iomanip>

const int MSG_SIZE = 100;
const int TAG_DONE = 100, TAG_QUIT = 200, TAG_HAND = 300;
unsigned const CARDS_PER_HAND = 5;  // # of cards to distribute per hand
bool complete = false;

//double stime = 0; //used by master to store starting time

//print out the deck
void printDeck(Deck deck)
{
	Deck::iterator it;
	// print the deck out
	for(it = deck.begin(); it != deck.end(); ++it)
	{
		cout << (*it).getName() << endl;
		// cout << it->getName() << endl;
	}
}

//takes in a deck of 5 cards and determines the best possible poker hand
int findBestPokerHand(Deck deck)
{
	Deck::iterator it;
	Deck sortedDeck;

	if (deck.size() == 5)
	{
		//sort cards in order from lowest to highest
		
		for (int i = 0; i < 13 ; ++i) //iterate each number and add the card if it matches this value
		{
			for (int c = 0; c < 5; ++c) //iterate each card
			{
				if (deck[c].getRank() == i)
					sortedDeck.push_back(deck[c]);
			}
		}

		//store how many of each rank we have in it's own array to determine pairs, trips and quads
		int valArray[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};

		for (int i = 0; i < 5 ; ++i)
		{
			valArray[sortedDeck[i].getRank()]++;
		}

		//we have a flush
		if (sortedDeck[0].getSuit() == sortedDeck[1].getSuit() && 
			sortedDeck[0].getSuit() == sortedDeck[2].getSuit() && 
			sortedDeck[0].getSuit() == sortedDeck[3].getSuit() && 
			sortedDeck[0].getSuit() == sortedDeck[4].getSuit())
		{
			//royal flush
			if (sortedDeck[0].getRank() == 8 &&
				sortedDeck[1].getRank() == 9 &&
				sortedDeck[2].getRank() == 10 &&
				sortedDeck[3].getRank() == 11 &&
				sortedDeck[4].getRank() == 12)
			{
				//cout << "ROYALFLUSH!" << endl;
				return ROYALFLUSH;
			}

			//straight flush
			else if (sortedDeck[1].getRank() == sortedDeck[0].getRank() + 1 &&
				sortedDeck[2].getRank() ==  sortedDeck[1].getRank() + 1 &&
				sortedDeck[3].getRank() ==  sortedDeck[2].getRank() + 1 &&
				sortedDeck[4].getRank() ==  sortedDeck[3].getRank() + 1)
			{
				return STRAIGHTFLUSH;
			}

			//regular flush
			else
			{
				return FLUSH;
			}
		}//end flush check
		//pair, tripps, quads, full house, straight, two pair
		else
		{
			//four of a kind
			for (int i = 0; i < 13 ;i++)
			{
				if (valArray[i]==4) 
				{
					//four of a kind was found
					return FOUROFAKIND;
				}
			}

			//full house
			if (( sortedDeck[0].getRank() == sortedDeck[1].getRank() && 
				sortedDeck[3].getRank() == sortedDeck[4].getRank() && 
				( sortedDeck[2].getRank() == sortedDeck[1].getRank() || sortedDeck[2].getRank() == sortedDeck[3].getRank() )))
			{
				return FULLHOUSE;
			}

			//straight
			if (sortedDeck[1].getRank() == sortedDeck[0].getRank() + 1 &&
				sortedDeck[2].getRank() ==  sortedDeck[1].getRank() + 1 &&
				sortedDeck[3].getRank() ==  sortedDeck[2].getRank() + 1 &&
				sortedDeck[4].getRank() ==  sortedDeck[3].getRank() + 1)
			{
				return STRAIGHT;
			}
			
			//tripps
			for (int i = 0; i < 13 ;i++)
			{
				if (valArray[i]==3) 
				{
					//three of a kind was found
					return THREEOFAKIND;
				}
			}

			//two pair
			for (int i = 0; i < 13 ;i++)
			{
				if (valArray[i]==2) 
				{
					for (int x = i+1; x < 13 ;x++)
					{
						if (valArray[x] ==2)
						{
							//two pairs were found
							return TWOPAIR;
						}
					}
				}
			}

			//single pair
			for (int i = 0; i < 13 ;i++)
			{
				if (valArray[i]==2) 
				{
					//a pair was found
					return PAIR;
				}
			}
		}
	}
	//we made it to the end and didn't return anything. Must be high card.
	return HIGHCARD;
}

int getHandsFromSlaves()
{
    static MPI_Request request;
	static int msgBuff, recvFlag;
	MPI_Status status;

	if( request )
	{
		// Already listening for a message

		// Test to see if message has been received
		MPI_Test( &request, &recvFlag, &status );

		if( recvFlag )
		{
			//check if a new hand was found on another process
				if( status.MPI_TAG == ROYALFLUSH )
					return ROYALFLUSH;
				else if( status.MPI_TAG == STRAIGHTFLUSH )
					return STRAIGHTFLUSH;
				else if( status.MPI_TAG == FOUROFAKIND )
					return FOUROFAKIND;
				else if( status.MPI_TAG == FULLHOUSE )
					return FULLHOUSE;
				else if( status.MPI_TAG == FLUSH )
					return FLUSH;
				else if( status.MPI_TAG == STRAIGHT )
					return STRAIGHT;
				else if( status.MPI_TAG == THREEOFAKIND )
					return THREEOFAKIND;
				else if( status.MPI_TAG == TWOPAIR )
					return TWOPAIR;
				else if( status.MPI_TAG == PAIR )
					return PAIR;
				else if( status.MPI_TAG == HIGHCARD )
					return HIGHCARD;

					// Reset the request handle
					request = 0;
		}
	}

	if( !request )
		// Start listening again
		MPI_Irecv(&msgBuff, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);

	return -1;	
}

void createPokerHands (int numProcs, int rank)
{
	// Message passing variables
	int sendBuff[MSG_SIZE];
	double msgBuff2[10] = {0,0,0,0,0,0,0,0,0,0};
	MPI_Status status;
	MPI_Request request;
	int arraySize = numProcs - 1;

	//2. All processes set-up some data structures for keeping track of:
	//a. Which hand types have been generated at least once by ANY process
	bool recvHands[10] = {false, false, false, false, false, false, false, false, false, false};
	//b. The frequencies (counts) for each hand type generated by the CURRENT process –
	double handArray[10] = {0,0,0,0,0,0,0,0,0,0};
	//used by master to tally at end
	double handTotal[10] = {0,0,0,0,0,0,0,0,0,0};

	//3. If you’re using the rand() function, all processes should seed the random number generator
	//	 using a unique seed value.
	//rank 0 * time will always produce zero as the seed
	srand( rank+2 * (unsigned)time(NULL) );

	//4. The master process stores the starting time.
	double stime = MPI_Wtime();

	//6. All processes run a main loop that will terminate once the hands found array indicates that
	//	 every hand type has been generated at least once. The loop body:
	do
	{
		int hand = -1; //stores which hand we got back
		int newHand = -1;

		//a. Calls a function (checkMessagesFromSlaves?) that uses a non-blocking “receive”
		//	 function call (similar to the SinCheckNonBlocking example) to check to see if there 
		//	 is a message from any other process and if so returns the new hand type. 
		//	 Any hand type returned is used to update the hands found array with the newly found hand type. 
		//	 The main loop should terminate right away if all hand types have now been generated at least once.
		//check messages
		newHand = getHandsFromSlaves();
		if (newHand >= 0)
			recvHands[newHand] = true;

		complete = true;
		for(int i = 0; i < 10; i++)
		{
			if (recvHands[i] == false)
			{
				complete = false;
				break;
			}
		}

		if (complete)
		{
			if (request)
			{
				//MPI_Cancel( &request );
				// Reset the request handle
				request = 0;
			}
			break;
		}
		
		//5. All processes create a deck of cards
		Deck deck;
		for(int i = 0; i < Card::NUM_SUITS; ++i)
		{
			for(int j = 0; j < Card::NUM_RANKS; ++j)
			{
				Card card(i,j);  // calls constructor
				deck.push_back(card);  // puts the card on the vector
			}
		}
		assert(deck.size() == (Card::NUM_RANKS*Card::NUM_SUITS));

		//b. Shuffles the deck.
		random_shuffle(deck.begin(),deck.end());

		//create number of "players" per run and deal the cards
		PlayerHands playerHands(1); //only one hand needed to be generated
		assert((deck.size()) >= (CARDS_PER_HAND*playerHands.size()));

		// loop to deal the # of cards per hand
		for(int i = 0; i < CARDS_PER_HAND; ++i)
		{	// loop for number of players
			for(PlayerHands::iterator itHand=playerHands.begin(); itHand != playerHands.end(); ++itHand)
			{
				// takes the back card of the deck and puts in the players hand
				itHand->push_back(deck.back());
				// delete the last card from the deck
				deck.pop_back();
			}
		}

		for(size_t i = 0; i < playerHands.size(); ++i) //incase we use multiple hands (players) per run
		{
			//c. Determines the hand type represented by the first five cards in the deck.
			hand = findBestPokerHand(playerHands[i]);

			//d. Increments the appropriate counter in the local hand counts array.
			handArray[hand]++;			

			//e. If the hand type is a new type not previously generated:
			if  ((recvHands[hand] == false) && (handArray[hand] > 0))
			{
				//i. Updates the local hands found array
				recvHands[hand] = true;

				//ii. Sends a non-blocking message to ALL other processes (excluding the
				//	  current process) indicating the hand type found.
				for(int i = 0; i < numProcs; i++)
				{
					if (i != rank)
					{
						MPI_Isend(sendBuff, 1, MPI_INT, i, hand, MPI_COMM_WORLD, &request);
						MPI_Wait( &request, &status );
					}
				}

				//iii. And if there are no more hand types to be found, cancels any pending non-
				//	   blocking receive function call for the current process since the hand
				//	   generation is now complete.
				complete = true;
				for(int i = 0; i < 10; i++)
				{
					if (recvHands[i] == false)
					{
						complete = false;
						break;
					}
				}

				if (complete)
				{
					if (request)
					{
						//MPI_Cancel( &request );
						// Reset the request handle
						request = 0;
					}
					break;
				}
			}
		}
	}while(!complete);

	//7. After the main loop the slave processes each send their own hand counts array to the
	//	 master process using any suitable message passing technique.
	if (request)
		request = 0;

	if (numProcs > 0)
		MPI_Barrier( MPI_COMM_WORLD ) ; //only wait if there are other processes to wait for

	//8. After the main loop the master process receives and sums together the hand counts array
	//	 from each slave process.
	MPI_Reduce(handArray, handTotal, 10, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	//9. The master process checks the time again and calculates the elapsed time.
	//10. The master process calculates the total number of hands of any type generated.
	//11. The master process displays the combined frequencies (counts) and relative frequencies for
	//	  each type of poker hand. The master should also display the total number of hands
	//	  generated, the elapsed time and the total number of processes involved.
	if (rank == 0)
	{
		double totalHands = 0;
		for(int i = 0; i<10;++i)
			totalHands+=handTotal[i];

		const int WIDTHL = 39, WIDTHR = 10;
		const int PRECISION = 2;

		cout << setw(17) << setiosflags(ios::left) <<  " HAND TYPE "		<< setw(25) << setiosflags(ios::right) << "FREQUENCY"				<< setw(25) << setiosflags(ios::right) << "RELATIVE FREQUENCY (%)" << endl;
		cout << "================================================================================" << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "ROYAL FLUSH: "		<< setw(25) << setiosflags(ios::right) << handTotal[ROYALFLUSH]		<< setw(25) << setiosflags(ios::right) << handTotal[ROYALFLUSH]/totalHands*100 << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "STRAIGHT FLUSH: "	<< setw(25) << setiosflags(ios::right) << handTotal[STRAIGHTFLUSH]	<< setw(25) << setiosflags(ios::right) << handTotal[STRAIGHTFLUSH]/totalHands*100 << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "FOUR OF A KIND: "	<< setw(25) << setiosflags(ios::right) << handTotal[FOUROFAKIND]	<< setw(25) << setiosflags(ios::right) << handTotal[FOUROFAKIND]/totalHands*100 << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "FULL HOUSE: "		<< setw(25) << setiosflags(ios::right) << handTotal[FULLHOUSE]		<< setw(25) << setiosflags(ios::right) << handTotal[FULLHOUSE]/totalHands*100 << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "FLUSH: "			<< setw(25) << setiosflags(ios::right) << handTotal[FLUSH]			<< setw(25) << setiosflags(ios::right) << handTotal[FLUSH]/totalHands*100 << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "STRAIGHT: "			<< setw(25) << setiosflags(ios::right) << handTotal[STRAIGHT]		<< setw(25) << setiosflags(ios::right) << handTotal[STRAIGHT]/totalHands*100 << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "THREE OF A KIND: "	<< setw(25) << setiosflags(ios::right) << handTotal[THREEOFAKIND]	<< setw(25) << setiosflags(ios::right) << handTotal[THREEOFAKIND]/totalHands*100 << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "TWO PAIR: "			<< setw(25) << setiosflags(ios::right) << handTotal[TWOPAIR]		<< setw(25) << setiosflags(ios::right) << handTotal[TWOPAIR]/totalHands*100 << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "PAIR: "				<< setw(25) << setiosflags(ios::right) << handTotal[PAIR]			<< setw(25) << setiosflags(ios::right) << handTotal[PAIR]/totalHands*100 << endl;
		cout << setw(17) << setiosflags(ios::left) <<  "HIGH CARD: "		<< setw(25) << setiosflags(ios::right) << handTotal[HIGHCARD]		<< setw(25) << setiosflags(ios::right) << handTotal[HIGHCARD]/totalHands*100 << endl << endl;

		cout << "Total hands generated: " << totalHands << endl;
		cout << "Completed in " << (MPI_Wtime() - stime) << " seconds, using " << numProcs << " processes." << endl;
	}
}

//this is the old method for reference
void createDeckAndGetHand( int numProcs, int rank )
{
	// Message passing variables
	int msgBuff[MSG_SIZE];
	MPI_Status status;
	MPI_Request request;
	static int recvFlag;

	//2. All processes set-up some data structures for keeping track of:
	//a. Which hand types have been generated at least once by ANY process
	bool recvHands[10] = {false, false, false, false, false, false, false, false, false, false};
	//b. The frequencies (counts) for each hand type generated by the CURRENT process –
	double handArray[10] = {0,0,0,0,0,0,0,0,0,0};

	srand( (unsigned)time(NULL)); //seed for randum number using time

	do
	{
		Deck deck;
		int hand = -1;

		//check messages
		MPI_Irecv(&msgBuff, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);

		if( request )
		{
			// Already listening for a message

			// Test to see if message has been received
			MPI_Test( &request, &recvFlag, &status );

			if( recvFlag )
			{
				// Message received
				//cout << "Slave received a message!" << endl;
				//check if a new hand was found on another process
				if( status.MPI_TAG == ROYALFLUSH )
					recvHands[ROYALFLUSH] = true;
				else if( status.MPI_TAG == STRAIGHTFLUSH )
					recvHands[STRAIGHTFLUSH] = true;
				else if( status.MPI_TAG == FOUROFAKIND )
					recvHands[FOUROFAKIND] = true;
				else if( status.MPI_TAG == FULLHOUSE )
					recvHands[FULLHOUSE] = true;
				else if( status.MPI_TAG == FLUSH )
					recvHands[FLUSH] = true;
				else if( status.MPI_TAG == STRAIGHT )
					recvHands[STRAIGHT] = true;
				else if( status.MPI_TAG == THREEOFAKIND )
					recvHands[THREEOFAKIND] = true;
				else if( status.MPI_TAG == TWOPAIR )
					recvHands[TWOPAIR] = true;
				else if( status.MPI_TAG == PAIR )
					recvHands[PAIR] = true;
				else if( status.MPI_TAG == HIGHCARD )
					recvHands[HIGHCARD] = true;

					// Reset the request handle
					request = 0;
			}
		}

		

		//cout << "Hands Found: " << endl;
		//for(int i = 0; i < 10; ++i)
		//{
		//	//recvHands[i] = true;
		//	if (recvHands[i] == true)
		//		cout << i << ", ";
		//}

		//cout << otherProcess << endl;

		complete = true;
		for(int i = 0; i < 10; ++i)
		{
			if (recvHands[i] == false)
				complete = false;
		}

		complete = true;
		if (complete == true)
		{
			//cout << "Work is done!" << endl;
			MPI_Send(&msgBuff[0], 1, MPI_INT, 0, TAG_DONE, MPI_COMM_WORLD);
		}

		// create the deck of cards
		for(int i = 0; i < Card::NUM_SUITS; ++i)
		{
			for(int j = 0; j < Card::NUM_RANKS; ++j)
			{
				Card card(i,j);  // calls constructor
				deck.push_back(card);  // puts the card on the vector
			}
		}
		assert(deck.size() == (Card::NUM_RANKS*Card::NUM_SUITS));
		random_shuffle(deck.begin(),deck.end());
		PlayerHands playerHands(1); //only one hand needed to be generated
		assert((deck.size()) >= (CARDS_PER_HAND*playerHands.size()));

		// loop to deal the # of cards per hand
		for(int i = 0; i < CARDS_PER_HAND; ++i)
		{	// loop for number of players
			for(PlayerHands::iterator itHand=playerHands.begin(); itHand != playerHands.end(); ++itHand)
			{
				// takes the back card of the deck and puts in the players hand
				itHand->push_back(deck.back());
				// delete the last card from the deck
				deck.pop_back();
			}
		}
		// determine best hand
		for(size_t i = 0; i < playerHands.size(); ++i) //incase we use multiple hands (players) per run
		{
			hand = findBestPokerHand(playerHands[i]);
			handArray[hand]++;

			//new hand found?
			if ( (handArray[hand] == 1) && (recvHands[hand] == false) )
			{
				//cout << "New hand found!" << endl;
				//notify other slaves
				for(int i = 1; i < numProcs; ++i)
					MPI_Isend(&msgBuff[0], 1, MPI_INT, i, hand, MPI_COMM_WORLD, &request);
			}
			
			//might need to notify self process that work is done
			//recvHands[hand] = true;
		}
	} while( complete != true );

	//this loop listens for a message from the master telling us to send over the data we collected
	do {
		if( request )
		{
			// Already listening for a message
			
			// Test to see if message has been received
			MPI_Test( &request, &recvFlag, &status );
			
			if( recvFlag )
			{
				// Message received
				//cout << "Slave got a msg! -> " << status.MPI_TAG << endl;
				if( status.MPI_TAG == TAG_HAND )
				{
					//request = 0;
					cout <<"Sending hand data to master!" << endl;
					MPI_Send(&handArray, 10, MPI_DOUBLE, 0, TAG_HAND, MPI_COMM_WORLD);
				}

				// Reset the request handle
				request = 0;
			}
		}

		if( !request )
		{
			//cout << "Slave listening..." << endl;
			MPI_Irecv(&msgBuff, 1, MPI_INT, 0, TAG_HAND, MPI_COMM_WORLD, &request);
		}
	}while (status.MPI_TAG != TAG_QUIT);
}


