#include <vector>
#include <iostream>
#include <string>

using namespace std; 

struct Node {
    // leads to all direct children in the gametree 
    vector<pair<string, Node*>> children; 
    // stores the flop for the current gametree we are solving
    // next action 
    string action;

    Node(vector<string> cards, string _action="P1C") { // start building the game tree from here 
        action = _action;
        if (action == "P1C") { // this action means we need to give player 1 a card

        }
    } 

};
