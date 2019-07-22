#include "HandSimulation.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

HandSimulation::HandSimulation(int bigBlind, int buttonLocation, const vector<int>& stacks){
  this->smallBlind = bigBlind / 2;
  this->bigBlind = bigBlind;
  this->buttonLocation = buttonLocation;
  this->stacks = stacks;

  hands.resize(numPlayers());

  // Deal cards and initialize player data
  deck.Shuffle();
  for(int i = 0; i < numPlayers(); i++) {
    if (this->stacks[i] > 0) {
      activePlayers.insert(i);
      Card a = deck.GetCard();
      Card b = deck.GetCard();
      hands[i] = {a, b};
    }
  }

  // Initialize game data
  pots.push_back(0);
  potEligibility.push_back(activePlayers);
  bets.resize(numPlayers(), 0);

  bettingRound = 0;
}

int HandSimulation::numPlayers() {
  return stacks.size();
}

int HandSimulation::getCurrentTurn() {
  return currentTurn;
}

const vector<Card>& HandSimulation::getBoard() {
  return board;
}

const vector<int>& HandSimulation::getStacks() {
  return stacks;
}

const vector<int>& HandSimulation::getBets() {
  return bets;
}

const vector<int>& HandSimulation::getPots() {
  return pots;
}

const set<int>& HandSimulation::getActivePlayers(){
  return activePlayers;
}

int HandSimulation::getCurrentBet(){
  return currentBet;
}

int HandSimulation::getMinRaise(){
  return betDifference + currentBet;
}

int HandSimulation::getButtonLocation() {
  return buttonLocation;
}

const pair<Card, Card>& HandSimulation::getHand(int player) {
  return hands[player];
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

int HandSimulation::getBet(int player) {
  return bets[player];
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

bool HandSimulation::mustShowdown() {
  return activePlayers.size() > 1;
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
  advanceAction();
}

void HandSimulation::Call() {
  placeMoney(currentTurn, currentBet);
  advanceAction();
}

void HandSimulation::Check() {
  if (currentBet > 0) {
    throw "Cannot check, as currentBet is non-zero";
  }
  advanceAction();
}

void HandSimulation::Fold() {
  activePlayers.erase(currentTurn);
  for (int i = 0; i < pots.size(); i++) {
    potEligibility[i].erase(currentTurn);
  }
  advanceAction();
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

void HandSimulation::advanceAction() {
  do {
    currentTurn = (currentTurn + 1) % stacks.size();
  } while (hasFolded(currentTurn) || (isAllIn(currentTurn) && currentTurn != prevBettor));

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

int HandSimulation::numPots() {
  return pots.size();
}

int HandSimulation::getPot(int pot) {
  return pots[pot];
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

void HandSimulation::endHand() {
  int oldButtonLocation = buttonLocation;

  buttonLocation++;
  buttonLocation %= stacks.size();
  while(stacks[buttonLocation] == 0 && buttonLocation != oldButtonLocation){
    buttonLocation++;
    buttonLocation %= stacks.size();
  }
}
