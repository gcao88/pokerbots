#include <random>

#include "../libs/skeleton/include/skeleton/actions.h"
#include "../libs/skeleton/include/skeleton/constants.h"
#include "../libs/skeleton/include/skeleton/game.h"
#include "../libs/skeleton/include/skeleton/runner.h"
#include "../libs/skeleton/include/skeleton/states.h"
#include "../libs/skeleton/include/skeleton/util.h"

using namespace pokerbots::skeleton;
using namespace std;

struct Bot {
  /*
    Called when a new round starts. Called NUM_ROUNDS times.

    @param gameState The GameState object.
    @param roundState The RoundState object.
    @param active Your player's index.
  */
  void handleNewRound(GameInfoPtr gameState, RoundStatePtr roundState,
                      int active) {
    // int myBankroll = gameState->bankroll;  // the total number of chips
    // you've gained or lost from the beginning of the game to the start of this
    // round float gameClock = gameState->gameClock;  // the total number of
    // seconds your bot has left to play this game int roundNum =
    // gameState->roundNum;  // the round number from 1 to State.NUM_ROUNDS auto
    // myCards = roundState->hands[active];  // your cards bool bigBlind =
    active == 1;  // true if you are the big blind
  }

  /*
    Called when a round ends. Called NUM_ROUNDS times.

    @param gameState The GameState object.
    @param terminalState The TerminalState object.
    @param active Your player's index.
  */
  void handleRoundOver(GameInfoPtr gameState, TerminalStatePtr terminalState,
                       int active) {
    // int myDelta = terminalState->deltas[active];  // your bankroll change
    // from this round auto previousState = std::static_pointer_cast<const
    // RoundState>(terminalState->previousState);  // RoundState before payoffs
    // int street = previousState->street;  // 0, 3, 4, or 5 representing when
    // this round ended auto myCards = previousState->hands[active];  // your
    // cards auto oppCards = previousState->hands[1-active];  // opponent's
    // cards or "" if not revealed
  }

