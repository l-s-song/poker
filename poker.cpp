#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <ctime>

using namespace std;
typedef long long ll;
template<typename A, typename B>
using hmap = unordered_map<A, B>;
typedef tuple<int, int> ii;
typedef tuple<ll, ll> lii;
#define PI 3.14159265358979323846
#define inf 0x3f3f3f3f
#define infl 0x3f3f3f3f3f3f3f3fL

struct node{
  node* prev;
  node* next;
  ll data;
};

/*

Player
=====
ID
Name

Game
=====
vector<Player> players]
int buttonLocation;
int currentTurn
int currentBet
int betDifference
int smallBlind
int bigBlind
int pot
vector<int> deck
map<int, int> bets
map<int, int> stacks

*/

void WipeScreen(){
  for(int i = 0; i < 200; i++){
    cout << endl;
  }
}

enum Suit {
  Clubs,
  Diamonds,
  Hearts,
  Spades,
  NoSuit
};

enum HandRank{
  HighCard,
  Pair,
  TwoPair,
  Trips,
  Straight,
  Flush,
  FullHouse,
  Quads,
  StraightFlush
};

class Card {
  int rank;
  Suit suit;
public:
  Card(){
    rank = -1;
    suit = NoSuit;
  }

  Card(int rank, Suit suit) {
    this->rank = rank;
    this->suit = suit;
  }

  int GetRank() const {
    if(rank == -1){
      throw "Rank does not exist.";
    } else {
      return rank;
    }
  }

  Suit GetSuit() const {
    if(suit == NoSuit){
      throw "Suit does not exist.";
    } else {
      return suit;
    }
  }

  bool Equals(Card a) const {
    if(rank == a.GetRank() && suit == a.GetSuit()){
      return true;
    } else {
      return false;
    }
  }

  string ShowCard(){
    string display;
    if(rank <= 10){
      display += '0' + rank;
    } else {
      if(rank == 11){
        display += "Q";
      } else if (rank == 12){
        display += "K";
      } else if (rank == 13){
        display += "A";
      }
    }
    if(suit == Hearts){
      display += "H";
    } else if (suit == Spades){
      display += "S";
    } else if (suit == Diamonds){
      display += "D";
    } else if (suit == Clubs){
      display += "C";
    }
    return display;
  }
};

class Deck {
public:
  vector<Card> cards;

  Deck() {
    Shuffle();
  }

  Deck(vector<Card> cards){
    this->cards = cards;
  }

  void Shuffle(){
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

  Card GetCard(){
    Card ret = cards.back();
    cards.pop_back();
    return ret;
  }

  string ShowDeck(){
    string s;
    for(int i = 0; i < cards.size(); i++){
      s += cards[i].ShowCard();
      s += "";
    }
    return s;
  }
};

class Player {
public:
  int id;
  string name;
};

class Game {
public:
  vector<Player> players;
  int smallBlind;
  int bigBlind;
  int stackSize;
  //update at beginning of game
  int buttonLocation;
  Deck deck;
  //update at beginning of round
  int currentTurn;
  int currentBet;
  int betDifference;
  set<int> activePlayers;
  //update throughout rounds
  int pot;
  //update throughout and at beginning of rounds
  map<int, int> bets;
  map<int, int> stacks;
  map<int, pair<Card, Card>> hands;
  vector<Card> board;

  Game(int smallBlind, int bigBlind, int stackSize){
    this->smallBlind = smallBlind;
    this->bigBlind = bigBlind;
    this->stackSize = stackSize;
  }

  void PostBlind(){
    if(players.size() == 2){
      Bet(buttonLocation, smallBlind);
      Bet(buttonLocation + 1, bigBlind);
    } else {
      Bet(buttonLocation + 1, smallBlind);
      Bet(buttonLocation + 2, bigBlind);
    }
  }

  bool IsValidBet(int player, int betSize){
    if(betSize - currentBet >= betDifference
      || betSize == stacks[player]){
        return true;
    } else {
      return false;
    }
  }

  void Bet(int player, int betSize){
    player = player % players.size();
    if(stacks[player] >= betSize){
      stacks[player] -= betSize;
      bets[player] += betSize;
      betDifference = betSize - currentBet;
      currentBet = betSize;
    } else {
      bets[player] += stacks[player];
      betDifference = stacks[player] - currentBet;
      currentBet = stacks[player];
      stacks[player] = 0;
    }
  }

