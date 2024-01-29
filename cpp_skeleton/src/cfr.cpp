#include <vector>
#include <iostream>
#include <string>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <algorithm>
using namespace std;

struct Node;
struct Action {
    string label;
    float r = 0;
    float s = 0;
    float prob = -1; //for chance node
    Action(const string &label, const float &prob) : label(label), prob(prob) {}
};
struct Node {
    float reward;
    int player; //1/2, or 3 for chance, -1 for N/A
    vector<pair<Action*, Node*>> children;
    Node(const int &reward, const int &player, vector<pair<Action*, Node*>> children) : reward(reward), player(player), children(children) {}
};


void print_tree(Node* u, string prefix = "") {
    if (u->children.empty()) {
        cout << prefix << " : " << u->reward << "\n";
    }
    else {
        for (auto [a, child] : u->children) {
            print_tree(child, prefix + a->label);
        }
    }
}

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
    else if (h->player == 3) {
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
        if (total_regret == 0) sigma[a] = 1.0/h->children.size();
        else sigma[a] = max(0.0f, a->r)/total_regret;
    }

    if (h->player != player) {
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

//store a set of actions; this uniquely defines a strategy
void encode_strategy(Node* h, int player, vector<int>* strategy, unordered_set<Action*>* added) {
    if (h->player != player) {
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
                cout << a->label << " " << i << endl;
                strategy->push_back(i);
                encode_strategy(child, player, strategy, added);
                return;
            }
        }
    }
}

void decode_strategy(Node* h, int i, vector<int>* strategy, int* ind, unordered_set<Action*>* added) {
    if (h->player != i) {
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

void print_s_values(unordered_map<string, vector<Action*>> p1_actions, unordered_map<string, vector<Action*>> p2_actions) {
    for (string card1 : {"A", "B", "C"}) {
        cout << card1 << endl;
        for (Action* a : p1_actions[card1]) {
            cout << a->label << " " << a->s << " ";
        }
        cout << "\n";
    }
    for (string card2 : {"A", "B", "C"}) {
        cout << card2 << "\n";
        for (Action* a : p2_actions[card2]) {
            cout << a->label << " " << a->s << " ";
        }
        cout << "\n";
    }
    cout << "\n";
}

int main() {
    Node* root = new Node(1e9, 3, {});
    unordered_map<string, vector<Action*>> p1_actions = {
        {"A", {new Action("x", -1), new Action("f", -1), new Action("c", -1), new Action("b", -1)}},
        {"B", {new Action("x", -1), new Action("f", -1), new Action("c", -1), new Action("b", -1)}},
        {"C", {new Action("x", -1), new Action("f", -1), new Action("c", -1), new Action("b", -1)}},
    };
    unordered_map<string, vector<Action*>> p2_actions = {
        {"A", {new Action("x", -1), new Action("b", -1), new Action("f", -1), new Action("c", -1)}},
        {"B", {new Action("x", -1), new Action("b", -1), new Action("f", -1), new Action("c", -1)}},
        {"C", {new Action("x", -1), new Action("b", -1), new Action("f", -1), new Action("c", -1)}},
    };

    for (string card1 : {"A", "B", "C"}) {
        for (string card2 : {"A", "B", "C"}) {
            if (card1 == card2) continue;

            Node* u = new Node(1e9, 1, {
                {p1_actions[card1][0], new Node(1e9, 2, {
                    {p2_actions[card2][0], new Node(card1>card2 ? 1 : -1, -1, {})},
                    {p2_actions[card2][1], new Node(1e9, 1, {
                        {p1_actions[card1][1], new Node(-1, -1, {})},
                        {p1_actions[card1][2], new Node(card1>card2 ? 2 : -2, -1, {})},
                    })},
                })},
                {p1_actions[card1][3], new Node(1e9, 2, {
                    {p2_actions[card2][2], new Node(1, -1, {})},
                    {p2_actions[card2][3], new Node(card1>card2 ? 2 : -2, -1, {})},
                })},
            });
            root->children.emplace_back(new Action(card1+card2, 1/6), u);
        }
    }

    vector<int> run_resets = {1};
    for (int i=1; i<=20; i++) {
        float reset = 4;
        run_resets.push_back(ceil(reset*run_resets[run_resets.size()-1]));
    }

    for (int i=0; i<5e3; i++) {
        if (find(run_resets.begin(), run_resets.end(), i) != run_resets.end()) {
            float scale = 3.8 + (-2.3)/(1+pow(2,-0.009*(i-500)));
            for (string card : {"A", "B", "C"}) {
                for (Action* a : p1_actions[card]) {
                    a->s /= scale;
                }
                for (Action* a : p2_actions[card]) {
                    a->s /= scale;
                }
            }
        }
        walk_tree(root, i%2+1, 1);
    }

    print_s_values(p1_actions, p2_actions);

    vector<int>* strategy1 = new vector<int>();
    encode_strategy(root, 1, strategy1, new unordered_set<Action*>());
    int ind1 = 0;
    decode_strategy(root, 1, strategy1, &ind1, new unordered_set<Action*>());

    cout << "player 2" << endl;

    vector<int>* strategy2 = new vector<int>();
    encode_strategy(root, 2, strategy2, new unordered_set<Action*>());
    int ind2 = 0;
    decode_strategy(root, 2, strategy2, &ind2, new unordered_set<Action*>());

    print_s_values(p1_actions, p2_actions);
}
