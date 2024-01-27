#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
// #include "helper_func.cpp"
#include "info_set.cpp"
#include "../../libs/skeleton/include/pokerengine/SevenEval.h"

using namespace std;

unordered_map<string, InfoSet*> mp;

struct Node {
    // leads to all direct children in the gametree
    /*
    TH (FC^A)
    */
    map<string, Node *> children;
    InfoSet *infoset = NULL; 
    // next action
    string action;
    double reward = 1e9; // if theres a terminal state it will have some non-zero reward. positive reward implies player1 wins chips. 
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
    Node(vector<int> board, vector<string> &history, string _action = "P12", int pot1 = 0, int pot2 = 0, vector<int> h1 = {}, vector<int> h2 = {}, int preflop = 2) { // start building the game tree from here

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
            if (helper_func::random_number(0, 1) == 0) {
                pot1 = pot1 * 7 / 2; 
            } else pot2 = pot2 * 7 / 2;
        }
        action = _action;
        history.push_back(action);

        if (action != "P12" && action != "P13" && action != "P22" 
            && action != "P23" && action != "t" && action != "r") {
            string key = "";
            for (auto h : history) {
                key += h + " "; 
            }
            if (action.size() % 2 == 0) {
                for (auto c : h1) 
                    key += to_string(c) + " "; 
            } else {
                for (auto c : h2) 
                    key += to_string(c) + " "; 
            }
            if (mp.find(key) != mp.end()) {
                mp[key] = new InfoSet(); 
            }
            infoset = mp[key]; 
        }
        if (action == "P12" || action == "P22") {
            auto cards__ = get_cards(); 

            for (int i = 0; i < 13; i++) {
                for (int j = 0; j <= i; j++) {
                    if ((i != j && cards__[i] && cards__[j]) || 
                        (i == j && cards__[i] >= 2)) {
                        vector<int> h1_ = h1; 
                        vector<int> h2_ = h2; 
                        if (action == "P12") h1_ = {i, j};
                        else h2_ = {i, j};
                        children[to_string(i) + " " + to_string(j)] =
                            new Node(board, history, (action == "P12" ? "P23" : "F"), pot1, pot2, h1_, h2_);
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
            for (int i = 0; i < 13; i++) {
                for (int j = 0; j <= i; j++) {
                    for (int k = 0; k <= i; k++) {
                        if ((i == j && j == k && cards__[i] >= 3) || 
                            (i == j && cards__[i] >= 2 && cards__[k]) || 
                            (i == k && cards__[i] >= 2 && cards__[j]) ||
                            (j == k && cards__[j] >= 2 && cards__[i]) ||
                            (cards__[i] && cards__[j] && cards__[k])) {

                            vector<int> h1_ = h1; 
                            vector<int> h2_ = h2; 
                            if (action == "P12") h1_ = {i, j, k};
                            else h2_ = {i, j, k};
                            children[to_string(i) + " " + to_string(j) + " " + to_string(k)] =
                                new Node(board, history, (action == "P13" ? "P22" : "F"), pot1, pot2, h1_, h2_);
                        } 
                    }
                }
            }
        } else if (action == "S" || max(pot1, pot2) == 400) { // we have reached showdown (or someone is all-in)
            auto get_winner = [&]() {
                auto cards1 = h1; 
                auto cards2 = h2; 
                cards1.insert(cards1.end(), board.begin(), board.end()); 
                cards2.insert(cards2.end(), board.begin(), board.end()); 
                // for (auto v : cards1) {
                //     cout << helper_func::num_to_card(v) << " ";
                // }
                // cout << "\n";
                // for (auto v : cards2) {
                //     cout << helper_func::num_to_card(v) << " ";
                // }
                // cout << "\n";
                uint16_t ma1 = 0, ma2 = 0; 
                for (int i = 0; i < 8; i++) {
                    vector<int> A; 
                    vector<int> B; 
                    int counter = 0; 
                    auto convert = [&](int C) {
                        auto suit = counter++ % 4;
                        auto card = C;
                        return suit + ((13 - card) % 13) * 4;
                    };
                    for (int j = 0; j < cards1.size(); j++) {
                        if (cards1.size() == 8) {
                            if (i != j) {
                                A.push_back(convert(cards1[j]));
                            }
                        } else {
                            A.push_back(convert(cards1[j]));
                        }
                    } 
                    for (int j = 0; j < cards2.size(); j++) {
                        if (cards2.size() == 8) {
                            if (i != j) {
                                B.push_back(convert(cards2[j]));
                            } 
                        } else {
                            B.push_back(convert(cards2[j]));
                        }
                    }

                    ma1 = max(ma1, SevenEval::GetRank(A[0], A[1], A[2], A[3], A[4], A[5], A[6])); 
                    ma2 = max(ma2, SevenEval::GetRank(B[0], B[1], B[2], B[3], B[4], B[5], B[6])); 
                }
                // if (ma1 > ma2) {
                //     cout << "P1\n";
                // } else if (ma1 < ma2) {
                //     cout << "P2\n";
                // } else {
                //     cout << "TIE\n";
                // }
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
                    reward = 1.0 * (pot1 + pot2) / 2 - pot1; 
                }
            } else if (board.size() == 4) {
                reward = 0;
                auto deck = get_cards();
                int sz = deck.size();
                for (auto c : deck) {
                    board.push_back(c);
                    auto win = get_winner();
                    if (win == -1) {
                        reward += 1.0 * pot2 / sz;
                    } else if (win == 1) {
                        reward += 1.0 * -pot1 / sz; 
                    } else {
                        reward += 1.0 * ((pot1 + pot2) / 2 - pot1) / sz; 
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
                            reward += 1.0 * pot2 / (sz * (sz - 1) / 2); 
                        } else if (win == 1) {
                            reward += 1.0 * -pot1 / (sz * (sz - 1) / 2); 
                        } else {
                            reward += 1.0 * ((pot1 + pot2) / 2 - pot1) / (sz * (sz - 1) / 2); 
                        }
                        board.pop_back();
                        board.pop_back();
                    }
                }
                cout << pot1 << " " << pot2 << "\n";
                for (auto v : board) {
                    cout << helper_func::num_to_card(v) << " ";
                }
                cout << "\n";
                for (auto v : h1) {
                    cout << helper_func::num_to_card(v) << " ";
                }
                cout << "\n";
                for (auto v : h2) {
                    cout << helper_func::num_to_card(v) << " ";
                }
                cout << "\n";
                cout << reward << "\n";
            }
        } else if (action == "t" || action == "r") { // CHANCE NODE
            // cout << "HISTORY: "; 
            // for (int x : h1) {
            //     cout << x << " ";
            // }
            // for (int x : h2) {
            //     cout << x << " ";
            // }

            // cout << pot1 << " "; 
            // cout << pot2 << " ";
            // for (string s : history)
            //     cout << s << " "; 
            // cout << endl;
            history.pop_back();
            return;// end early
            auto cards = get_cards(); 
            for (auto c : cards) {
                board.push_back(c); 
                children[to_string(c)] = new Node(board, history, action == "t" ? "T" : "R", pot1, pot2, h1, h2); 
                board.pop_back(); 
            }
        } else if (action == "F" || action == "T" || action == "R" || action == "FC" || action == "TC" || action == "RC") {
            for (auto decision : {"C", "H", "P", "A"}) {
                if (action.length() >= 2 && decision == string("C")) {
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
                children[decision] = new Node(board, history, action + string(decision), pot1, pot2, h1, h2);
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
            children["."] = new Node(board, history, action + ".", pot1 + bet[0], pot2 + bet[1], h1, h2); 
            // CALL 
            if (action[0] == 'F') children["C"] = new Node(board, history, "t", pot1 + max(bet[0], bet[1]), pot2 + max(bet[0], bet[1]), h1, h2); 
            else if (action[0] == 'T') children["C"] = new Node(board, history, "r", pot1 + max(bet[0], bet[1]), pot2 + max(bet[0], bet[1]), h1, h2); 
            else children["C"] = new Node(board, history, "S", pot1 + max(bet[0], bet[1]), pot2 + max(bet[0], bet[1]), h1, h2); 
            // RAISE 
            if (action[action.size() - 1] == 'A') {
                // TRIVIALLY YOU CANNOT RAISE 
            } else if (action[action.size() - 1] == '^') {
                // RAISE ALL-IN 
                children["A"] = new Node(board, history, action + "A", pot1, pot2, h1, h2);
            } else {
                // RAISE ALL-IN 
                children["A"] = new Node(board, history, action + "A", pot1, pot2, h1, h2); 
                if (3 * max(bet[0], bet[1]) < min(400 - pot1, 400 - pot2)) {
                    children["^"] = new Node(board, history, action + "^", pot1, pot2, h1, h2); 
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
    Node *a = new Node(vector<int>({8, 13, 44}), v); 
}