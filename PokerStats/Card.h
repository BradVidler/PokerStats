#include <string>
using namespace std;
// card class 
class Card
{
private:
	unsigned int iRank_;   // the number on the card
	unsigned int iSuit_;   // the suit of the card
public:
	static const unsigned int NUM_SUITS = 4;   
	static const unsigned int NUM_RANKS = 13;
	// two argument constructor
	Card(unsigned int s, unsigned int r): iSuit_(s), iRank_(r)
	{
		// found that we need this code to verify that uses don't put in bad rank or suit
		if(iSuit_ >= NUM_SUITS)
			throw("Bad suit");
		if(iRank_ >= NUM_RANKS)
			throw("Bad rank");
	}
	// method to get the card that was created
	string getName()
	{
		string sName;
		string suitName [] = { "Diamonds", "Clubs", "Hearts","Spades"};
		string rankName [] = { "Two","Three","Four","Five","Six","Seven",
			"Eight","Nine","Ten","Jack","Queen","King","Ace"};
		sName = rankName[iRank_];
		sName += " of ";
		sName += suitName[iSuit_];
		return sName;
	}

	//method to get suit of card
	int getSuit()
	{
		return iSuit_;
	}

	void setSuit(int uSuit)
	{
		iSuit_ = uSuit;
	}

	//method to get rank of card
	int getRank()
	{
		return iRank_;
	}

	void setRank(int uRank)
	{
		iRank_ = uRank;
	}
};  // end all classes with a semicolon