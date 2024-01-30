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
        for (const auto& [a, child] : h->children) {
            p_counter += a->prob;
            if (rand < p_counter) {
                return walk_tree(child, player, q);
            }
        }
    }
    float total_regret = 0;
    unordered_map<Action*, float> sigma;
    for (const auto& [a, child] : h->children) {
        total_regret += max(0.0f, a->r);
    }
    for (const auto& [a, child] : h->children) {
        if (total_regret == 0) sigma[a] = 1.0f/h->children.size();
        else sigma[a] = max(0.0f, a->r)/total_regret;
    }

    if (h->turn() != player) {
        for (const auto& [a, child] : h->children) {
            a->s += sigma[a]/q;
        }
        float p_counter = 0;
        float rand = random_num();
        for (const auto& [a, child] : h->children) {
            p_counter += sigma[a];
            if (rand < p_counter) {
                return walk_tree(child, player, q);
            }
        }
        cout << "ERROR " << p_counter << endl;
        return walk_tree(h->children[0].second, player, q);
    }

    float total_s = 0;
    unordered_map<Action*, float> v;
    for (const auto& [a, child] : h->children) {
        total_s += a->s;
    }
    for (const auto& [a, child] : h->children) {
        float rho = min(1.0f, max(epsilon, (beta_ + tau * a->s)/(beta_ + total_s)));
        v[a] = 0;
        if (random_num() < rho) {
            v[a] = walk_tree(child, player, q*rho);
        }
    }
    float expected_v = 0;
    for (const auto& [a, child] : h->children) {
        expected_v += sigma[a] * v[a];
    }
    for (const auto& [a, child] : h->children) {
        a->r += v[a] - expected_v;
    }
    return expected_v;
}

void encode_strategy(Node* h, string history, int player, vector<int>* strategy, unordered_map<string, string>* strategy_str, unordered_set<Action*>* added) {
    if (h->children.empty()) {
        return;
    }
    if (h->turn() != player) {
        for (auto [a, child] : h->children) {
            encode_strategy(child, history + a->label, player, strategy, strategy_str, added);
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
                (*strategy_str)[history] = a->label;
                encode_strategy(child, history + a->label, player, strategy, strategy_str, added);
                return;
            }
        }
    }
}

void list_actions(Node* h, vector<Action*>* actions, unordered_set<Action*>* added) {
    for (auto [a, child] : h->children) {
        if (added->find(a) == added->end()) {
            actions->push_back(a);
            added->insert(a);
        }
        list_actions(child, actions, added);
    }
}


int main() {
    vector<string> a;
    Node* root = new Node(vector<int>{6, 8, 10}, a);

    vector<Action*>* actions = new vector<Action*>();
    list_actions(root, actions, new unordered_set<Action*>());

    cout << "total actions: " << actions->size() << endl;

    for (int j=0; j<10000; j++) {
        cout << j << endl;
        for (int i=0; i<10000; i++) {
            walk_tree(root, i%2+1, 1);
        }

        vector<int>* strategy1 = new vector<int>();
        unordered_map<string, string>* strategy_str1 = new unordered_map<string, string>();
        encode_strategy(root, "", 1, strategy1, strategy_str1, new unordered_set<Action*>());
        ofstream fout1("strategy1.txt");
        for (auto [history, a_label] : *strategy_str1) {
            fout1 << history << " " << a_label << endl;
        }
        fout1 << endl;

        vector<int>* strategy2 = new vector<int>();
        unordered_map<string, string>* strategy_str2 = new unordered_map<string, string>();
        encode_strategy(root, "", 2, strategy2, strategy_str2, new unordered_set<Action*>());
        ofstream fout2("strategy2.txt");
        for (auto [history, a_label] : *strategy_str2) {
            fout2 << history << " " << a_label << endl;
        }
        fout2 << endl;
    }

}
