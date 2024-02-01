#include <random>

#include "../libs/skeleton/include/skeleton/actions.h"
#include "../libs/skeleton/include/skeleton/constants.h"
#include "../libs/skeleton/include/skeleton/game.h"
#include "../libs/skeleton/include/skeleton/runner.h"
#include "../libs/skeleton/include/skeleton/states.h"
#include "../libs/skeleton/include/skeleton/util.h"
#include "fstream"
#include "get_action.h"
#include "decode.h"
#include "helper_func.h"

using namespace pokerbots::skeleton;
using namespace std;

struct Bot {
  // PREFLOP STUFF:
  vector<vector<bool>> sb_rfi;
  vector<vector<pair<float, float>>> bb_vs_2bet;
  vector<vector<pair<float, float>>> sb_vs_3bet;
  vector<vector<pair<float, float>>> bb_vs_4bet;
  int preflop;
  pair<int,int> preflop_chart_pos;
  string cards = "23456789TJQKA";
  string suits = "shcd";
  // OTHER STUFF:
  bool bigBlind;
  int myBankroll;
  int roundNum;
  unordered_map<string, string> post_flop_data;
  string history = "";
  bool inpos = false;
  // FLOP BUCKETING:
  vector<vector<int>> computed_flops_ip = {
    {3, 3, 10}, {5, 11, 11},
    {4, 8, 9}, {1, 6, 8}, {3, 4, 12},
    {5, 6, 11}, {6, 8, 10}, {3, 5, 10},
    {8, 9, 12}, {1, 5, 6}, {2, 10, 12},
    {2, 3, 11}, {0, 2, 9}
  };
  vector<vector<int>> computed_flops_op = {
    {4, 4, 10}, {3, 9, 9},
    {0, 3, 5}, {2, 5, 7}, {6, 10, 12},
    {3, 5, 12}, {2, 9, 11}, {3, 4, 11},
    {5, 7, 12}, {4, 7, 9}, {5, 6, 10},
    {0, 2, 10}, {2, 3, 8}, {7, 9, 12}
  };
  vector<vector<int>> computed_features_ip;
  vector<vector<int>> computed_features_op;
  pair<int, vector<int>> flop_bucketing;
  vector<int> cur_flop;
  bool flop_is_monotone;
  bool mustfold;

