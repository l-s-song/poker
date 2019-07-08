#ifndef DECK_H
#include "Card.h"
#include <set>
#include <vector>

class Deck {
public:
  vector<Card> cards;

  Deck();

  Deck(vector<Card>& cards);

  void Shuffle();

  Card GetCard();

  string ShowDeck();
};
#endif
#define DECK_H
