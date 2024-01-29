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
  vector<vector<bool>> sb_rfi;
  vector<vector<pair<float, float>>> bb_vs_2bet;
  vector<vector<pair<float, float>>> sb_vs_3bet;
  vector<vector<pair<float, float>>> bb_vs_4bet;
  int preflop;
  pair<int,int> preflop_chart_pos;
  string cards = "23456789TJQKA";
  string suits = "shcd";



  Bot() {
    import_preflop();



  }
  void import_preflop_floats(string filepath, vector<vector<pair<float, float>>>* vec_ptr) {
      ifstream inputFile(filepath);
      string line;
      while (getline(inputFile, line)) {
          stringstream ss(line);
          string field;
          vec_ptr->push_back(vector<pair<float, float>>());

          while (getline(ss, field, ',')) {
              stringstream sss(field);
              string probability;
              int i = 0;
              pair<float, float> prob_pair;
              while (getline(sss, probability, ';')) {
                  if (i == 0) {
                      prob_pair.first = stof(probability);
                  }
                  else {
                      prob_pair.second = stof(probability);
                  }
                  i++;
              }
              vec_ptr->back().push_back(prob_pair);
          }
      }
      inputFile.close();
  }
  void import_preflop() {
      ifstream inputFile("preflop_data/preflop - btn rfi.csv");
      string line;
      while (getline(inputFile, line)) {
          stringstream ss(line);
          string field;
          sb_rfi.push_back(vector<bool>());

          while (getline(ss, field, ',')) {
              sb_rfi.back().push_back(field == "1");
          }
      }
      inputFile.close();

      import_preflop_floats("preflop_data/preflop - bb vs. 2bet.csv", &bb_vs_2bet);
      import_preflop_floats("preflop_data/preflop - btn vs. 3bet.csv", &sb_vs_3bet);
      import_preflop_floats("preflop_data/preflop - bb vs. 4bet.csv", &bb_vs_4bet);

      return;
  }
  pair<int,int> hand_to_chart_pos(int x, int y) {
    bool suited = (x/13 == y/13);
    int num1 = x%13;
    int num2 = y%13;
    if (suited) {
        pair<int,int> p(min(num1, num2), max(num1, num2));
        return p;
    }
    else {
        pair<int,int> p(max(num1, num2), max(num1, num2));
        return p;
    }
  }
  int card_to_num(string card) {
        int num = 0;
        for (int i = 0; i < 13; i++) {
            if (card[0] == cards[i]) {
                num += i;
            }
        }
        for (int i = 0; i < 4; i++) {
            if (suits[i] == card[1]) {
                num += 13 * i;
            }
        }
        return num;
    }

  /*
    Called when a new round starts. Called NUM_ROUNDS times.

    @param gameState The GameState object.
    @param roundState The RoundState object.
    @param active Your player's index.
  */
  void handleNewRound(GameInfoPtr gameState, RoundStatePtr roundState, int active) {
    int myBankroll = gameState->bankroll;  // the total number of chips
    // you've gained or lost from the beginning of the game to the start of this round
    float gameClock = gameState->gameClock;  // the total number of
    // seconds your bot has left to play this game
    int roundNum = gameState->roundNum;  // the round number from 1 to State.NUM_ROUNDS
    auto myCards = roundState->hands[active];  // your cards bool bigBlind =
    active == 1;  // true if you are the big blind

    // Own stuff:
    preflop = 0;
    preflop_chart_pos = hand_to_chart_pos(card_to_num(myCards[0]), card_to_num(myCards[1]));
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
      // PREFLOP
      if (active == 1) { // bigblind

      }
      else { // smallblind
        if (preflop == 0) {
          if (sb_rfi[preflop_chart_pos.first][preflop_chart_pos.second]) {
            return {Action::Type::RAISE, 5};
          }
          else {
            return {Action::Type::FOLD};
          }
        }
        else if (preflop == 1) { // bb 3bet

        }
      }
      preflop++;
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
