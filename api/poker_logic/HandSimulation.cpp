#include "HandSimulation.h"

#include <algorithm>
#include <cmath>
#include <iostream>

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


HandSimulation::HandSimulation(int smallBlind, int bigBlind, int stackSize){
  this->smallBlind = smallBlind;
  this->bigBlind = bigBlind;
  this->stackSize = stackSize;
}

bool HandSimulation::IsValidBet(int player, int betSize){
  //checks if bet/raise size is good. Doesn't do calls
  if(betSize - currentBet >= max(betDifference, bigBlind)
    || betSize == stacks[player] + bets[player]){
      return true;
  } else {
    return false;
  }
}

bool HandSimulation::comparePlayerFinalHands(int a, int b) {
  return compareFinalHands(GetFinalHand(a), GetFinalHand(b));
}

FinalHand HandSimulation::GetFinalHand(int player) {
  vector<Card> finalHand;
  finalHand.push_back(hands[player].first);
  finalHand.push_back(hands[player].second);
  if (board.size() < 3) {
    return {NoHand, finalHand};
  } else {
    for(Card& c : board) {
      finalHand.push_back(c);
    }
  }
  return Best5Of7(finalHand);
}

void HandSimulation::initBettingRound() {
  if (bettingRound == 1) {
    deck.GetCard();

    board.push_back(deck.GetCard());
    board.push_back(deck.GetCard());
    board.push_back(deck.GetCard());
  } else if (bettingRound > 1) {
    deck.GetCard();

    board.push_back(deck.GetCard());
  }

  if (bettingRound == 0) {
    if(stacks.size() == 2){
      placeMoney(buttonLocation, smallBlind);
      placeMoney(buttonLocation + 1, bigBlind);
      currentTurn = buttonLocation;
    } else {
      placeMoney(buttonLocation + 1, smallBlind);
      placeMoney(buttonLocation + 2, bigBlind);
      currentTurn = (buttonLocation + 3) % stacks.size();
    }
    currentBet = bigBlind;
  } else {
    currentBet = 0;
    currentTurn = (buttonLocation + 1) % stacks.size();
  }

  while(hasFolded(currentTurn)) {
    currentTurn = (currentTurn + 1) % stacks.size();
  }

  betDifference = bigBlind;
  prevBettor = currentTurn;

  roundDone = false;
}

void HandSimulation::endBettingRound() {
  CollectPot();
  bettingRound++;
}

void HandSimulation::placeMoney(int player, int betSize) {
  player %= stacks.size();
  int betAmount = min(stacks[player], betSize - bets[player]);
  stacks[player] -= betAmount;
  bets[player] += betAmount;
  betDifference = max(bets[player] - currentBet, betDifference);
  currentBet = max(currentBet, bets[player]);
}

void HandSimulation::Bet(int betSize){
  if (betSize > currentBet) {
    prevBettor = currentTurn;
  }
  placeMoney(currentTurn, betSize);
  //handles mechanics of a bet
  handleEndAction();
}

void HandSimulation::Call() {
  placeMoney(currentTurn, currentBet);
  handleEndAction();
}

void HandSimulation::Fold() {
  activePlayers.erase(currentTurn);
  for (int i = 0; i < pots.size(); i++) {
    potEligibility[i].erase(currentTurn);
  }
  handleEndAction();
}

bool HandSimulation::isHandOver() {
  return bettingRound > 3;
}

bool HandSimulation::isBettingRoundOver() {
  int numNotAllIn = 0;
  for(int i : activePlayers) {
    if (stacks[i] > 0 && bets[i] < currentBet && !hasFolded(i)) {
      return false;
    }
    if (stacks[i] > 0 && !hasFolded(i)) {
      numNotAllIn++;
    }
  }
  return roundDone || numNotAllIn < 2 || activePlayers.size() < 2;
}

