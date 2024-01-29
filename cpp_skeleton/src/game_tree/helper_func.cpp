#include <random>
#include <string>
#include <iostream>
#include <chrono>

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
            return "QUADS";
        }
        if (arr[3] >= 2 || (arr[3] == 1 && arr[2] >= 1)) {

            pair<int, vector<int>> ans = {5, };
            return ans;
        }
        sort(cards.begin(), cards.end());
        vector<int> diffs;
        for (int i=1; i<cards.size(); i++) {
            diffs.push_back(cards[i] - cards[i-1]);
        }
        for (int i=diffs.size()-1; i>=3; i--) {
            if (diffs[i] == 1 && diffs[i-1] == 1 && diffs[i-2] == 1 & diffs[i-3] == 1) {
                pair<int, vector<int>> ans = {4, {cards[i]}};
                return ans;
            }
        }
        if (arr[3] >= 1) {
            int trip = -1;
            for (const auto& pair : card_frequencies) {
                if (pair.second == 3) {
                    trip = pair.first;
                }
            }
            pair<int, vector<int>> ans = {3, {trip}};
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
            pair<int, vector<int>> ans = {2, {vec[vec.size()-1], vec[vec.size()-1]}};
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
