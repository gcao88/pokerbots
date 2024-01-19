#include <vector>
#include <iostream>
#include <string>

using namespace std; 

struct Node {
    // leads to all direct children in the gametree 
    vector<pair<string, Node*>> children; 
    // next action 
    string action;  

    /*
        key for what the actions mean: 
        P12 means P1 is going to get 2 cards at this stage, P13 3 cards
        similarly P22 P23
    */
    // WARNING: you should put cards in that dont have the cards on the flop 
    Node(vector<int> cards, string _action = "P12") { // start building the game tree from here 
        action = _action;
        if (action == "P12" || action == "P22") {
            for (int i = 0; i < cards.size(); i++) {
                for (int j = i + 1; j < cards.size(); j++) {
                    auto c1 = cards[i];
                    auto c2 = cards[j]; 
                    auto new_cards = cards; 
                    new_cards.erase(find(new_cards.begin(), new_cards.end(), c1)); 
                    new_cards.erase(find(new_cards.begin(), new_cards.end(), c2)); 
                    if (action == "P12")
                        children.push_back(make_pair(to_string(c1) + " " + to_string(c2), new Node(new_cards, "P23"))); 
                    else
                        children.push_back(make_pair(to_string(c1) + " " + to_string(c2), new Node(new_cards, "F")));
                }
            }
        } else if(action == "P13" || action == "P23") {
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
                        if (action == "P13")
                            children.push_back(make_pair(to_string(c1) + " " + to_string(c2) + " " + to_string(c3), new Node(new_cards, "P22"))); 
                        else
                            children.push_back(make_pair(to_string(c1) + " " + to_string(c2) + " " + to_string(c3), new Node(new_cards, "F")));
                    }
                }
            }
        }
    } 

};