void HandSimulation::handleEndAction() {
  do {
    currentTurn = (currentTurn + 1) % stacks.size();
  } while (hasFolded(currentTurn));

  if(currentTurn == prevBettor){
    roundDone = true;
  }
}

bool HandSimulation::canCheck(int player) {
  return bets[player] >= currentBet;
}

bool HandSimulation::isAllIn(int player) {
  return stacks[player] == 0;
}

bool HandSimulation::hasFolded(int player) {
  return activePlayers.count(currentTurn) == 0;
}

void HandSimulation::CollectPot(){
  while(true) {
    // Get the smallest bet made, so that it can be cherry picked out
    int smallestActiveBet = -1;
    for(int i = 0; i < stacks.size(); i++) {
      if (potEligibility.back().count(i)) {
        if (smallestActiveBet == -1) {
          smallestActiveBet = bets[i];
        } else {
          smallestActiveBet = min(smallestActiveBet, bets[i]);
        }
      }
    }

    // Keep track of bets that must be in separate pots
    set<int> stillNeedsCollection;
    // pots : 5 35 10
    // potEligibility : {Bob, Alice, Charlie, David}, {Alice, Charlie, Bob}, {Charlie, Alice}

    // $5, $10, $10
    //
    // Cherry pick all bets out
    for (int i : potEligibility.back()) {
      pots.back() += smallestActiveBet;
      bets[i] -= smallestActiveBet;
      if (bets[i] > 0) {
        stillNeedsCollection.insert(i);
      }
    }

    // If a sidepot must be made, then make it
    // Otherwise, all bets are collected, so exit
    if (stillNeedsCollection.size() > 0) {
      pots.push_back(0);
      potEligibility.push_back(stillNeedsCollection);
    } else {
      break;
    }
  }
}

vector<vector<winnerData>> HandSimulation::getWinners() {
  vector<vector<winnerData>> ret;
  for( int i = 0; i < pots.size(); i++) {
    vector<winnerData> playerWinnings;

    for(int j : potEligibility[i]){
      playerWinnings.push_back({j, 0});
      //if multiple remaining players, get their hand ranks
    }

    // Sort from least to greatest
    sort(playerWinnings.begin(), playerWinnings.end(),
      [&](const winnerData& a,
         const winnerData& b)
      {
        return comparePlayerFinalHands(b.player, a.player);
      }
    );

    // Sorts playerHands from highest to lowest, equivalent to sorting by rank
    int numChopping = 1;
    for (int j = 1; j <= playerWinnings.size(); j++) {
      if (j == playerWinnings.size() || comparePlayerFinalHands(playerWinnings[j].player, playerWinnings[j-1].player)) {
        for(int k = j - numChopping; k < j; k++) {
          playerWinnings[k].winnings = pots[i] / numChopping;
        }
        break;
      } else {
        numChopping++;
      }
    }

    ret.push_back(playerWinnings);
  }

  return ret;
}

void HandSimulation::awardWinners() {
  vector<vector<winnerData>> winners = getWinners();
  for (int i = 0; i < pots.size(); i++) {
    for (const winnerData& w : winners[i]) {
      stacks[w.player] += w.winnings;
    }
  }
}

void HandSimulation::initHand( vector<int>& playerStacks ) {
  activePlayers.clear();
  stacks.clear();

  // Deal cards and initialize player data
  deck.Shuffle();
  for(int i = 0; i < playerStacks.size(); i++) {
    stacks[i] = playerStacks[i];
    if (stacks[i] > 0) {
      activePlayers.insert(i);
      Card a = deck.GetCard();
      Card b = deck.GetCard();
      hands[i] = {a, b};
    }
  }

  // Initialize game data
  pots.clear();
  pots.push_back(0);

  potEligibility.clear();
  potEligibility.push_back(activePlayers);

  bets.clear();
  board.clear();
  bettingRound = 0;
}

