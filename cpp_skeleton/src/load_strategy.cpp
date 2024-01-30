#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <algorithm>
#include "game_tree/tree_node.cpp"
using namespace std;


void decode_strategy(Node* h, int i, vector<int>* strategy, int* ind, unordered_set<Action*>* added) {
    if (h->turn() != i) {
        for (auto [a, child] : h->children) {
            decode_strategy(child, i, strategy, ind, added);
        }
    }
    else {
        for (auto [a, child] : h->children) {
            if (added->find(a) != added->end()) return;
        }

        auto& [a, child] = h->children[(*strategy)[*ind]];
        a->s = 1e9; //take this line
        added->insert(a);
        (*ind)++;
        decode_strategy(child, i, strategy, ind, added);
    }
}

int main() {
    ifstream input("strategy1.txt");
    vector<int>* strategy1 = new vector<int>((std::istream_iterator<int>(input)), std::istream_iterator<int>());

    cout << strategy1->size() << endl;

    vector<string> a;
    Node* root = new Node(vector<int>{6, 8, 10}, a);

    
    int ind1 = 0;
    decode_strategy(root, 1, strategy1, &ind1, new unordered_set<Action*>());
    
    assert (ind1 == strategy1->size());
}