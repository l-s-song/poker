#include "HandSimulation.h"

#include <iostream>
#include <cmath>

vector<string> playerNames;

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

string ShowTable(HandSimulation& hs, bool isShowdown){
  //creates boker board graphic which displays board, pot,
  //stacks, bets, button, and who has folded
  int longestName = 0;
  for(int i = 0; i < hs.numPlayers(); i++){
    if(playerNames[i].size() > longestName){
      longestName = playerNames[i].size();
    }
  }
  int maxDigits = 0;
  for(int i = 0; i < hs.numPlayers(); i++){
    int numDigits = NumDigits(hs.getStacks()[i]);
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
  vector<Card>& board = hs.getBoard();
  if(board.size() > 0){
    table += Deck(board).ShowDeck();
    table += repeatString(" ", totalWidth - board.size()*3);
    table += "|\n";
  }
  for(int i = 0; i < hs.numPots(); i++) {
    int potSize = hs.getPot(i);
    table += "Pot: " + to_string(potSize) +
      repeatString(" ", totalWidth - 5 - NumDigits(potSize)) + "|\n";
  }
  table += divider;
  for(int i = 0; i < hs.numPlayers(); i++){
    if(i == hs.getCurrentTurn()){
      table += "> ";
    } else {
      table += "  ";
    }
    table += playerNames[i] + repeatString(" ",
      2 + longestName - playerNames[i].size());
    table += to_string(hs.getStacks()[i]) + repeatString(" ",
      maxDigits + 1 - NumDigits(hs.getStacks()[i]));
    if(i == hs.getButtonLocation()){
      table += "[B] ";
    } else {
      table += "    ";
    }
    if(hs.hasFolded(i)) {
      table += "Folded" + repeatString(" ", 4);
    } else if(isShowdown){
      if (hs.mustShowdown()) {
        table += "[ " + hs.getHand(i).first.ShowCard() + " " +
                      hs.getHand(i).second.ShowCard() + " ] ";
      } else {
        table += "Mucked    ";
      }
    } else {
      table += to_string(hs.getBet(i)) + repeatString(" ",
      10 - NumDigits(hs.getBet(i)));
    }
    table += "|\n";
  }
  table += divider;
  return table;
}

void BettingRound(HandSimulation& hs){
  //handles a single round of betting. Ex. preflop is a single betting round.
  hs.initBettingRound();

  while(!hs.isBettingRoundOver()){
    WipeScreen();
    cout << ShowTable(hs, false);
    char ans = '\0';
    if(hs.canCheck(hs.getCurrentTurn())) {
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
      hs.Check();
    } else if (ans == 'f'){
      cout << "You folded." << endl;
      hs.Fold();
    } else if (ans == 'k'){
      cout << "You called." << endl;
      hs.Call();
    } else if (ans == 'r' || ans == 'b'){
      int bet = 0;
      cout << "How much are you betting?" << endl;
      cin >> bet;
      while(!hs.IsValidBet(hs.getCurrentTurn(), bet)){
        cin.clear();
        cin.ignore();
        cout << "That is not a valid bet size." << endl;
        cin >> bet;
      }
      hs.Bet(bet);
    }
    cin.ignore();
    //stops the while loop if the previous bettor is reached
    //this is the first person to go if no one bets
  }

  hs.endBettingRound();
}

void Showdown(HandSimulation& hs){
  //computes post-betting action, such as saying who won
  WipeScreen();
  cout << ShowTable(hs, true) << endl;
  vector<vector<winnerData>> winners = hs.getWinners();
  for (int i = 0; i < winners.size(); i++) {
    bool isChopping = false;
    if (winners[i].size() > 1 && winners[i][1].winnings > 1) {
      isChopping = true;
    }
    for (const winnerData& w : winners[i]) {
      if (w.winnings > 0) {
        if (isChopping) {
          cout << playerNames[w.player] << " chops the pot of " << hs.getPot(i) << " and wins " << w.winnings;
        } else {
          cout << playerNames[w.player] << " wins the pot of " << hs.getPot(i);
        }
        if(hs.mustShowdown()) {
          cout << " with a " <<
            HRtoString(hs.GetFinalHand(w.player).handRank) << ": " <<
            Deck(hs.GetFinalHand(w.player).hand).ShowDeck() << endl;
        } else {
          cout << "." << endl;
        }
      } else {
        cout << playerNames[w.player] <<
          " has a " << HRtoString(hs.GetFinalHand(w.player).handRank) << ", " <<
          Deck(hs.GetFinalHand(w.player).hand).ShowDeck() << endl;
      }
    }
    cout << endl;
  }
  hs.awardWinners();
}

void PlayHand(HandSimulation& hs) {
  WipeScreen();
  for (int i = 0; i < hs.numPlayers(); i++) {
    if (hs.getStacks()[i] == 0) {
      continue;
    }
    cout << "Showing " << playerNames[i]
      << "\'s hand. Press enter to continue." << endl;
    cin.ignore();
    WipeScreen();
    cout << playerNames[i] << ", " <<
      "your hand is " <<
      hs.getHand(i).first.ShowCard() << " " <<
      hs.getHand(i).second.ShowCard() << endl;
    cout << endl;
    cout << "Press enter to continue." << endl;
    cin.ignore();
    WipeScreen();
  }
  BettingRound(hs);
  while(!hs.isHandOver()){
    BettingRound(hs);
  }

  Showdown(hs);
  hs.endHand();
}

void PlayGame(){
  int buttonLocation = (rand() % playerNames.size());
  vector<int> stacks(playerNames.size(), 200);
  while(true){
    HandSimulation hs(2, buttonLocation, stacks);
    PlayHand(hs);
    buttonLocation = hs.getButtonLocation();
    stacks = hs.getStacks();
    cout << endl << "Press enter to continue." << endl;
    cin.ignore();
    WipeScreen();
  }
}

int main() {
  srand(time(NULL));
  int numPlayers;
  cout << "How many players?" << endl;
  cin >> numPlayers;
  cout << "Enter player first names." << endl;
  playerNames = vector<string>(numPlayers);
  for(int i = 0; i < numPlayers; i++){
    cin >> playerNames[i];
  }
  cin.ignore();
  PlayGame();
  return 0;
}
