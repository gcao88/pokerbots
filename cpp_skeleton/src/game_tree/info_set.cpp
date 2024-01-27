#include <vector>
#include "helper_func.cpp"
#include <unordered_map>
#include <array>

struct InfoSet {
    // Key: String (for action), Value: (sample probability sigma, regret r, cumulative sum s)
    unordered_map<string, array<float, 3>> possible_actions;

    /*
        C: Check
        H: Half
        P: Pot
        ^: Raise (3X)
        A: All-In
        .: Fold
    */

    InfoSet() {
        vector<string> all_actions = {"C", "H", "P", "^", "A", "."};
        float initial_prob = 1.0/all_actions.size();
        for (auto action : all_actions) {
            possible_actions[action] = array<float, 3>({initial_prob, 0.0, 0.0});
        }
    }
    void update_sigma(string action, float new_value) {
        possible_actions[action][0] = new_value;
    }
    void update_r(string action, float new_value) {
        possible_actions[action][1] = new_value;
    }
    void update_s(string action, float new_value) {
        possible_actions[action][2] = new_value;
    }
    string sample_action() {
        vector<string> actions;
        vector<float> prob;
        prob.push_back(0);
        for (const auto& pair : possible_actions) {
            prob.push_back(prob.back() + pair.second[0]);
            actions.push_back(pair.first);
        }
        float rand = helper_func::random_real();
        int i = 0;
        while (rand > prob[i]) {
            i++;
        }
        return actions[i-1];
    }
};
