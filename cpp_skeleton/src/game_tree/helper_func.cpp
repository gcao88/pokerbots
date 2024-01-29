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

    int pow13[6] = {1, 13, 169, 2197, 28561, 371293};

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

    int eight_eval(vector<int> cards) {
        sort(cards.begin(), cards.end());
        vector<int> arr[5];
        int cnt = 1;
        for (int i=0; i<cards.size(); i++) {
            if (i != cards.size()-1 && cards[i] == cards[i+1]) {
                cnt++;
            }
            else {
                arr[cnt].push_back(cards[i]);
                cnt = 1;
            }
        }

        if (arr[4].size() >= 1) {
            int quad = arr[4][arr[4].size()-1];
            int kicker = cards[cards.size()-1] == quad ? cards[cards.size()-5] : cards[cards.size()-1];
            return 6*pow13[5] + quad*pow13[4] + quad*pow13[3] + quad*pow13[2] + quad*pow13[1] + kicker;
        }
        if (arr[3].size() >= 2 || (arr[3].size() == 1 && arr[2].size() >= 1)) {
            int trip = arr[3][arr[3].size()-1];
            int pair = arr[3].size() == 2 ? max(arr[3][0], arr[2][arr[2].size()]) : arr[2][arr[2].size()];
            return 5*pow13[5] + trip*pow13[4] + trip*pow13[3] + trip*pow13[2] + pair*pow13[1] + pair;
        }
        for (int i=cards.size()-1; i>=4; i++) {
            if (cards[i] == cards[i-1]+1 && cards[i-1] == cards[i-2]+1 && cards[i-2] == cards[i-3]+1 && cards[i-3] == cards[i-4]+1) {
                return 4*pow13[5] + cards[i]*pow13[4] + cards[i-1]*pow13[3] + cards[i-2]*pow13[2] + cards[i-3]*pow13[1] + cards[i-4];
            }
        }
        // WheeL straight:
        if (cards[cards.size()-1] == 12) {
            int want_to_appear = 0;
            for (int i=0; i<cards.size()-1; i++) {
                if (cards[i] == want_to_appear) {
                    want_to_appear++;
                    if (want_to_appear == 4) {
                        return 4*pow13[5];
                    }
                }
                else if (cards[i] > want_to_appear) {
                    break;
                }
            }
        }
        if (arr[3].size() == 1) {
            int trip = arr[3][0];
            int kicker1 = cards[cards.size()-1];
            int kicker2 = cards[cards.size()-2];
            if (kicker1 == trip) {
                kicker1 = cards[cards.size()-4];
                kicker2 = cards[cards.size()-5];
            }
            else if (kicker2 == trip) {
                kicker2 = cards[cards.size()-5];
            }
            return 3*pow13[5] + trip*pow13[4] + trip*pow13[3] + trip*pow13[2] + kicker1*pow13[1] + kicker2;
        }
        if (arr[2].size() >= 2) {
            int pair1 = arr[2][arr[2].size()-1];
            int pair2 = arr[2][arr[2].size()-2];
            int kicker = cards[cards.size()-1];
            if (kicker == pair1) {
                kicker = cards[cards.size()-3] == pair2 ? cards[cards.size()-5] : cards[cards.size()-3];
            }
            return 2*pow13[5] + pair1*pow13[4] + pair1*pow13[3] + pair2*pow13[2] + pair2*pow13[1] + kicker;
        }
        if (arr[2].size() == 1) {
            int pair = arr[2][0];
            int kicker1 = cards[cards.size()-1];
            int kicker2 = cards[cards.size()-2];
            int kicker3 = cards[cards.size()-3];
            if (kicker1 == pair) {
                kicker1 = cards[cards.size()-3];
                kicker2 = cards[cards.size()-4];
                kicker3 = cards[cards.size()-5];
            }
            else if (kicker2 == pair) {
                kicker2 = cards[cards.size()-4];
                kicker3 = cards[cards.size()-5];
            }
            else if (kicker3 == pair) {
                kicker3 = cards[cards.size()-5];
            }
            return pow13[5] + pair*pow13[4] + pair*pow13[3] + kicker1*pow13[2] + kicker2*pow13[1] + kicker3;
        }
        return cards[cards.size()-1]*pow13[4] + cards[cards.size()-2]*pow13[3] + cards[cards.size()-3]*pow13[2] + cards[cards.size()-4]*pow13[1] + cards[cards.size()-5];
    }
}
