#include <random>
#include <string>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <algorithm>

using namespace std;

namespace helper_func {
    string cards = "23456789TJQKA";
    string suits = "shcd";


    int card_to_num(string card) {
        int num = 0;
        for (int i = 0; i < 13; i++) {
            if (card[0] == cards[i]) {
                num += i;
            }
        }
        for (int i = 0; i < 4; i++) {
            if (suits[i] == card[1]) {
                num += 13 * i;
            }
        }
        return num;
    }

    string num_to_card(int num) {
        return string(1, cards[num % 13]) + string(1, suits[num / 13]);
    }

    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    int random_number(int a, int b) {
        return uniform_int_distribution<int>(a, b)(rng);
    }

    float random_real() {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<double> dis(0.0, 1.0);
        return dis(gen);
    }

    pair<int, vector<int>> EightEval(vector<int> cards) {
        unordered_map<int, int> card_frequencies;
        for (int num : cards) {
            card_frequencies[num]++;
        }
        int arr[5];
        for (const auto& pair : card_frequencies) {
            arr[pair.second]++;
        }
        if (arr[4] >= 1) {
            vector<int> vec;
            for (const auto& pair : card_frequencies) {
                if (pair.second == 4) {
                    vec.push_back(pair.first);
                }
            }
            sort(vec.begin(), vec.end());
            int quad = vec[vec.size()-1];
            int kicker = -1;
            for (int i=cards.size()-1; i>=0; i--) {
                if (cards[i] != quad) {
                    kicker = cards[i];
                    break;
                }
            }
            pair<int, vector<int>> ans = {6, {quad, quad, quad, quad, kicker}};
            return ans;
        }
        if (arr[3] >= 2 || (arr[3] == 1 && arr[2] >= 1)) {
            vector<int> vec;
            for (const auto& pair : card_frequencies) {
                if (pair.second == 3) {
                    vec.push_back(pair.first);
                }
            }
            sort(vec.begin(), vec.end());
            int trip = vec[vec.size()-1];
            vector<int> vec;
            for (const auto& pair : card_frequencies) {
                if (pair.second >= 2 && pair.first != trip) {
                    vec.push_back(pair.first);
                }
            }
            int pairc = vec[vec.size()-1];
            pair<int, vector<int>> ans = {5, {trip, trip, trip, pairc, pairc}};
            return ans;
        }
        sort(cards.begin(), cards.end());
        vector<int> diffs;
        for (int i=1; i<cards.size(); i++) {
            diffs.push_back(cards[i] - cards[i-1]);
        }
        for (int i=diffs.size()-1; i>=3; i--) {
            if (diffs[i] == 1 && diffs[i-1] == 1 && diffs[i-2] == 1 & diffs[i-3] == 1) {
                pair<int, vector<int>> ans = {4, {cards[i+1], cards[i], cards[i-1], cards[i-2], cards[i-3]}};
                return ans;
            }
        }
        if (arr[3] >= 1) {
            int pairc = -1;
            for (const auto& pair : card_frequencies) {
                if (pair.second == 3) {
                    pairc = pair.first;
                }
            }
            vector<int> car = {pairc, pairc, pairc};
            for (int i=cards.size()-1; i>=0; i--) {
                if (cards[i] != pairc) {
                    car.push_back(cards[i]);
                    if (car.size() == 5) {
                        break;
                    }
                }
            }
            pair<int, vector<int>> ans = {3, car};
            return ans;
        }
        if (arr[2] >= 2) {
            vector<int> vec;
            for (const auto& pair : card_frequencies) {
                if (pair.second == 2) {
                    vec.push_back(pair.first);
                }
            }
            sort(vec.begin(), vec.end());
            vector<int> pairs = {vec[vec.size()-1], vec[vec.size()-2]};
            int kicker = -1;
            for (int i=cards.size()-1; i>=0; i--) {
                if (cards[i] != pairs[0] && cards[i] != pairs[1]) {
                    kicker = cards[i];
                    break;
                }
            }
            vector<int> vec = {pairs[0], pairs[0], pairs[1], pairs[1], kicker};
            pair<int, vector<int>> ans = {2, vec};
            return ans;
        }
        if (arr[2] == 1) {
            int pairc = -1;
            for (const auto& pair : card_frequencies) {
                if (pair.second == 2) {
                    pairc = pair.first;
                }
            }
            vector<int> car = {pairc, pairc};
            for (int i=cards.size()-1; i>=0; i--) {
                if (cards[i] != pairc) {
                    car.push_back(cards[i]);
                    if (car.size() == 5) {
                        break;
                    }
                }
            }
            pair<int, vector<int>> ans = {1, car};
            return ans;
        }
        else {
            int last = cards.size();
            pair<int, vector<int>> ans = {0, {cards[last-1], cards[last-2], cards[last-3], cards[last-4], cards[last-5]}};
            return ans;
        }
    }

}