void HandSimulation::endHand() {
  int oldButtonLocation = buttonLocation;

  buttonLocation++;
  buttonLocation %= stacks.size();
  while(stacks[buttonLocation] == 0 && buttonLocation != oldButtonLocation){
    buttonLocation++;
    buttonLocation %= stacks.size();
  }
}

void HandSimulation::BettingRound(){
  //handles a single round of betting. Ex. preflop is a single betting round.
  initBettingRound();

  while(!isBettingRoundOver()){
    if(hasFolded(currentTurn)){
      currentTurn = (currentTurn + 1) % stacks.size();
        cout << prevBettor << " " << currentTurn << endl;
    } else {
      WipeScreen();
      cout << ShowTable(false);
        cout << prevBettor << " " << currentTurn << endl;
      if(isAllIn(currentTurn)){
        Call();
      } else {
        char ans = '\0';
        if(canCheck(currentTurn)) {
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
          Call();
        } else if (ans == 'f'){
          cout << "You folded." << endl;
          Fold();
        } else if (ans == 'k'){
          cout << "You called." << endl;
          Call();
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
          Bet(bet);
        }
        cin.ignore();
      }
    }
    //stops the while loop if the previous bettor is reached
    //this is the first person to go if no one bets
  }

  endBettingRound();
}

void HandSimulation::Showdown(){
  //computes post-betting action, such as saying who won
  WipeScreen();
  cout << ShowTable(true) << endl;
  vector<vector<winnerData>> winners = getWinners();
  for (int i = 0; i < pots.size(); i++) {
    bool isChopping = false;
    if (winners[i].size() > 1 && winners[i][1].winnings > 1) {
      isChopping = true;
    }
    for (const winnerData& w : winners[i]) {
      if (w.winnings > 0) {
        if (isChopping) {
          cout << players[w.player].name << " chops the pot of " << pots[i] << " and wins " << w.winnings;
        } else {
          cout << players[w.player].name << " wins the pot of " << pots[i];
        }
        if(activePlayers.size() > 1) {
          cout << " with a " <<
            HRtoString(GetFinalHand(w.player).handRank) << ": " <<
            Deck(GetFinalHand(w.player).hand).ShowDeck() << endl;
        } else {
          cout << "." << endl;
        }
      } else {
        cout << players[w.player].name <<
          " has a " << HRtoString(GetFinalHand(w.player).handRank) << ", " <<
          Deck(GetFinalHand(w.player).hand).ShowDeck() << endl;
      }
    }
    cout << endl;
  }
  awardWinners();
}

void HandSimulation::PlayHand() {
  vector<int> newStacks;
  for(int i = 0; i < stacks.size(); i++) {
    newStacks.push_back(stacks[i]);
  }
  initHand(newStacks);

  WipeScreen();
  for (int i = 0; i < stacks.size(); i++) {
    if (stacks[i] == 0) {
      continue;
    }
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
  BettingRound();
  while(!isHandOver()){
    BettingRound();
  }

  Showdown();
  endHand();
}

void HandSimulation::PlayGame(){
  buttonLocation = (rand()) % stacks.size();
  while(true){
    PlayHand();
    cout << endl << "Press enter to continue." << endl;
    cin.ignore();
    WipeScreen();
  }
  //need to update players to remove knocked out
  //players
}

string HandSimulation::ShowTable(bool isShowdown){
  //creates boker board graphic which displays board, pot,
  //stacks, bets, button, and who has folded
  int longestName = 0;
  for(int i = 0; i < stacks.size(); i++){
    if(players[i].name.size() > longestName){
      longestName = players[i].name.size();
    }
  }
  int maxDigits = 0;
  for(int i = 0; i < stacks.size(); i++){
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
  for(int i = 0; i < pots.size(); i++) {
    table += "Pot: " + to_string(pots[i]) +
      repeatString(" ", totalWidth - 5 - NumDigits(pots[i])) + "|\n";
  }
  table += divider;
  for(int i = 0; i < stacks.size(); i++){
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
