#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <ctime>
#include "Card.h"
#include "Deck.h"

using namespace std;

string repeatString(string s, int a){
  //returns string that is string s repeated a times
  string ret;
  for(int i = 0; i < a; i++){
    ret += s;
  }
  return ret;
}

void WipeScreen(){
  //wipe terminal
  cout << repeatString("\n", 200) << endl;
}

int NumDigits(int a){
  //calculate number of digits of int
  return floor(log10(max(a, 1))) + 1;
}

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

string HRtoString(int a){
  vector<string> store = {"High Card", "Pair", "Two Pair",
    "Trips", "Straight", "Flush", "Full House", "Quads",
    "Straight Flush"};
  return store[a];
}


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
  //multiples of bigblind to compute  starting stack
  int buttonLocation;
  Deck deck;
  int currentTurn;
  int currentBet;
  int betDifference;
  set<int> activePlayers;
  //players still active in betting round
  int pot;
  map<int, int> bets;
  //bets made and not yet collected into pot
  map<int, int> stacks;
  map<int, pair<Card, Card>> hands;
  //cards dealt to people
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
    //checks if bet/raise size is good. Doesn't do calls
    if(betSize - currentBet >= max(betDifference, bigBlind)
      || betSize == stacks[player]){
        return true;
    } else {
      return false;
    }
  }

  void Bet(int player, int betSize){
    //handles mechanics of a bet
    player = player % players.size();
    if(stacks[player] >= betSize - bets[player]){
      stacks[player] -= betSize - bets[player];
      bets[player] = betSize;
      if (betSize != currentBet) {
        betDifference = betSize - currentBet;
      }
      currentBet = betSize;
    } else {
      bets[player] += stacks[player];
      if(bets[player] > (currentBet + betDifference) ){
        betDifference = stacks[player] - currentBet;
      }
      stacks[player] = 0;
    }
  }

  void CollectPot(){
    //transfers bets into pot
    for(int i = 0; i < players.size(); i++){
      pot += bets[i];
      bets[i] = 0;
    }
  }

  void BettingRound(){
    //handles a single round of betting. Ex. preflop is a single betting round.
    int prevBettor = currentTurn;
    // cout << "prevBettor: " << prevBettor << endl;
    bool roundDone = false;

    while(!roundDone && activePlayers.size() >= 2){
      //doesn't run if only 1 player in hand
      if(activePlayers.count(currentTurn) == 1){
        WipeScreen();
        cout << ShowTable(false);
        if(stacks[currentTurn] != 0){
          char ans = '0';
          if(bets[currentTurn] >= currentBet){
            while(ans != 'c' && ans != 'b'){
              cout << "Check or Bet? c/b" << endl;
              cin >> ans;
            }
          } else {
            while(ans != 'f' && ans != 'k' && ans != 'r'){
              cout << "Fold or Call or Raise? f/c/r" << endl;
              cin >> ans;
              if (ans == 'c') {
                ans = 'k';
              }
            }
          }
          if(ans == 'c'){
            cout << "You checked." << endl;
          } else if (ans == 'f'){
            cout << "You folded." << endl;
            activePlayers.erase(currentTurn);
          } else if (ans == 'k'){
            Bet(currentTurn, currentBet);
          } else if (ans == 'r' || ans == 'b'){
            int bet = 0;
            cout << "How much are you betting?" << endl;
            cin >> bet;
            while(!IsValidBet(currentTurn, bet)){
              cin.clear();
              cin.ignore();
              cout << "That is not a valid bet size." << endl;
              cin >> bet;
            }
            Bet(currentTurn, bet);
            prevBettor = currentTurn;
          }
          cin.ignore();

        }
      }
      currentTurn++;
      currentTurn %= players.size();
      if(currentTurn == prevBettor){
        roundDone = true;
      }
      //stops the while loop if the previous bettor is reached
      //this is the first person to go if no one bets
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
    //From 7 cards, calculates the best 5 card hand and
    //returns relevant cards in order
    vector<vector<Suit>> orderedRank(15);
    //vector index indicaets rank, suits of that rank are stored in the vector cells
    vector<vector<int>> orderedSuit(4);
    //vector index indicates suit, ranks of that suit are stored in the vector cells
    vector<Card> seven = board;
    //has the 7 seven cards
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
    sort(seven.begin(), seven.end(),
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
    //computes post-betting action, such as saying who won
    WipeScreen();
    cout << ShowTable(true);
    vector<pair<int, pair<HandRank, vector<Card>>>> playerHands;
    int winner = 0;
    if(activePlayers.size() == 1){
      winner = *activePlayers.begin();
      //if only 1 remaining active player, they are the winner people
      //foldled to
    } else {
      for(int i : activePlayers){
        playerHands.push_back({i, GetHandRank(i)});
        //if multiple remaining players, get their hand ranks
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
      //sorts playerHands from highest to lowest, equivalent to sorting by rank
      winner = playerHands[0].first;
    }
    stacks[winner] += pot;

    cout << players[winner].name <<
    " wins the pot of " << pot;
    if(activePlayers.size() > 1){
      cout << " with a " <<
        HRtoString(playerHands[0].second.first) << ": " <<
        Deck(playerHands[0].second.second).ShowDeck() << endl;
    } else {
      cout << "." << endl;
    }
    cout << endl;
    for(int i = 1; i < playerHands.size(); i++){
      cout << players[playerHands[i].first].name <<
        " has a " << HRtoString(playerHands[i].second.first) << ", " <<
        Deck(playerHands[i].second.second).ShowDeck() << endl;
    }
    if (playerHands.size() >= 2) {
      cout << endl;
    }
    cout << players[winner].name << " now has " <<
      stacks[winner] << " chips." << endl;
  }

  void PlayHand(){
    //plays a single hand, prefop through river
    activePlayers = set<int>();
    for(int i = 0; i < players.size(); i++){
      if(stacks[i] > 0){
        activePlayers.insert(i);
      }
    }
    deck.Shuffle();
    pot = 0;
    bets = map<int, int>();
    board = vector<Card>();
    //resetting data
    if(players.size() == 2){
      currentTurn = buttonLocation;
    } else {
      currentTurn = (buttonLocation + 3) % players.size();
    }
    WipeScreen();
    for(int i = 0; i < players.size(); i++){
      Card a = deck.GetCard();
      Card b = deck.GetCard();
      hands[i] = {a, b};
      cout << "Showing " << players[i].name
        << "\'s hand. Press enter to continue." << endl;
      cin.ignore();
      WipeScreen();
      cout << players[i].name << ", " <<
        "your hand is " <<
        hands[i].first.ShowCard() << " " <<
        hands[i].second.ShowCard() << endl;
      cout << endl;
      cout << "Press enter to continue." << endl;
      cin.ignore();
      WipeScreen();
    }
    //showing people their hands
    currentBet = bigBlind;
    betDifference = bigBlind;
    board = vector<Card>();

    PostBlind();
    BettingRound();
    //betting preflop
    if(activePlayers.size() >= 2){
      board.push_back(deck.GetCard());
      board.push_back(deck.GetCard());
    }
    //only add cards to their board if there are multiple players active
    //to prevent rabbit hunt
    for(int i = 0; i < 3; i++){
      currentBet = 0;
      betDifference = bigBlind;
      currentTurn = (buttonLocation + 1) % players.size();
      CollectPot();
      if(activePlayers.size() >= 2){
        board.push_back(deck.GetCard());
        BettingRound();
        CollectPot();
      }
    }
    Showdown();
    buttonLocation++;
    buttonLocation %= players.size();
    while(stacks[buttonLocation] == 0){
      buttonLocation++;
      buttonLocation %= players.size();
    }
    //button moved until it is at player with nonzero stack
    //cout << "TEST" << endl;
  }

  void PlayGame(){
    buttonLocation = (rand()) % players.size();
    while(true){
      PlayHand();
      cout << endl << "Press enter to continue." << endl;
      cin.ignore();
      WipeScreen();
    }
    //need to update players to remove knocked out
    //players
  }

  string ShowTable(bool isShowdown){
    //creates boker board graphic which displays board, pot,
    //stacks, bets, button, and who has folded
    int longestName = 0;
    for(int i = 0; i < players.size(); i++){
      if(players[i].name.size() > longestName){
        longestName = players[i].name.size();
      }
    }
    int maxDigits = 0;
    for(int i = 0; i < players.size(); i++){
      int numDigits = NumDigits(stacks[i]);
      if(numDigits > maxDigits){
        maxDigits = numDigits;
      }
    }
    int totalWidth = 19 + longestName + maxDigits;

    string divider = "";
    for(int i = 0; i < totalWidth; i++){
      divider += "=";
    }
    divider += "|\n";

    string table = divider;
    if(board.size() > 0){
      table += Deck(board).ShowDeck();
      table += repeatString(" ", totalWidth - board.size()*3);
      table += "|\n";
    }
    table += "Pot: " + to_string(pot) +
      repeatString(" ", totalWidth - 5 - NumDigits(pot)) + "|\n";
    table += divider;
    for(int i = 0; i < players.size(); i++){
      if(i == currentTurn){
        table += "> ";
      } else {
        table += "  ";
      }
      table += players[i].name + repeatString(" ",
        2 + longestName - players[i].name.size());
      table += to_string(stacks[i]) + repeatString(" ",
        maxDigits + 1 - NumDigits(stacks[i]));
      if(i == buttonLocation){
        table += "[B] ";
      } else {
        table += "    ";
      }
      if(activePlayers.count(i) == 1 && isShowdown){
        if (activePlayers.size() > 1) {
          table += "[ " + hands[i].first.ShowCard() + " " +
            hands[i].second.ShowCard() + " ] ";
        } else {
          table += "Mucked    ";
        }
      } else if (activePlayers.count(i) == 1){
        table += to_string(bets[i]) + repeatString(" ",
        10 - NumDigits(bets[i]));
      } else {
        table += "Folded" + repeatString(" ", 4);
      }
      table += "|\n";
    }
    table += divider;
    return table;
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
  cin.ignore();
  game.PlayGame();
  return 0;
}