  void CollectPot(){
    for(int i = 0; i < players.size(); i++){
      pot += bets[i];
      bets[i] = 0;
    }
  }

  void BettingRound(){
    int prevBettor = currentTurn;
    bool roundDone = false;

    while(!roundDone){
      if(activePlayers.count(currentTurn) != 0){
        if(board.size() > 0){
          cout << "The board is ";
          for(int i = 0; i < board.size(); i++){
            cout << board[i].ShowCard() << " ";
          }
          cout << endl;
        }
        cout << "It is " << players[currentTurn].name <<
          "\' turn." << endl;
        cout << "Stack size:  " << stacks[currentTurn] <<
        " " << "Bets made: " << bets[currentTurn] <<
          " " << endl;
        cout << "You must match a raise of " << currentBet
          << "." << endl;

        char ans;
        if(bets[currentTurn] >= currentBet){
          while(ans != 'c' && ans != 'b'){
            cout << "Check or Bet? c/b" << endl;
            cin >> ans;
          }
        } else {
          while(ans != 'b' && ans != 'f'){
            cout << "Fold or Bet? f/b" << endl;
            cin >> ans;
          }
        }
        if(ans == 'c'){
          cout << "You checked." << endl;
        } else if (ans == 'f'){
          cout << "You folded." << endl;
          activePlayers.erase(currentTurn);
        } else if (ans == 'B'){
          int bet = 0;
          cout << "How much are you betting?" << endl;
          cin >> bet;
          while(!IsValidBet(currentTurn, bet)){
            cout << "Try again" << endl;
            cin >>  bet;
          }
          Bet(currentTurn, bet);
          prevBettor = currentTurn;
        }
      }
      bool hasmoney = true;
      while(hasmoney){
        currentTurn++;
        currentTurn %= players.size();
        if(currentTurn == prevBettor){
          roundDone = true;
        }
        if(stacks[currentTurn] != 0){
          hasmoney = false;
        }
      }
    }
  }

  vector<Card> FiveCards(vector<Card> primary,
    vector<Card> remainder){
      //returns a five card hand. primary cards are
      //first, remainder fills up until there are 5 cards
      //remainder is sorted from lowest to highest
      Deck remainderDeck = Deck(remainder);
      calc: while(primary.size() < 5){
        Card temp = remainderDeck.GetCard();
        for(int i = 0; i < primary.size(); i++){
          if(primary[i].Equals(temp) ){
            goto calc;
          }
        }
        primary.push_back(temp);
      }
      return primary;
    }

  pair<HandRank, vector<Card>> GetHandRank(int player){
    vector<vector<Suit>> orderedRank(15);
    vector<vector<int>> orderedSuit(4);
    vector<Card> seven = board;
    seven.push_back(hands[player].first);
    seven.push_back(hands[player].second);
    for(int i = 0; i < 7; i++){
      orderedRank[seven[i].GetRank()].push_back(seven[i].GetSuit());
      orderedSuit[seven[i].GetSuit()].push_back(seven[i].GetRank());
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
        }
      }
    }
    sort(seven.begin(), seven.end(),
      [](const Card& a, const Card& b)
      {
          return a.GetRank() < b.GetRank();
      }
    );
    vector<int> pairs;
    vector<int> trips;
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
        return {Quads, FiveCards(quadhand, seven)};
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
      if (inarow >= 5){
        vector<Card> straightHand;
        for(int j = i; j < i+5; j++){
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
      return {Trips, FiveCards(tripsHand, seven)};
    }
    if(pairs.size() >= 2){
      vector<Card> twoPair;
      for(int i = 0; i < 2; i++){
        for(int j = 0; j < 2; j++){
          twoPair.push_back(Card(pairs[i], orderedRank[pairs[i]][j]));
        }
      }
      return {TwoPair, FiveCards(twoPair, seven)};
    }
    if(pairs.size() >= 1){
      vector<Card> pairHand;
      for(int i = 0; i < 2; i++){
        pairHand.push_back(Card(pairs[0], orderedRank[pairs[0]][i]));
      }
      return {Pair, FiveCards(pairHand, seven)};
    }
    return {HighCard, FiveCards(vector<Card>(), seven)};
  }

