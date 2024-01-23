#include "tree_node.cpp"
#include <unordered_set>
#include <vector>
#include "helper_func.cpp"

struct InfoSet {
    // Key: String (for action), Value: (sample probability sigma, regret r, cumulative sum s)
    unordered_map<string, array<int, 3>> possible_actions;

    InfoSet() {
        
    }
    void update_sigma(action, new_value) {
        possible_actions.get(action)[0] = new_value;
    }
    void update_r(action, new_value) {
        possible_actions.get(action)[1] = new_value;
    }
    void update_s(action, new_value) {
        possible_actions.get(action)[2] = new_value;
    }
    string sample_action() {
        vector<string> actions;
        vector<float> prob;
        prob.push_back(0);
        for (const auto& pair : myMap) {
            std::cout << pair.first << ": " << pair.second << std::endl;
            prob.push_back(prob.back() + pair.second[0])
            actions.push_back(pair.first)
        }
        float rand = random_real();
        i = 0
        while rand > prob[i]{
            i++;
        }
        return actions[i-1];
    }
};
