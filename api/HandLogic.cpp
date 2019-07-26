#include "HandLogic.h"
#include <algorithm>

string HRtoString(int a) {
  vector<string> store = {"High Card", "Pair", "Two Pair",
    "Trips", "Straight", "Flush", "Full House", "Quads",
    "Straight Flush"};
  return store[a];
}

vector<Card> FiveCards(vector<Card> primary, vector<Card> remainder) {
  //returns a five card hand. primary cards are
  //first, remainder fills up until there are 5 cards
  //remainder is sorted from lowest to highest
  calc: while(primary.size() < 5){
    Card temp = remainder.back();
    remainder.pop_back();
    for(int i = 0; i < primary.size(); i++){
      if(primary[i] == temp){
        goto calc;
      }
    }
    primary.push_back(temp);
  }
  return primary;
}

FinalHand Best5Of7(vector<Card>& finalHand) {
  //From 7 cards, calculates the best 5 card hand and
  //returns relevant cards in order
  vector<vector<Suit>> orderedRank(15);
  //vector index indicaets rank, suits of that rank are stored in the vector cells
  vector<vector<int>> orderedSuit(4);
  //vector index indicates suit, ranks of that suit are stored in the vector cells
  //has the 7 finalHand cards
  for(Card& c : finalHand){
    orderedRank[c.GetRank()].push_back(c.GetSuit());
    orderedSuit[c.GetSuit()].push_back(c.GetRank());
  }
  bool isFlush = false;
  bool isStraightFlush = false;
  vector<Card> flushhand;
  for(int i = 0; i < 4; i++){
    if(orderedSuit[i].size() >= 5){
      isFlush = true;
      sort(orderedSuit[i].begin(), orderedSuit[i].end());
      reverse(orderedSuit[i].begin(), orderedSuit[i].end());
      //largest to smallest
      for(int j = 0; j < 5; j++){
        flushhand.push_back(Card(orderedSuit[i][j], Suit(i)));
      }
      int inarow = 1;
      for(int j = 1; j < orderedSuit[i].size(); j++){
        if(orderedSuit[i][j] == orderedSuit[i][j-1] + 1){
          inarow++;
        } else {
          inarow = 1;
        }
        if(inarow >= 5){
          vector<Card> straightflush;
          for(int k = j-4; k < j+1; k++){
            straightflush.push_back(Card(orderedSuit[i][k], Suit(i)));
          }
          return {StraightFlush, straightflush};
        }
        if (inarow == 4 &&
          orderedSuit[i][orderedSuit[i].size() - 1] == 2 &&
          orderedSuit[i][0] == 14){
          vector<Card> straightflush;
          for(int k = orderedSuit[i].size() - 1;
            k >= orderedSuit[i].size() - 4; k--){
            straightflush.push_back(Card(orderedSuit[i][k], Suit(i)));
          }
          straightflush.push_back(Card(orderedSuit[i][0], Suit(i)));
          return {StraightFlush, straightflush};
        }
        //Ace-5 straight calculation
      }
    }
  }
  sort(finalHand.begin(), finalHand.end(),
    [](const Card& a, const Card& b)
    {
        return a.GetRank() < b.GetRank();
    }
  );
  //sort cards by rank from smallest to largest
  vector<int> pairs;
  //stores rank of any pairs
  vector<int> trips;
  //stores rank of any trips
  //ordered greatest to least
  for(int i = 14; i >= 2; i--){
    if(orderedRank[i].size() == 2){
      pairs.push_back(i);
    } else if (orderedRank[i].size() == 3){
      trips.push_back(i);
    } else if (orderedRank[i].size() == 4){
      vector<Card> quadhand;
      for(int j = 0; j < 4; j++){
        quadhand.push_back(Card(i, orderedRank[i][j]));
      }
      return {Quads, FiveCards(quadhand, finalHand)};
    }
  }
  if(pairs.size() >= 1 && trips.size() >= 1){
    vector<Card> fullhouse;
    for(int i = 0; i < 3; i++){
      fullhouse.push_back(Card(trips[0],
        orderedRank[trips[0]][i]));
    }
    for(int i = 0; i < 2; i++){
      fullhouse.push_back(Card(pairs[0],
        orderedRank[pairs[0]][i]));
    }
    return {FullHouse, fullhouse};
  }
  if(isFlush){
    return {Flush, flushhand};
  }
  bool isStraight = false;
  vector<int> presentRanks;
  int inarow = 1;
  for(int i = 13; i >= 2; i--){
    if(orderedRank[i].size() > 0 &&
      orderedRank[i+1].size() > 0){
        inarow++;
    } else {
      inarow = 1;
    }
    if (inarow >= 5 ){
      vector<Card> straightHand;
      for(int j = i; j < i+5; j++){
        straightHand.push_back(Card(j, orderedRank[j][0]));
      }
      return {Straight, straightHand};
    } else if (inarow >= 4 && orderedRank[14].size() >= 1 &&
      i == 2) {
        vector<Card> straightHand;
        straightHand.push_back(Card(14, orderedRank[14][0]));
        for(int j = i; j < i+4; j++){
          straightHand.push_back(Card(j, orderedRank[j][0]));
        }
        return {Straight, straightHand};
    }
  }
  if(trips.size() >= 1){
    vector<Card> tripsHand;
    for(int i = 0; i < 3; i++){
      tripsHand.push_back(Card(trips[0], orderedRank[trips[0]][i]));
    }
    return {Trips, FiveCards(tripsHand, finalHand)};
  }
  if(pairs.size() >= 2){
    vector<Card> twoPair;
    for(int i = 0; i < 2; i++){
      for(int j = 0; j < 2; j++){
        twoPair.push_back(Card(pairs[i], orderedRank[pairs[i]][j]));
      }
    }
    return {TwoPair, FiveCards(twoPair, finalHand)};
  }
  if(pairs.size() >= 1){
    vector<Card> pairHand;
    for(int i = 0; i < 2; i++){
      pairHand.push_back(Card(pairs[0], orderedRank[pairs[0]][i]));
    }
    return {Pair, FiveCards(pairHand, finalHand)};
  }
  return {HighCard, FiveCards(vector<Card>(), finalHand)};
}

bool compareFinalHands(const FinalHand& a, const FinalHand& b) {
  if(a.handRank == b.handRank){
    for(int i = 0; i < 7; i++){
      int aRank = a.hand[i].GetRank();
      int bRank = b.hand[i].GetRank();
      if (aRank != bRank) {
        return aRank < bRank;
      }
    }
  } else {
    return a.handRank < b.handRank;
  }
}
