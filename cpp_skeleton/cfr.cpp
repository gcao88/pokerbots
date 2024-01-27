#include <vector>
#include <iostream>
#include <string>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <fstream>

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
    bool is_terminal;
    float reward;
    int player; //1/2, or 3 for chance, -1 for N/A
    unordered_map<Action*, Node*> children;
    Node(const bool &is_terminal, const int &reward, const int &player, unordered_map<Action*, Node*> children) : is_terminal(is_terminal), reward(reward), player(player), children(children) {}
    Node* next(Action* a) {
        for (auto child : children) {
            if (child.first == a) {
                return child.second;
            }
        }
    }
};


void print_tree(Node* u, string prefix = "") { 
    if (u->is_terminal) {
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
float beta = 1e6;
float tau = 1000;
float walk_tree(Node* h, int i, float q) {
    if (h->is_terminal) {
        if (i == 1) return h->reward/q;
        else return -h->reward/q;
    }
    else if (h->player == 3) {
        float p_counter = 0;
        float rand = random_num();
        for (auto [a, child] : h->children) {
            p_counter += a->prob;
            if (rand < p_counter) {
                return walk_tree(child, i, q);
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

    if (h->player != i) {
        for (auto [a, child] : h->children) {
            a->s += sigma[a]/q;
        }
        float p_counter = 0;
        float rand = random_num();
        for (auto [a, child] : h->children) {
            p_counter += sigma[a];
            if (rand < p_counter) {
                return walk_tree(child, i, q); 
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
        float rho = min(1.0f, max(epsilon, (beta + tau * a->s)/(beta + total_s)));
        v[a] = 0;
        if (random_num() < rho) {
            v[a] = walk_tree(h->next(a), i, q*rho);
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
void encode_strategy(Node* h, int i, unordered_set<Action*>* strategy) {
    if (h->player != i) {
        for (auto [a, child] : h->children) {
            encode_strategy(child, i, strategy);
        }
    }
    else {
        //to account for 2 nodes in same infoset, if an action is in strategy, then skip
        for (auto [a, child] : h->children) {
            if (strategy->contains(a)) return;
        }

        float total_s = 0;
        for (auto [a, child] : h->children) {
            total_s += a->s;
        }
        float p_counter = 0;
        float rand = random_num();
        for (auto [a, child] : h->children) {
            p_counter += a->s / total_s;
            if (rand < p_counter) {
                strategy->insert(a);
                encode_strategy(child, i, strategy);
                return;
            }
        }
    }
}

void decode_strategy(Node* h) {

}

int main() {
    Node* root = new Node(false, 1e9, 3, {});
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

            Node* u = new Node(false, 1e9, 1, {
                {p1_actions[card1][0], new Node(false, 1e9, 2, {
                    {p2_actions[card2][0], new Node(true, card1>card2 ? 1 : -1, -1, {})},
                    {p2_actions[card2][1], new Node(false, 1e9, 1, {
                        {p1_actions[card1][1], new Node(true, -1, -1, {})},
                        {p1_actions[card1][2], new Node(true, card1>card2 ? 2 : -2, -1, {})},
                    })},
                })},
                {p1_actions[card1][3], new Node(false, 1e9, 2, {
                    {p2_actions[card2][2], new Node(true, 1, -1, {})},
                    {p2_actions[card2][3], new Node(true, card1>card2 ? 2 : -2, -1, {})},
                })},
            });
            root->children[new Action(card1+card2, 1/6)] = u;
        }
    }
  
    print_tree(root);
    ofstream fout("data.txt");
    for(int j=0; j<1; j++) {
        for (int i=0; i<6561; i++) {
            walk_tree(root, i%2+1, 1);
        }

        // cout << "P1" << endl;
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
}