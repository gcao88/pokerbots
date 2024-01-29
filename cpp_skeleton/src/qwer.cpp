#include <vector>
#include <iostream>
#include <string>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <algorithm>
#include "game_tree/tree_node.cpp"
using namespace std;

float random_num() {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> random(0, 1);
    return random(gen);
};

float epsilon = 0.05;
float beta_ = 1e6;
float tau = 1000;
float walk_tree(Node* h, int player, float q) {
    if (h->children.empty()) {
        if (player == 1) return h->reward/q;
        else return -h->reward/q;
    }
    else if (h->turn() == 3) {
        float p_counter = 0;
        float rand = random_num();
        for (auto [a, child] : h->children) {
            p_counter += a->prob;
            if (rand < p_counter) {
                return walk_tree(child, player, q);
            }
        }
    }
    float total_regret = 0;
    unordered_map<Action*, float> sigma;
    for (auto [a, child] : h->children) {
        total_regret += max(0.0f, a->r);
    }
    for (auto [a, child] : h->children) {
        if (total_regret == 0) sigma[a] = 1.0f/h->children.size();
        else sigma[a] = max(0.0f, a->r)/total_regret;
    }

    if (h->turn() != player) {
        for (auto [a, child] : h->children) {
            a->s += sigma[a]/q;
        }
        float p_counter = 0;
        float rand = random_num();
        for (auto [a, child] : h->children) {
            p_counter += sigma[a];
            if (rand < p_counter) {
                return walk_tree(child, player, q);
            }
        }
        if (p_counter != 1) {
            cout << p_counter << endl;
            throw exception();
        }
    }

    float total_s = 0;
    unordered_map<Action*, float> v;
    for (auto [a, child] : h->children) {
        total_s += a->s;
    }
    for (auto [a, child] : h->children) {
        float rho = min(1.0f, max(epsilon, (beta_ + tau * a->s)/(beta_ + total_s)));
        v[a] = 0;
        if (random_num() < rho) {
            v[a] = walk_tree(child, player, q*rho);
        }
    }
    float expected_v = 0;
    for (auto [a, child] : h->children) {
        expected_v += sigma[a] * v[a];
    }
    for (auto [a, child] : h->children) {
        a->r += v[a] - expected_v;
    }
    return expected_v;
}

void encode_strategy(Node* h, int player, vector<int>* strategy, unordered_set<Action*>* added) {
    if (h->turn() != player) {
        for (auto [a, child] : h->children) {
            encode_strategy(child, player, strategy, added);
        }
    }
    else {
        for (auto [a, child] : h->children) {
            if (added->find(a) != added->end()) return;
        }

        float total_s = 0;
        for (auto [a, child] : h->children) {
            total_s += a->s;
        }
        float p_counter = 0;
        float rand = random_num();
        for (int i = 0; i < h->children.size(); i++) {
            auto& [a, child] = h->children[i];
            p_counter += a->s / total_s;
            if (rand < p_counter) {
                added->insert(a);
                //cout << a->action << " " << i << endl;
                strategy->push_back(i);
                encode_strategy(child, player, strategy, added);
                return;
            }
        }
    }
}



int main() {
    vector<string> a;
    Node* root = new Node(vector<int>{6, 8, 10}, a);

    for (int j=0; j<100000; j++) {
        cout << 1000*j << endl;
        for (int i=0; i<1000; i++) {
            walk_tree(root, i%2+1, 1);
        }

        vector<int>* strategy1 = new vector<int>();
        encode_strategy(root, 1, strategy1, new unordered_set<Action*>());

        ofstream fout("data.txt");

        for (int i = 0; i < strategy1->size(); i++) {
            fout << strategy1->at(i) << endl;
        }
        fout << endl;
    }

}
