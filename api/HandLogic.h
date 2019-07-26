#ifndef HANDLOGIC_H
#include "Card.h"
#include <vector>

enum HandRank{
  HighCard,
  Pair,
  TwoPair,
  Trips,
  Straight,
  Flush,
  FullHouse,
  Quads,
  StraightFlush,
  NoHand = -1
};

string HRtoString(int a);

class FinalHand {
public:
  HandRank handRank;
  vector<Card> hand;
};

vector<Card> FiveCards(vector<Card> primary, vector<Card> remainder);

FinalHand Best5Of7(vector<Card>& finalHand);

bool compareFinalHands(const FinalHand& a, const FinalHand& b);

#endif
#define HANDLOGIC_H
