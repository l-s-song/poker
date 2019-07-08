#include "Deck.h"

Deck::Deck() {
  Shuffle();
}

Deck::Deck(vector<Card>& cards){
  this->cards = cards;
}

void Deck::Shuffle(){
  //Generates deck of 52 cards = vector of cards
  set<int> usedCards;
  cards = vector<Card>();
  while(usedCards.size() != 52){
    int a = rand() % 52;
    //a = 1-52
    if(usedCards.count(a) == 0){
      cards.push_back(Card(a/4 + 2, Suit(a%4)));
    }
    usedCards.insert(a);
  }
}

Card Deck::GetCard(){
  //extracts and returns card from deck
  Card ret = cards.back();
  cards.pop_back();
  return ret;
}

string Deck::ShowDeck(){
  string s;
  for(int i = 0; i < cards.size(); i++){
    s += cards[i].ShowCard();
    s += " ";
  }
  return s;
}