  /*
    Where the magic happens - your code should implement this function.
    Called any time the engine needs an action from your bot.

    @param gameState The GameState object.
    @param roundState The RoundState object.
    @param active Your player's index.
    @return Your action.
  */
  Action getAction(GameInfoPtr gameState, RoundStatePtr roundState,
                   int active) {
    // May be useful, but you can choose to not use.
    auto legalActions =
        roundState->legalActions();   // the actions you are allowed to take
    int street = roundState->street;  // 0, 3, 4, or 5 representing pre-flop,
                                      // flop, turn, or river respectively
    auto myCards = roundState->hands[active];  // your cards
    auto boardCards = roundState->deck;        // the board cards
    int myPip =
        roundState->pips[active];  // the number of chips you have contributed
                                   // to the pot this round of betting
    int oppPip =
        roundState
            ->pips[1 - active];  // the number of chips your opponent has
                                 // contributed to the pot this round of betting
    int myStack =
        roundState->stacks[active];  // the number of chips you have remaining
    int oppStack = roundState->stacks[1 - active];  // the number of chips your
                                                    // opponent has remaining
    int myBid = roundState->bids[active].value_or(
        0);  // How much you bid previously (available only after auction)
    int oppBid = roundState->bids[1 - active].value_or(
        0);  // How much opponent bid previously (available only after auction)
    int continueCost =
        oppPip - myPip;  // the number of chips needed to stay in the pot
    int myContribution =
        STARTING_STACK -
        myStack;  // the number of chips you have contributed to the pot
    int oppContribution =
        STARTING_STACK - oppStack;  // the number of chips your opponent has
                                    // contributed to the pot
    int minCost = 0;
    int maxCost = 0;
    int pot = 800 - myStack - oppStack;

    if (street == 0) {
      // preflop
    }
    else if (street == 3) {
      if (legalActions.find(Action::Type::BID) != legalActions.end()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> bid_distribution(3*pot, 4*pot);
        return {Action::Type::BID, bid_distribution(gen)};
      }
      if (boardCards[0][1] == boardCards[1][1] && boardCards[0][1] == boardCards[2][1]) {
        // MONOTONE
        char suit = boardCards[0][1];
        if (myCards.size() == 2) {
          if (myCards[0][1] != suit && myCards[1][1] != suit) {
            if (legalActions.find(Action::Type::CHECK) != legalActions.end()) {
              return {Action::Type::CHECK};
            }
            return {Action::Type::FOLD};
          }
          if (myCards[0][1] == suit && myCards[1][1] == suit) {
            // hit flush
            if (legalActions.find(Action::Type::CHECK) != legalActions.end()) {
              if (legalActions.find(Action::Type::RAISE) != legalActions.end()) {
                auto raiseBounds = roundState->raiseBounds();
                minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
                maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
                return {Action::Type::RAISE, min(max(int(minCost), int(pot/2)), int(maxCost))};
              }
              return {Action::Type::CHECK};
            }
            if (legalActions.find(Action::Type::CALL) != legalActions.end()) {
              return {Action::Type::CALL};
            }
          }
          else {
            if (legalActions.find(Action::Type::CHECK) != legalActions.end()) {
              return {Action::Type::CHECK};
            }
            char num = myCards[0][0];
            if (myCards[1][1] == suit) {
              num = myCards[1][0];
            }
            else if (myCards[2][1] == suit) {
              num = myCards[2][0];
            }
            if (num == 'A' || num == 'K' || num == 'Q' || num == 'J') {
              if (legalActions.find(Action::Type::CALL) != legalActions.end()) {
                return {Action::Type::CALL};
              }
            }
            else {
              return {Action::Type::FOLD};
            }
          }
        }
        else {
          // 3 cards
          int hits = (suit==myCards[0][1])+(suit==myCards[1][1])+(suit==myCards[2][1]);
          if (hits >= 2) {
            if (active == 1) {
              if (legalActions.find(Action::Type::RAISE) != legalActions.end()) {
                auto raiseBounds = roundState->raiseBounds();
                minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
                maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
                return {Action::Type::RAISE, min(max(int(minCost), int(pot*4/10)), int(maxCost))};
              }
            }
            else {
              if (legalActions.find(Action::Type::CALL) != legalActions.end()) {
                return {Action::Type::CALL};
              }
              if (legalActions.find(Action::Type::RAISE) != legalActions.end()) {
                auto raiseBounds = roundState->raiseBounds();
                minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
                maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
                return {Action::Type::RAISE, min(max(int(minCost), int(pot*4/10)), int(maxCost))};
              }
              if (legalActions.find(Action::Type::CHECK) != legalActions.end()) {
                return {Action::Type::CHECK};
              }
            }
          }
          else if (hits == 1) {
            char num = myCards[0][0];
            if (myCards[1][1] == suit) {
              num = myCards[1][0];
            }
            else if (myCards[2][1] == suit) {
              num = myCards[2][0];
            }
            if (num == 'A' || num == 'K' || num == 'Q' || num == 'J') {
              if (legalActions.find(Action::Type::CALL) != legalActions.end()) {
                return {Action::Type::CALL};
              }
              if (legalActions.find(Action::Type::RAISE) != legalActions.end()) {
                auto raiseBounds = roundState->raiseBounds();
                minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
                maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
                return {Action::Type::RAISE, min(max(int(minCost), int(pot*4/10)), int(maxCost))};
              }
              if (legalActions.find(Action::Type::CHECK) != legalActions.end()) {
                return {Action::Type::CHECK};
              }
            }
            else {
              if (legalActions.find(Action::Type::CALL) != legalActions.end()) {
                return {Action::Type::CALL};
              }
              if (legalActions.find(Action::Type::RAISE) != legalActions.end()) {
                auto raiseBounds = roundState->raiseBounds();
                minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
                maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
                return {Action::Type::RAISE, min(max(int(minCost), int(pot*4/10)), int(maxCost))};
              }
              if (legalActions.find(Action::Type::CHECK) != legalActions.end()) {
                return {Action::Type::CHECK};
              }
            }
          }
          else {
            if (continueCost > 0) {
              return {Action::Type::FOLD};
            }
            if (legalActions.find(Action::Type::RAISE) != legalActions.end()) {
              auto raiseBounds = roundState->raiseBounds();
              minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
              maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
              return {Action::Type::RAISE, min(max(int(minCost), int(pot/3)), int(maxCost))};
            }
            if (legalActions.find(Action::Type::CHECK) != legalActions.end()) {
              return {Action::Type::CHECK};
            }
          }
        }
      }
      // else if (boardCards[0][1] == boardCards[1][1] || boardCards[0][1] == boardCards[2][1] || boardCards[1][1] == boardCards[2][1]) {
      //   // TWOTONE
      // }
      else {
        // FLOP
        // RAINBOW
        // (OR TWOTONE)

      }
    }
    else if (street == 4) {

    }
    else { // street == 5

    }

  }
};

/*
  Main program for running a C++ pokerbot.
*/
// int main(int argc, char *argv[]) {
//   auto [host, port] = parseArgs(argc, argv);
//   runBot<Bot>(host, port);
//   return 0;
// }
