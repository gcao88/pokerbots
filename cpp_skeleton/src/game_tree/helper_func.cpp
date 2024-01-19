#include <string>
#include <iostream>

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

}