  Bot() {
    cout << "FUCK" << endl;
    import_preflop();
    post_flop_data = get_data_69();
    cout << "preflop loaded in" << endl;
    preflop = 0;
    compute_diffs();

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
      ifstream inputFile("src/preflop_data/rfi.txt");
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

      import_preflop_floats("src/preflop_data/preflop - bb vs. 2bet.txt", &bb_vs_2bet);
      import_preflop_floats("src/preflop_data/preflop - btn vs. 3bet.txt", &sb_vs_3bet);
      import_preflop_floats("src/preflop_data/preflop - bb vs. 4bet.txt", &bb_vs_4bet);

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
        pair<int,int> p(max(num1, num2), min(num1, num2));
        return p;
    }
  }
  // A is 12, etc.
  int card_to_num_preflop(string card) {
        int num = 0;
        for (int i = 0; i < 13; i++) {
            if (card[0] == cards[i]) {
                num += 12-i;
            }
        }
        for (int i = 0; i < 4; i++) {
            if (suits[i] == card[1]) {
                num += 13 * i;
            }
        }
        return num;
    }
  void compute_diffs() {
    for (int i=0; i<computed_flops_ip.size(); i++) {
        sort(computed_flops_ip[i].begin(), computed_flops_ip[i].end());
        vector<int> diffs = {
            computed_flops_ip[i][0]-(-1),
            computed_flops_ip[i][1]-computed_flops_ip[i][0],
            computed_flops_ip[i][2]-computed_flops_ip[i][1],
            computed_flops_ip[i][0]+computed_flops_ip[i][1]+computed_flops_ip[i][2]
        };
        computed_features_ip.push_back(diffs);
    }
  }
  pair<int, vector<int>> flopbuckets(vector<int> flop, bool isInPosition) {
    sort(flop.begin(), flop.end());
    vector<int> swaps;
    for (int i=0; i<13; i++) {
        swaps.push_back(i);
    }
    vector<int> flop_features = {
        flop[0]-(-1),
        flop[1]-flop[0],
        flop[2]-flop[1],
        flop[0]+flop[1]+flop[2]
    };
    if (flop_features[1] == 0 && flop_features[2] == 0) {
        // trips
        pair<int, vector<int>> p(-1, swaps);
        return p;
    }
    if (flop_features[1] == 0 || flop_features[2] == 0) {
        int ind = flop_features[1] == 0 ? 0 : 1;
        int a = isInPosition ? computed_flops_ip[ind][0] : computed_flops_op[ind][0];
        int b = isInPosition ? computed_flops_ip[ind][2] : computed_flops_op[ind][2];
        swaps[a] = flop[0];
        swaps[flop[0]] = a;
        swaps[b] = flop[2];
        swaps[flop[2]] = b;
        pair<int, vector<int>> p(ind, swaps);
        return p;
    }
    if (flop_features[1] == 1) {
        if (isInPosition) {
            for (int i=2; i<computed_flops_ip.size(); i++) {
                if (flop[0] == computed_flops_ip[i][0] && flop[1] == computed_flops_ip[i][1]) {
                    swaps[flop[2]] = computed_flops_ip[i][2];
                    swaps[computed_flops_ip[i][2]] = flop[2];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
                if (flop[0]-1 == computed_flops_ip[i][0] && flop[1] == computed_flops_ip[i][1]) {
                    swaps[flop[0]] = flop[0]-1;
                    swaps[flop[0]-1] = flop[0];
                    swaps[flop[2]] = computed_flops_ip[i][2];
                    swaps[computed_flops_ip[i][2]] = flop[2];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
                if (flop[0] == computed_flops_ip[i][0] && flop[1]+1 == computed_flops_ip[i][1]) {
                    swaps[flop[1]+1] = flop[1];
                    swaps[flop[1]] = flop[1]+1;
                    swaps[flop[2]] = computed_flops_ip[i][2];
                    swaps[computed_flops_ip[i][2]] = flop[2];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
            }
        }
        else {
            for (int i=2; i<computed_flops_op.size(); i++) {
                if (flop[0] == computed_flops_op[i][0] && flop[1] == computed_flops_op[i][1]) {
                    swaps[flop[2]] = computed_flops_op[i][2];
                    swaps[computed_flops_op[i][2]] = flop[2];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
                if (flop[0]-1 == computed_flops_op[i][0] && flop[1] == computed_flops_op[i][1]) {
                    swaps[flop[0]] = flop[0]-1;
                    swaps[flop[0]-1] = flop[0];
                    swaps[flop[2]] = computed_flops_op[i][2];
                    swaps[computed_flops_op[i][2]] = flop[2];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
                if (flop[0] == computed_flops_op[i][0] && flop[1]+1 == computed_flops_op[i][1]) {
                    swaps[flop[1]+1] = flop[1];
                    swaps[flop[1]] = flop[1]+1;
                    swaps[flop[2]] = computed_flops_op[i][2];
                    swaps[computed_flops_op[i][2]] = flop[2];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
            }
        }
    }
    if (flop_features[2] == 1) {
        if (isInPosition) {
            for (int i=2; i<computed_flops_ip.size(); i++) {
                if (flop[1] == computed_flops_ip[i][1] && flop[2] == computed_flops_ip[i][2]) {
                    swaps[flop[0]] = computed_flops_ip[i][0];
                    swaps[computed_flops_ip[i][0]] = flop[0];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
                if (flop[1]-1 == computed_flops_ip[i][1] && flop[2] == computed_flops_ip[i][2]) {
                    swaps[flop[1]] = flop[1]-1;
                    swaps[flop[1]-1] = flop[1];
                    swaps[flop[0]] = computed_flops_ip[i][0];
                    swaps[computed_flops_ip[i][0]] = flop[0];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
                if (flop[1] == computed_flops_ip[i][1] && flop[2]+1 == computed_flops_ip[i][2]) {
                    swaps[flop[2]+1] = flop[2];
                    swaps[flop[2]] = flop[2]+1;
                    swaps[flop[0]] = computed_flops_ip[i][0];
                    swaps[computed_flops_ip[i][0]] = flop[0];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
            }
        }
        else {
            for (int i=2; i<computed_flops_op.size(); i++) {
                if (flop[1] == computed_flops_op[i][1] && flop[2] == computed_flops_op[i][2]) {
                    swaps[flop[0]] = computed_flops_op[i][0];
                    swaps[computed_flops_op[i][0]] = flop[0];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
                if (flop[1]-1 == computed_flops_op[i][1] && flop[2] == computed_flops_op[i][2]) {
                    swaps[flop[1]] = flop[1]-1;
                    swaps[flop[1]-1] = flop[1];
                    swaps[flop[0]] = computed_flops_op[i][0];
                    swaps[computed_flops_op[i][0]] = flop[0];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
                if (flop[1] == computed_flops_op[i][1] && flop[2]+1 == computed_flops_op[i][2]) {
                    swaps[flop[2]+1] = flop[2];
                    swaps[flop[2]] = flop[2]+1;
                    swaps[flop[0]] = computed_flops_op[i][0];
                    swaps[computed_flops_op[i][0]] = flop[0];
                    pair<int, vector<int>> p(i, swaps);
                    return p;
                }
            }
        }
    }

    int closest_dist = INT_MAX;
    int best_flop_index = -1;
    if (isInPosition) {
        for (int i=2; i<computed_features_ip.size(); i++) {
            int dist = (flop_features[0]-computed_features_ip[i][0])*(flop_features[0]-computed_features_ip[i][0]);
            dist += (flop_features[1]-computed_features_ip[i][1])*(flop_features[1]-computed_features_ip[i][1]);
            dist += (flop_features[2]-computed_features_ip[i][2])*(flop_features[2]-computed_features_ip[i][2]);
            dist += 0.5*(flop_features[3]-computed_features_ip[i][3])*(flop_features[3]-computed_features_ip[i][3]);
            if (dist < closest_dist) {
                closest_dist = dist;
                best_flop_index = i;
            }
        }
    }
    else {
        for (int i=2; i<computed_features_op.size(); i++) {
            int dist = (flop_features[0]-computed_features_op[i][0])*(flop_features[0]-computed_features_op[i][0]);
            dist += (flop_features[1]-computed_features_op[i][1])*(flop_features[1]-computed_features_op[i][1]);
            dist += (flop_features[2]-computed_features_op[i][2])*(flop_features[2]-computed_features_op[i][2]);
            dist += 0.5*(flop_features[3]-computed_features_op[i][3])*(flop_features[3]-computed_features_op[i][3]);
            if (dist < closest_dist) {
                closest_dist = dist;
                best_flop_index = i;
            }
        }
    }
    vector<int> computed;
    if (isInPosition) {
        computed = computed_flops_ip[best_flop_index];
    }
    else {
        computed = computed_flops_op[best_flop_index];
    }
    vector<int> flop_copy = flop;
    for (int i=0; i<3; i++) {
        if (find(computed.begin(), computed.end(), flop[i]) != computed.end()) {
            for (int j=0; j<computed.size(); j++) {
                if (computed[j] == flop[i]) {
                    computed.erase(computed.begin() + j);
                }
                if (flop_copy[j] == flop[i]) {
                    flop_copy.erase(flop_copy.begin() + j);
                }
            }
        }
    }
    for (int i=0; i<computed.size(); i++) {
        swaps[flop_copy[i]] = computed[i];
        swaps[computed[i]] = flop_copy[i];
    }
    pair<int, vector<int>> p(best_flop_index, swaps);
    return p;
  }

  /*
    Called when a new round starts. Called NUM_ROUNDS times.

    @param gameState The GameState object.
    @param roundState The RoundState object.
    @param active Your player's index.
  */
  void handleNewRound(GameInfoPtr gameState, RoundStatePtr roundState, int active) {
    myBankroll = gameState->bankroll;  // the total number of chips
    // you've gained or lost from the beginning of the game to the start of this round
    float gameClock = gameState->gameClock;  // the total number of
    // seconds your bot has left to play this game
    roundNum = gameState->roundNum;  // the round number from 1 to State.NUM_ROUNDS
    auto myCards = roundState->hands[active];  // your cards
    bigBlind = (active == 1);  // true if you are the big blind

    // Own stuff:
    preflop = 0;
    preflop_chart_pos = hand_to_chart_pos(card_to_num_preflop(myCards[0]), card_to_num_preflop(myCards[1]));
    history = "";
    flop_bucketing = {-2,{}};
    cur_flop = {};
    flop_is_monotone = false;
    mustfold = false;
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

    auto classifybet = [&]() {
      if (continueCost >= 0.2 * (pot - continueCost) && continueCost <= (pot - continueCost) * 0.7) {
        return 'H';
      } else if (continueCost >= 0.7 * (pot - continueCost) && continueCost <= (pot - continueCost) * 1.5) {
        return 'P';
      } else if (continueCost >= 1.5 * (pot - continueCost)) {
        return 'A';
      } else return 'C';
    };

    auto classifyraise = [&]() {
      if (myPip * 5.0 < oppPip) {
        return 'A';
      } else {
        return '^';
      }
    };

    if (myBankroll > 1.5*(1001-roundNum) || mustfold == true) {
        return {Action::Type::FOLD};
    }

    if (street == 0) {
      // PREFLOP
      preflop++;
      if (bigBlind) { // bigblind
        if (preflop == 1) {
          // sb limp/raise
          if (oppPip == 2) { //limp
            if (sb_rfi[preflop_chart_pos.first][preflop_chart_pos.second]) {
              return {Action::Type::RAISE, 3};
            }
            else {
              return {Action::Type::CHECK};
            }
          }
          else {
            double rand = helper_func::random_real();
            float prob_3bet = bb_vs_2bet[preflop_chart_pos.first][preflop_chart_pos.second].first;
            float prob_call = bb_vs_2bet[preflop_chart_pos.first][preflop_chart_pos.second].second + prob_3bet;
            if (rand <= prob_3bet) {
              auto raiseBounds = roundState->raiseBounds();
              minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
              maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
              int raise = 4*oppPip;
              return {Action::Type::RAISE, max(min(maxCost, raise), minCost)};
            }
            else if (rand <= prob_call) {
              return {Action::Type::CALL};
            }
            else {
              return {Action::Type::FOLD};
            }
          }
        }
        else if (preflop == 2) {
          // sb 4bets
          double rand = helper_func::random_real();
          float prob_5bet = bb_vs_4bet[preflop_chart_pos.first][preflop_chart_pos.second].first;
          float prob_call = bb_vs_4bet[preflop_chart_pos.first][preflop_chart_pos.second].second + prob_5bet;
          if (rand <= prob_5bet) {
            auto raiseBounds = roundState->raiseBounds();
            minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
            maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
            return {Action::Type::RAISE, maxCost};
          }
          else if (rand <= prob_call) {
            return {Action::Type::CALL};
          }
          else {
            return {Action::Type::FOLD};
          }
        }
        // should never get here
        return {Action::Type::FOLD};
      }
      else { // smallblind
        if (preflop == 1) {
          if (sb_rfi[preflop_chart_pos.first][preflop_chart_pos.second]) {
            return {Action::Type::RAISE, 5};
          }
          else {
            return {Action::Type::FOLD};
          }
        }
        else if (preflop == 2) { // bb 3bet
          double rand = helper_func::random_real();
          float prob_4bet = sb_vs_3bet[preflop_chart_pos.first][preflop_chart_pos.second].first;
          float prob_call = sb_vs_3bet[preflop_chart_pos.first][preflop_chart_pos.second].second + prob_4bet;
          if (rand <= prob_4bet) {
            auto raiseBounds = roundState->raiseBounds();
            minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
            maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
            int raise = 2.2*oppPip;
            return {Action::Type::RAISE, max(min(maxCost, raise), minCost)};
          }
          else if (rand <= prob_call) {
            return {Action::Type::CALL};
          }
          else {
            return {Action::Type::FOLD};
          }
        }
        else {
          // bb 5bet
          vector<pair<int,int>> call5bet = {{0,0}, {0,1}, {1,0}, {1,1}, {2,2}, {3,3}};
          if (find(call5bet.begin(), call5bet.end(), preflop_chart_pos) != call5bet.end()) {
            return {Action::Type::CALL};
          }
          else {
            return {Action::Type::FOLD};
          }
        }
      }
    }
    else if (street == 3) {
      if (legalActions.find(Action::Type::BID) != legalActions.end()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> bid_distribution(3*pot, 4*pot);
        int bid = min(bid_distribution(gen), myStack);
        return {Action::Type::BID, bid};
      }
      if ((oppBid > myBid && bigBlind) || (myBid > oppBid && !bigBlind)) {
        inpos = true;
      } else {
        inpos = false;
      }
      if (flop_bucketing.first == -2) {
        vector<int> curFlop = {helper_func::card_to_num(boardCards[0]), helper_func::card_to_num(boardCards[1]), helper_func::card_to_num(boardCards[2])};
        flop_bucketing = flopbuckets(curFlop, inpos);
        if (flop_bucketing.first == -1) {
          must_fold = true;
        }
        cur_flop = inpos ? computed_flops_ip[flop_bucketing.first] : computed_flops_op[flop_bucketing.first];
      }
      // MONOTONE
      if (boardCards[0][1] == boardCards[1][1] && boardCards[0][1] == boardCards[2][1]) {
        flop_is_monotone = true;
        cout << "MONOTONE" << " " << roundNum << endl;
        char suit = boardCards[0][1];
        if (oppBid > myBid) {
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
            if (bigBlind) {
              if (legalActions.find(Action::Type::RAISE) != legalActions.end()) {
                auto raiseBounds = roundState->raiseBounds();
                minCost = raiseBounds[0] - myPip;  // the cost of a minimum bet/raise
                maxCost = raiseBounds[1] - myPip;  // the cost of a maximum bet/raise
                return {Action::Type::RAISE, min(max(int(minCost), int(pot*4/10)), int(maxCost))};
              } else if (legalActions.find(Action::Type::CALL) != legalActions.end()) {
                return {Action::Type::CALL};

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
      else {
        // FLOP
        // RAINBOW
        // (OR TWOTONE)
        if (bigBlind) {
          if (continueCost > 0) { // they raised on us
            if (history.back() == 'C') {
              history += classifybet();
            } else {
              history += classifyraise();
            }
            if (history.back() == 'C') {
              return {Action::Type::CALL};
            }
            string next_action = get_action(post_flop_data, history, vector<int>{
                helper_func::card_to_num(boardCards[0]) % 13,
                helper_func::card_to_num(boardCards[1]) % 13,
                helper_func::card_to_num(boardCards[2]) % 13
            }, inpos);
            if (next_action == ".") {
              return {Action::Type::FOLD};
            } else if (next_action == "C") {
              return {Action::Type::CALL};
            } else if (next_action == "^") {
              return {Action::Type::RAISE, 3 * oppPip};
            } else if (next_action == "A") {
              return {Action::Type::RAISE, min(myStack, oppStack)};
            }
          } else {
          string next_action = get_action(post_flop_data, history, vector<int>{
              helper_func::card_to_num(boardCards[0]) % 13,
              helper_func::card_to_num(boardCards[1]) % 13,
              helper_func::card_to_num(boardCards[2]) % 13
          }, inpos);
          if (next_action == ".") {
            return {Action::Type::FOLD};
          } else if (next_action == "C") {
            return {Action::Type::CALL};
          } else if (next_action == "^") {
            return {Action::Type::RAISE, 3 * oppPip};
          } else if (next_action == "A") {
            return {Action::Type::RAISE, min(myStack, oppStack)};
          } else {
            if (legalActions.find(Action::Type::FOLD) != legalActions.end())
              return {Action::Type::FOLD};
            return {Action::Type::CHECK};
          }
        } }
        else {
          if (continueCost > 0) {
            if (history.back() == 'C') {
              history += classifybet();
            } else if(history.back() == '^'){
              history += 'A';
            } else {
              history += classifyraise();
            }
            if (history.back() == 'C') {
              return {Action::Type::CALL};
            }
            string next_action = get_action(post_flop_data, history, vector<int>{
                helper_func::card_to_num(boardCards[0]) % 13,
                helper_func::card_to_num(boardCards[1]) % 13,
                helper_func::card_to_num(boardCards[2]) % 13
            }, inpos);
            if (next_action == ".") {
              return {Action::Type::FOLD};
            } else if (next_action == "C") {
              return {Action::Type::CALL};
            } else if (next_action == "^") {
              return {Action::Type::RAISE, 3 * oppPip};
            } else if (next_action == "A") {
              return {Action::Type::RAISE, min(myStack, oppStack)};
            } else {
              if (legalActions.find(Action::Type::FOLD) != legalActions.end())
                return {Action::Type::FOLD};
              return {Action::Type::CHECK};
            }
          } else {
            string next_action = get_action(post_flop_data, history, vector<int>{
              helper_func::card_to_num(boardCards[0]) % 13,
              helper_func::card_to_num(boardCards[1]) % 13,
              helper_func::card_to_num(boardCards[2]) % 13

            }, inpos);
            if (next_action == ".") {
              return {Action::Type::FOLD};
            } else if (next_action == "C") {
              return {Action::Type::CALL};
            } else if (next_action == "H") {
              return {Action::Type::RAISE, pot / 2};
            } else if (next_action == "P") {
              return {Action::Type::RAISE, pot};
            }  else if (next_action == "A") {
              return {Action::Type::RAISE, min(myStack, oppStack)};
            } else {
              if (legalActions.find(Action::Type::FOLD) != legalActions.end())
                return {Action::Type::FOLD};
              return {Action::Type::CHECK};
            }
          }
        }}


      }
    else if (street == 4) {
      if (flop_is_monotone) {
        if (legalActions.find(Action::Type::CHECK) != legalActions.end()) {
          return {Action::Type::CHECK};
        }
        return {Action::Type::Fold};
      }
      else {
        // USE CFR HERE
        // USE CFR HERE
        // USE CFR HERE
        // USE CFR HERE
        // USE CFR HERE
        // USE CFR HERE
      }
    }
    else { // street == 5
      if (legalActions.find(Action::Type::CHECK) != legalActions.end()) {
        return {Action::Type::CHECK};
      }
      return {Action::Type::CALL};
    }
  }
};

/*
  Main program for running a C++ pokerbot.
*/
int main(int argc, char *argv[]) {
  auto [host, port] = parseArgs(argc, argv);
  runBot<Bot>(host, port);
  return 0;
}
