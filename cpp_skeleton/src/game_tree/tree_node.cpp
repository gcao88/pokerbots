#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "helper_func.cpp"
#include "../../libs/skeleton/include/omp/HandEvaluator.h"
#include "../../libs/skeleton/include/omp/Hand.h"
// #include "../../HandEvaluator.h"
// #include "Hand.h"
#include <unordered_map>
#include <unordered_set>
#include "info_set.cpp"

using namespace std;

struct Node {
    // leads to all direct children in the gametree
    unordered_map<string, Node *> children;
    // next action
    string action;
    int reward = 1e9; // if theres a terminal state it will have some non-zero reward. positive reward implies player1 wins chips.
    // negative reward implies the opposite of that.

    /*
        key for what the actions mean:
        P12 means P1 is going to get 2 cards at this stage, P13 3 cards
        similarly P22 P23
        F means that you have just arrived on the flop
        FC means that you were checked into
        FH means that you were bet half pot into
        FP means that you were bet pot into
        FA means that you were bet all-in
    */
    // WARNING: you should put cards in that dont have the cards on the flop
    Node(vector<int> board, vector<string> &history, string _action = "P12", int pot1 = 0, int pot2 = 0, vector<int> h1 = {}, vector<int> h2 = {}, int preflop = -1) { // start building the game tree from here
        auto get_cards = [&]() -> vector<int> {
            vector<int> cards(52);
            iota(cards.begin(), cards.end(), 0);
            for (auto c : board) {
                cards.erase(find(cards.begin(), cards.end(), c));
            }
            for (auto c : h1) {
                cards.erase(find(cards.begin(), cards.end(), c));
            }
            for (auto c : h2) {
                cards.erase(find(cards.begin(), cards.end(), c));
            }
            return cards;
        };
		if (preflop == 2) {
            pot1 = pot2 = 6;
		} else if (preflop == 3) {
            pot1 = pot2 = 18;
		}
		if (preflop != -1) {
            (helper_func::random_number(0, 1) ? pot1 : pot2) *= 7;
            (helper_func::random_number(0, 1) ? pot1 : pot2) /= 2;
		}
        action = _action;
        history.push_back(action);
        if (action == "P12" || action == "P22") {
            auto cards = get_cards();
            for (int i = 0; i < cards.size(); i++) {
                for (int j = i + 1; j < cards.size(); j++) {
                    auto c1 = cards[i];
                    auto c2 = cards[j];
                    vector<int> h1_ = h1;
                    vector<int> h2_ = h2;
                    if (action == "P12") h1_ = {cards[i], cards[j]};
                    else h2_ = {cards[i], cards[j]};
                    children[to_string(c1) + " " + to_string(c2)] =
                        new Node(board, history, (action == "P12") ? "P23" : "F", pot1, pot2, h1_, h2_);
                }
            }
        }
        else if (action == "P13" || action == "P23") {
            auto cards = get_cards();
            for (int i = 0; i < cards.size(); i++) {
                for (int j = i + 1; j < cards.size(); j++) {
                    for (int k = j + 1; k < cards.size(); k++) {
                        auto c1 = cards[i];
                        auto c2 = cards[j];
                        auto c3 = cards[k];
                        vector<int> h1_ = h1;
                        vector<int> h2_ = h2;
                        if (action == "P13") h1_ = {cards[i], cards[j], cards[k]};
                        else h2_ = {cards[i], cards[j], cards[k]};
                        children[to_string(c1) + " " + to_string(c2) + " " + to_string(c3)] =
                            new Node(board, (action == "P13") ? "P22" : "F", pot1, pot2, h1_, h2_); 
                    }
                }
            }
        } else if (action == "t" || action == "r") { // CHANCE NODE
            auto cards = get_cards();
            for (auto c : cards) {
                board.push_back(c); 
                children[to_string(c)] = new Node(board, action == "t" ? "T" : "R", pot1, pot2, h1, h2); 
                board.pop_back(); 
            }
        } else if (action == "S" || max(pot1, pot2) == 400) { // we have reached showdown (or someone is all-in)
            auto get_winner = [&]() {
                auto cards1 = h1;
                auto cards2 = h2;
                cards1.insert(cards1.end(), board.begin(), board.end());
                cards2.insert(cards2.end(), board.begin(), board.end());
                uint16_t ma1 = 0, ma2 = 0;
                omp::HandEvaluator eval;
                for (int i = 0; i < 8; i++) {
                    omp::Hand e1 = omp::Hand::empty();
                    omp::Hand e2 = omp::Hand::empty();
                    for (int j = 0; j < cards1.size(); j++) {
                        if (cards1.size() == 8) {
                            if (i != j) {
                                e1 += omp::Hand(cards1[j]);
                            }
                        } else e1 += omp::Hand(cards1[j]);
                    }
                    for (int j = 0; j < cards2.size(); j++) {
                        if (cards2.size() == 8) {
                            if (i != j) {
                                e2 += omp::Hand(cards2[j]);
                            } else e2 += omp::Hand(cards2[j]);
                        }
                    }
                    ma1 = max(eval.evaluate(e1), ma1);
                    ma2 = max(eval.evaluate(e2), ma2);
                }
                if (ma1 > ma2) return -1;
                else if (ma1 < ma2) return 1;
                else return 0;
            };
            if (board.size() == 5) {
                // omp::Hand p1 = omp::Hand::empty();
                auto win = get_winner();
                if (win == -1) {
                    reward = pot2;
                } else if (win == 1) {
                    reward = -pot1;
                } else {
                    reward = (pot1 + pot2) / 2 - pot1;
                }
            } else if (board.size() == 4) {
                reward = 0;
                auto deck = get_cards();
                int sz = deck.size();
                for (auto c : deck) {
                    board.push_back(c);
                    auto win = get_winner();
                    if (win == -1) {
                        reward += pot2 / sz;
                    } else if (win == 1) {
                        reward += -pot1 / sz;
                    } else {
                        reward += ((pot1 + pot2) / 2 - pot1) / sz;
                    }
                    board.pop_back();
                }
            } else {
                assert(board.size() == 3);
                reward = 0;
                auto deck = get_cards();
                int sz = deck.size();
                for (int i = 0; i < sz; i++) {
                    for (int j = 0; j < i; j++) {
                        auto c1 = deck[i];
                        auto c2 = deck[j];
                        board.push_back(c1);
                        board.push_back(c2);
                        auto win = get_winner();
                        if (win == -1) {
                            reward += pot2 / (sz * (sz - 1) / 2);
                        } else if (win == 1) {
                            reward += -pot1 / (sz * (sz - 1) / 2);
                        } else {
                            reward += ((pot1 + pot2) / 2 - pot1) / (sz * (sz - 1) / 2);
                        }
                        board.pop_back();
                        board.pop_back();
                    }
                }
            }
        } else if (action[action.size() - 1] == '.') {
            if (action.size() % 2 == 1) {
                reward = pot2;
            } else {
                reward = -pot1;
            }
        } else if (action == "F" || action == "T" || action == "R" || action == "FC" || action == "TC" || action == "RC") {
            for (auto decision : {"C", "H", "P", "A"}) {
                if (action.length() >= 2 && decision == "C") {
                    if (action[0] == 'F') {
                        children[decision] = new Node(board, history, "t", pot1, pot2, h1, h2);
                    }
                    else if (action[0] == 'T') {
                        children[decision] = new Node(board, history, "r", pot1, pot2, h1, h2);
                    }
                    else {
                        children[decision] = new Node(board, history, "S", pot1, pot2, h1, h2);
                    }
                }
                if (decision == "H") {
                    if ((pot1 + pot2) / 2 >= min(400 - pot1, 400 - pot2)) {
                        continue;
                    }
                }
                if (decision == "P") {
                    if ((pot1 + pot2) >= min(400 - pot1, 400 - pot2)) {
                        continue;
                    }
                } 
                children[decision] = new Node(board, action + string(decision), pot1, pot2, h1, h2);
            }
        }
        else if (action[action.size() - 1] == 'H' || action[action.size() - 1] == 'P' || action[action.size() - 1] == 'A' || action[action.size() - 1] == '^') {     // they've bet into you
            int bet[2] = {0, 0};
            for (int i = 1; i < action.size(); i++) {
                if (action[i] == 'H') {
                    bet[i & 1 ^ 1] = (pot1 + pot2) / 2;
                } else if (action[i] == 'P') {
                    bet[i & 1 ^ 1] = pot1 + pot2;
                } else if (action[i] == 'A') {
                    bet[i & 1 ^ 1] = min(400 - pot1, 400 - pot2);
                } else if (action[i] == '^') {
                    bet[i & 1 ^ 1] = 3 * bet[i & 1];
                }
                // FOLD 
                children["."] = new Node(board, action + ".", pot1 + bet[0], pot2 + bet[1], h1, h2); 
                // CALL 
                if (action[0] == 'F') children["C"] = new Node(board, "t", pot1 + max(bet[0], bet[1]), pot2 + max(bet[0], bet[1]), h1, h2); 
                else if (action[0] == 'T') children["C"] = new Node(board, "r", pot1 + max(bet[0], bet[1]), pot2 + max(bet[0], bet[1]), h1, h2); 
                else children["C"] = new Node(board, "S", pot1 + max(bet[0], bet[1]), pot2 + max(bet[0], bet[1]), h1, h2); 
                // RAISE 
                if (action[action.size() - 1] == 'A') {
                    // TRIVIALLY YOU CANNOT RAISE
                } else if (action[action.size() - 1] == '^') {
                    // RAISE ALL-IN 
                    children["A"] = new Node(board, action + "A", pot1, pot2, h1, h2);
                } else {
                    // RAISE ALL-IN 
                    children["A"] = new Node(board, action + "A", pot1, pot2, h1, h2); 
                    if (3 * max(bet[0], bet[1]) < (400 - pot1, 400 - pot2)) {
                        children["^"] = new Node(board, action + "^", pot1, pot2, h1, h2); 
                    }
                }

            }
            // CALL
            // RAISE
            // FOLD

        } 

    }
};


int main() {
    vector<string> v;
    Node *a = new Node(vector<int>({0, 1, 2}), v); 
}