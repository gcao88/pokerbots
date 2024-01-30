#include <vector>
#include <iostream>
#include <string>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <algorithm>
#include "game_tree/tree_node.cpp"
// #pragma GCC target ("avx")
#pragma GCC optimization ("O3")
#pragma GCC optimization ("unroll-loops")
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
long long walk_tree_counter = 0;
float walk_tree(Node* h, int player, float q) {
    walk_tree_counter++;
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
        cout << "ERROR 3 " << p_counter;
        return 0;
    }
    else {
        float total_regret = 0;
        vector<float> sigma;
        for (const auto& [a, child] : h->children) {
            total_regret += max(0.0f, a->r);
        }
        int i = 0;
        for (const auto& [a, child] : h->children) {
            if (total_regret == 0) sigma.push_back(1.0f/h->children.size());
            else sigma.push_back(max(0.0f, a->r)/total_regret);
            i++;
        }

        if (h->turn() != player) {
            i = 0;
            for (const auto& [a, child] : h->children) {
                a->s += sigma[i]/q;
                i++;
            }
            float p_counter = 0;
            float rand = random_num();
            i = 0;
            for (const auto& [a, child] : h->children) {
                p_counter += sigma[i];
                if (rand < p_counter) {
                    return walk_tree(child, player, q);
                }
                i++;
            }
            cout << "ERROR " << p_counter << endl;
            return walk_tree(h->children[0].second, player, q);
        }

        float total_s = 0;
        vector<float> v;
        for (const auto& [a, child] : h->children) {
            total_s += a->s;
        }
        float expected_v = 0;
        i = 0;
        for (const auto& [a, child] : h->children) {
            float rho = min(1.0f, max(epsilon, (beta_ + tau * a->s)/(beta_ + total_s)));
            v.push_back(0);
            if (random_num() < rho) {
                v[i] = walk_tree(child, player, q*rho);
            }
            expected_v += sigma[i] * v[i];
            i++;
        }
        i = 0;
        for (const auto& [a, child] : h->children) {
            a->r += v[i] - expected_v;
            i++;
        }
        return expected_v;
    }
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
    Node* root = new Node(vector<int>{8,11,11}, a);

    vector<Action*>* actions = new vector<Action*>();
    list_actions(root, actions, new unordered_set<Action*>());

    cout << "total actions: " << actions->size() << endl;

    vector<int> run_resets = {1};
    for (int i=1; i<=20; i++) {
        float reset = 4;
        run_resets.push_back(ceil(reset*run_resets[run_resets.size()-1]));
    }

    for (long long i=0; i<1e11; i++) {
        if (find(run_resets.begin(), run_resets.end(), i) != run_resets.end()) {
            float scale = 3.8 + (-2.3)/(1+pow(2,-0.009*(i-500)));
            for (Action* a : *actions) {
                a->s /= scale;
            }
        }
        walk_tree(root, i%2+1, 1);

        if (i%1000000 == 0) {
            cout << i << " " << walk_tree_counter << endl;
        }


        if (i%10000000 == 0) {
            cout << "Saving at " << i << endl;
            vector<int>* strategy1 = new vector<int>();
            unordered_map<string, string>* strategy_str1 = new unordered_map<string, string>();
            encode_strategy(root, "", 1, strategy1, strategy_str1, new unordered_set<Action*>());
            ofstream fout1("YEEEET1.txt");
            for (auto [history, a_label] : *strategy_str1) {
                fout1 << history << " " << a_label << endl;
            }
            fout1 << endl;

            vector<int>* strategy2 = new vector<int>();
            unordered_map<string, string>* strategy_str2 = new unordered_map<string, string>();
            encode_strategy(root, "", 2, strategy2, strategy_str2, new unordered_set<Action*>());
            ofstream fout2("YEEEET2.txt");
            for (auto [history, a_label] : *strategy_str2) {
                fout2 << history << " " << a_label << endl;
            }
            fout2 << endl;
        }
    }
}
