#include "Card.h"

  Card::Card(){
    rank = -1;
    suit = NoSuit;
  }

  Card::Card(int rank, Suit suit) {
    this->rank = rank;
    this->suit = suit;
  }

  int Card::GetRank() const {
    if(rank == -1){
      throw "Rank does not exist.";
    } else {
      return rank;
    }
  }

  Suit Card::GetSuit() const {
    if(suit == NoSuit){
      throw "Suit does not exist.";
    } else {
      return suit;
    }
  }

  bool Card::operator==(const Card& b) const {
    return this->GetRank() == b.GetRank() && this->GetSuit() == b.GetSuit();
  }

  string Card::ShowCard(){
    //shows card, ex. Qspade
    string display;
    if(rank <= 9){
      display += '0' + rank;
    } else {
      if(rank == 10){
        display += "T";
      } else if(rank == 11){
        display += "J";
      } else if (rank == 12){
        display += "Q";
      } else if (rank == 13){
        display += "K";
      } else if (rank == 14){
        display += "A";
      }
    }
    if(suit == Hearts){
      display += "\u2665";
    } else if (suit == Spades){
      display += "\u2660";
    } else if (suit == Diamonds){
      display += "\u25C6";
    } else if (suit == Clubs){
      display += "\u2663";
    }
    return display;
  }
