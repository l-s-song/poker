#ifndef CARD_H
#include <string>

using namespace std;

enum Suit {
  Clubs,
  Diamonds,
  Hearts,
  Spades,
  NoSuit
};

class Card {
  int rank;
  Suit suit;
public:
  Card();

  Card(int rank, Suit suit);

  int GetRank() const;

  Suit GetSuit() const;

  bool operator==(const Card& b) const;

  string ShowCard();
};
#endif
#define CARD_H
