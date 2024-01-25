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
}
