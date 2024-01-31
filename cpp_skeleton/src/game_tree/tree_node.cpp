#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cassert>
// #include "helper_func.cpp"
#include "info_set.cpp"
#include "action.cpp"

#include <ext/pb_ds/assoc_container.hpp>

using namespace std;

__gnu_pbds::gp_hash_table<string, Action*> mp;
int num = 0;

struct Node {
    // leads to all direct children in the gametree
    vector<pair<Action*, Node*>> children;
    // next action
    string action;
    double reward = 1e9;

    // if theres a terminal state it will have some non-zero reward. positive reward implies player1 wins chips.
    // negative reward implies the opposite of that.

    int turn() {
        if (action == "P12" || action == "P13" || action == "P22" || action == "P23" || action == "t" || action == "r") {
            return 3;
        } else if (action.size() % 2 == 1) {
            return 1;
        } else {
            return 2;
        }
    }
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

    Node(vector<int> board, vector<string> &history, string _action = "P12", int pot1 = 0, int pot2 = 0, vector<int> h1 = {}, vector<int> h2 = {}, int preflop = 2) { // start building the game tree from here
        auto get_action = [&](string childname, float prob = -1) -> Action* {
            string key = "";
            for (int i = 0; i < history.size(); i++) {
                auto h = history[i];
                if (h == "t") {
                    key += to_string(board[3]) + " ";
                } else if (h == "r") {
                    key += to_string(board[4]) + " ";
                }
                else if (h != "P12" && h != "P13" && h != "P22" && h != "P23" && (i == history.size() - 1 || h.size() >= history[i + 1].size())) {
                    key += h + " ";
                }
            }
            key += childname + " ";
            if (action.size() % 2 == 1) {
                for (auto c : h1)
                    key += to_string(c) + " ";
            } else {
                for (auto c : h2)
                    key += to_string(c) + " ";
            }
            if (mp.find(key) == mp.end()) {
                mp[key] = new Action(childname, prob);
            }

            // cout << key << " " << mp[key] << "\n";
            return mp[key];
        };
            num += 1;
        if (num % 1'000'00 == 0) {
            cout << num << "\n";
        }
            // cout << num << "\n";
        auto get_cards = [&]() -> vector<int> {
            vector<int> cards(13, 4);
            for (auto v : board)
                cards[v]--;
            for (auto v : h1)
                cards[v]--;
            for (auto v : h2)
                cards[v]--;
            return cards;
        };
        if (pot1 == 0 || pot2 == 0) {

            if (preflop == 2) {
                pot1 = pot2 = 6;
            } else if (preflop == 3) {
                pot1 = pot2 = 18;
            }
            pot2 += 7 * (pot1 + pot2) / 2;
        }
        action = _action;
        history.push_back(action);


        if (action == "P12" || action == "P22") {
            auto cards__ = get_cards();
            float sum_prob = 0;
            for (int i = 0; i < 13; i++) {
                for (int j = 0; j <= i; j++) {
                    if ((i != j && cards__[i] && cards__[j]) ||
                        (i == j && cards__[i] >= 2)) {
                        vector<int> h1_ = h1;
                        vector<int> h2_ = h2;
                        if (action == "P12") h1_ = {i, j};
                        else h2_ = {i, j};
                        int decksz = 52 - h1.size() - h2.size();
                        float prob = 2.0 * cards__[i] * cards__[j]  / (decksz * (decksz - 1));
                        if (i == j) {
                            prob = 1.0 * cards__[i] * (cards__[i] - 1) / (decksz * (decksz - 1));
                        }
                        sum_prob += prob;
                        children.push_back(make_pair(get_action(to_string(i) + " " + to_string(j), prob),
                            new Node(board, history, (action == "P12" ? "P23" : "F"), pot1, pot2, h1_, h2_)));
                    }
                }
            }
            

        } else if (action[action.size() - 1] == '.') {

            if (action.size() % 2 == 1) {
                reward = pot2;
            } else {
                reward = -pot1;
            }

        } else if (action == "P13" || action == "P23") {
            auto cards__ = get_cards();
            
            float sum_prob = 0;
            for (int i = 0; i < 13; i++) {
                for (int j = 0; j <= i; j++) {
                    for (int k = 0; k <= j; k++) { // CHANGE THIS BACK #WARNING TODO JKDJFKLSDJFLK
                        if ((i == j && j == k && cards__[i] >= 3) ||
                            (i == j && j != k && cards__[i] >= 2 && cards__[k]) ||
                            (i == k && j != k && cards__[i] >= 2 && cards__[j]) ||
                            (j == k && i != k && cards__[j] >= 2 && cards__[i]) ||
                            (i != j && j != k && cards__[i] && cards__[j] && cards__[k])) {
                            int decksz = 52 - h1.size() - h2.size() - board.size();
                            double prob = 6.0 * cards__[i] * cards__[j] * cards__[k] / (decksz * (decksz - 1) * (decksz - 2));
                            if (i == j && j == k) {
                                prob = 1.0 * cards__[i] * (cards__[i] - 1) * (cards__[i] - 2) /
                                    (decksz * (decksz - 1) * (decksz - 2));
                            } else if (i == j) {
                                prob = 3.0 * cards__[i] * (cards__[i] - 1) * cards__[k] /
                                    (decksz * (decksz - 1) * (decksz - 2));
                            } else if (i == k) {
                                prob = 3.0 * cards__[i] * (cards__[i] - 1) * cards__[j] /
                                    (decksz * (decksz - 1) * (decksz - 2));
                            } else if (k == j) {
                                prob = 3.0 * cards__[k] * (cards__[k] - 1) * cards__[i] /
                                    (decksz * (decksz - 1) * (decksz - 2));
                            }
                            // prob /= decksz * (decksz - 1) * (decksz - 2);
                            sum_prob += prob;
                            vector<int> h1_ = h1;
                            vector<int> h2_ = h2;
                            if (action == "P12") h1_ = {i, j, k};
                            else h2_ = {i, j, k};
                            children.push_back({get_action(to_string(i) + " " + to_string(j) + " " + to_string(k), prob),
                                new Node(board, history, (action == "P12" ? "P23" : "F"), pot1, pot2, h1_, h2_)});
                        }
                    }
                }
            }
            // cout << sum_prob << "\n";
            // if (abs(sum_prob - 1) > 0.0001) {

            //     cout << "Sum prob: " << sum_prob << endl;
            //     for (int i = 0; i < 13; i++) {
            //         cout << cards__[i] << " ";
            //     }
            //     cout << "\n";
            // }
            // // assert(abs(sum_prob - 1) < 0.0001);
        } else if (action == "r" || max(pot1, pot2) == 400) { // we have reached showdown (or someone is all-in)
            auto get_winner = [&]() {
                auto cards1 = h1;
                auto cards2 = h2;
                cards1.insert(cards1.end(), board.begin(), board.end());
                cards2.insert(cards2.end(), board.begin(), board.end());
                int ma1 = helper_func::eight_eval(cards1), ma2 = helper_func::eight_eval(cards2);


                if (ma1 > ma2) return -1;
                else if (ma1 < ma2) return 1;
                else return 0;
            };
            // history.pop_back();
            // return;
            if (board.size() == 5) {
                // omp::Hand p1 = omp::Hand::empty();
                auto win = get_winner();
                if (win == -1) {
                    reward = pot2;
                } else if (win == 1) {
                    reward = -pot1;
                } else {
                    reward = 1.0 * (pot1 + pot2) / 2 - pot1;
                }
            } else if (board.size() == 4) {
                // history.pop_back();
                // return;

                reward = 0;
                auto deck = get_cards();
                int sz = 52 - 3 - 2 - 4;
                for (int i = 0; i < 13; i++) {
                    if (deck[i]) {
                        board.push_back(i);
                        auto win = get_winner();
                        if (win == -1) {
                            reward += 1.0 * pot2 * deck[i] / sz;
                        } else if (win == 1) {
                            reward += 1.0 * -pot1 * deck[i] / sz;
                        } else {
                            reward += 1.0 * ((pot1 + pot2) / 2 - pot1) * deck[i] / sz;
                        }
                        board.pop_back();
                    }
                }


            } else {
                assert(board.size() == 3);
                reward = 0;
                auto deck = get_cards();
                // int sz = deck.size();

                for (int i = 0; i < 13; i++) {
                    for (int j = 0; j < 13; j++) {
                        if ((i == j && deck[i] >= 2) || (i != j && deck[i] && deck[j])) {
                            board.push_back(i);
                            board.push_back(j);
                            auto win = get_winner();

                            double prob = (1.0 * deck[i] / 44) * (1.0 * deck[j] / 43);
                            if (i == j) {
                                prob = (1.0 * deck[i] / 44) * (1.0 * (deck[i] - 1)) / 43;
                            }
                            if (win == -1) {
                                reward += 1.0 * pot2 * prob;
                            } else if (win == 1) {
                                reward += 1.0 * (-pot1) * prob;
                            } else {
                                reward += 1.0 * ((pot1 + pot2) / 2 - pot1) * prob;
                            }
                            board.pop_back();
                            board.pop_back();
                        }
                    }
                }
          
            }
        } else if (action == "t" || action == "r") { // CHANCE NODE
            auto cards = get_cards();
            for (int i = 0; i < 13; i++) {
                if (cards[i]) {
                    board.push_back(i);
                    children.push_back({get_action(to_string(i), 1.0 * double(cards[i]) / double(52 - 3 - 2 - 3)), new Node(board, history, action == "t" ? "T" : "R", pot1, pot2, h1, h2)});
                    board.pop_back();
                }
            }
        } else if (action == "F" || action == "T" || action == "R" || action == "FC" || action == "TC" || action == "RC") {
            for (auto decision : {"C", "H", "P", "A"}) {
                if (action.length() >= 2 && decision == string("C")) {
                    if (action[0] == 'F') {
                        children.push_back({get_action(decision), new Node(board, history, "t", pot1, pot2, h1, h2)});
                    }
                    else if (action[0] == 'T') {
                        children.push_back({get_action(decision), new Node(board, history, "r", pot1, pot2, h1, h2)});
                    }
                    else {
                        children.push_back({get_action(decision), new Node(board, history, "S", pot1, pot2, h1, h2)});
                    }
                    continue; 
                }
                if (decision == string("H")) {
                    if ((pot1 + pot2) / 2 >= min(400 - pot1, 400 - pot2)) {
                        continue;
                    }
                }
                if (decision == string("P")) {
                    if ((pot1 + pot2) >= min(400 - pot1, 400 - pot2)) {
                        continue;
                    }
                }
                children.push_back({get_action(decision), new Node(board, history, action + string(decision), pot1, pot2, h1, h2)});
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
            }
            children.push_back({get_action("."), new Node(board, history, action + ".", pot1 + bet[0], pot2 + bet[1], h1, h2)});
            // CALL
            if (action[0] == 'F') children.push_back({get_action("C"), new Node(board, history, "t", pot1 + max(bet[0], bet[1]), pot2 + max(bet[0], bet[1]), h1, h2)});
            else if (action[0] == 'T') children.push_back({get_action("C"), new Node(board, history, "r", pot1 + max(bet[0], bet[1]), pot2 + max(bet[0], bet[1]), h1, h2)});
            else children.push_back({get_action("C"), new Node(board, history, "S", pot1 + max(bet[0], bet[1]), pot2 + max(bet[0], bet[1]), h1, h2)});
            // RAISE
            if (action[action.size() - 1] == 'A') {
                // TRIVIALLY YOU CANNOT RAISE
            } else if (action[action.size() - 1] == '^') {
                // RAISE ALL-IN
                children.push_back({get_action("A"), new Node(board, history, action + "A", pot1, pot2, h1, h2)});
            } else {
                // RAISE ALL-IN
                children.push_back({get_action("A"), new Node(board, history, action + "A", pot1, pot2, h1, h2)});
                if (3 * max(bet[0], bet[1]) < min(400 - pot1, 400 - pot2)) {
                    children.push_back({get_action("^"), new Node(board, history, action + "^", pot1, pot2, h1, h2)});
                }
            }
            // CALL
            // RAISE
            // FOLD

        }

        history.pop_back();
    }
};


// int main() {
//     vector<string> v;
//     Node *a = new Node(vector<int>({8, 12, 10}), v);
// }
