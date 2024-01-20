#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "helper_func.cpp"

using namespace std;

struct Node {
    // leads to all direct children in the gametree
    unordered_map<string, Node *> children;
    // next action
    string action;
	array<int, 2> pots;

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
    Node(vector<int> cards, string _action = "P12", int pot1 = 0, int pot2 = 0, int preflop = -1, int auction = -1) { // start building the game tree from here
		if (preflop == 2) {
			pots[0] = pots[1] = 6;
		} else if (preflop == 3) {
			pots[0] = pots[1] = 18;
		} 
		if (preflop != -1) {
			pots[helper_func::random_number(0, 1)] = pots[helper_func::random_number(0, 1)] * 7 / 2; 
		}
        action = _action;
        if (action == "P12" || action == "P22") {
            for (int i = 0; i < cards.size(); i++) {
                for (int j = i + 1; j < cards.size(); j++) {
                    auto c1 = cards[i];
                    auto c2 = cards[j];
                    auto new_cards = cards;
                    new_cards.erase(find(new_cards.begin(), new_cards.end(), c1));
                    new_cards.erase(find(new_cards.begin(), new_cards.end(), c2));
                    children[to_string(c1) + " " + to_string(c2)] =
                        new Node(new_cards, (action == "P12") ? "P23" : "F", pots[0], pots[1]);
                }
            }
        }
        else if (action == "P13" || action == "P23") {
            for (int i = 0; i < cards.size(); i++) {
                for (int j = i + 1; j < cards.size(); j++) {
                    for (int k = j + 1; k < cards.size(); k++) {
                        auto c1 = cards[i];
                        auto c2 = cards[j];
                        auto c3 = cards[k];
                        auto new_cards = cards;
                        new_cards.erase(find(new_cards.begin(), new_cards.end(), c1));
                        new_cards.erase(find(new_cards.begin(), new_cards.end(), c2));
                        new_cards.erase(find(new_cards.begin(), new_cards.end(), c3));
                        children[to_string(c1) + " " + to_string(c2) + " " + to_string(c3)] =
                            new Node(new_cards, (action == "P13") ? "P22" : "F", pots[0], pots[1]);
                    }
                }
            }
        }
        else if (action == "F" || action == "T" || action == "R" || action == "FC" || action == "TC" || action == "RC") {
            for (auto decision : {"C", "H", "P", "A"}) {
                if (action.length() >= 2) {
                    if (action[0] == 'C') {
                        children[decision] = new Node(cards, "F");
                    }
                    else if (action[0] == 'F') {
                        children[decision] = new Node(cards, "R");
                    }
                    else {
                        // TERMINAL STATE => Showdown
                    }
                }
                children[decision] = new Node(cards, action + string(decision));
            }
        }
        else if (action == "TH" || action == "TP" || action == "TA" || action == "FH" || action == "FP" ||
                 action == "FA" || action == "RH" || action == "RP" || action == "RA") {     // they've bet into you

        }
    }
};