  void Showdown(){
    if(activePlayers.size() == 1){
      cout << "Folded to " <<
      players[*activePlayers.begin()].name << endl;
    }
    vector<pair<int, pair<HandRank, vector<Card>>>> playerHands;
    for(int i : activePlayers){
      playerHands.push_back({i, GetHandRank(i)});
    }
    sort(playerHands.begin(), playerHands.end(),
      [](const pair<int, pair<HandRank, vector<Card>>>& a,
         const pair<int, pair<HandRank, vector<Card>>>& b)
      {
        if(a.second.first == b.second.first){
          for(int i = 0; i < 7; i++){
            if(a.second.second[i].GetRank() != b.second.second[i].GetRank() ){
              return a.second.second[i].GetRank() > b.second.second[i].GetRank();
            }
          }
        } else {
          return a.second.first > b.second.first;
        }
      }
    );
    //sorts playerHands from highest to lowest
    stacks[playerHands[0].first] += pot;
    if(activePlayers.size() > 1){
      cout << players[playerHands[0].first].name <<
        "wins the pot of " << pot << "with a " <<
        playerHands[0].second.first << ", " <<
        Deck(playerHands[0].second.second).ShowDeck() << endl;
    }
    cout << players[playerHands[0].first].name << "now has" <<
      stacks[playerHands[0].first] << "chips." << endl;
    for(int i = 1; i < playerHands.size(); i++){
      cout << players[playerHands[i].first].name <<
        "has a " << playerHands[i].second.first << ", " <<
        Deck(playerHands[i].second.second).ShowDeck() << endl;
    }
  }

  void PlayHand(){
    for(int i = 0; i < players.size(); i++){
      cout << players[i].name << " has " << stacks[i] <<
      " chips." << endl;
    }
    cout << endl;
    activePlayers = set<int>();
    for(int i = 0; i < players.size(); i++){
      if(stacks[i] > 0){
        activePlayers.insert(i);
      }
    }
    deck.Shuffle();
    pot = 0;
    bets = map<int, int>(); // ?
    board = vector<Card>();
    if(players.size() == 2 ){
      currentTurn = buttonLocation;
    } else {
      currentTurn = buttonLocation + 1;
    }
    cin.ignore();
    for(int i = 0; i < players.size(); i++){
      Card a = deck.GetCard();
      Card b = deck.GetCard();
      hands[i] = {a, b};
      cout << "Showing " << players[i].name
        << "\'s hand. Press enter to continue." << endl;
      cin.ignore();
      cout << players[i].name << ", " <<
        "your hand is " <<
        hands[i].first.ShowCard() << " " <<
        hands[i].second.ShowCard() << "." << endl;
      cout << "You have " << stacks[i] << " chips." << endl;
      cout << "Press enter to continue." << endl;
      cin.ignore();
      WipeScreen();
    }
    currentBet = bigBlind;
    betDifference = bigBlind - smallBlind;
    board = vector<Card>();

    PostBlind();
    BettingRound();
    //betting preflop
    board.push_back(deck.GetCard());
    board.push_back(deck.GetCard());
    for(int i = 0; i < 3; i++){
      board.push_back(deck.GetCard());
      currentTurn = buttonLocation + 1;
      if(activePlayers.size() >= 2){
        BettingRound();
        CollectPot();
      }
    }
    Showdown();
    buttonLocation++;
    while(stacks[buttonLocation] == 0){
      buttonLocation++;
    }
  }

  void PlayGame(){
    buttonLocation = (rand()) % players.size();
    while(true){
      PlayHand();
      char c;
      cout << "Press enter to continue." << endl;
      cin >> c;
      WipeScreen();
    }
    //need to update players to remove knocked out
    //players
  }
};

int main() {
  srand(time(NULL));
  int numplayers;
  cout << "How many players?" << endl;
  cin >> numplayers;
  cout << "Enter player first names." << endl;
  Game game(1, 2, 100);
  game.players = vector<Player>(numplayers);
  for(int i = 0; i < numplayers; i++){
    cin >> game.players[i].name;
    game.stacks[i] = game.bigBlind*100;
  }
  game.PlayGame();
  return 0;
}
