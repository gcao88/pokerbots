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
            int pair;
            if (arr[3].size() >= 2) {
                pair = arr[3][0];
                if (arr[2].size() >= 1) {
                    pair = max(pair, arr[2][arr[2].size()-1]);
                }
            }
            else {
                pair = arr[2][arr[2].size()-1];
            }
            return 5*pow13[5] + trip*pow13[4] + trip*pow13[3] + trip*pow13[2] + pair*pow13[1] + pair;
        }

        for (int i=cards.size()-1; i>=4; i--) {
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

    int eight_eval_suit(const vector<int>& cards_suits) {
        vector<int> suits;
        for (int i=0; i<cards_suits.size(); i++) {
            suits.push_back(cards_suits[i]/13);
        }
        sort(suits.begin(), suits.end());
        vector<int> suit_arr[9];
        int cnt = 1;
        for (int i=0; i<suits.size(); i++) {
            if (i != suits.size()-1 && suits[i] == suits[i+1]) {
                cnt++;
            }
            else {
                suit_arr[cnt].push_back(suits[i]);
                cnt = 1;
            }
        }
        bool isFlush = false;
        if (suit_arr[5].size() >= 1 || suit_arr[6].size() >= 1 || suit_arr[7].size() >= 1 || suit_arr[8].size() >= 1) {
            isFlush = true;
        }
        vector<int> cards;
        for (int i=0; i<cards_suits.size(); i++) {
            cards.push_back(cards_suits[i]%13);
        }
        sort(cards.begin(), cards.end());
        vector<int> arr[5];
        cnt = 1;
        for (int i=0; i<cards.size(); i++) {
            if (i != cards.size()-1 && cards[i] == cards[i+1]) {
                cnt++;
            }
            else {
                arr[cnt].push_back(cards[i]);
                cnt = 1;
            }
        }
        vector<int> isFlushVec;
        if (isFlush) {
            int flush_suit = suits[3];
            for (int i=0; i<cards_suits.size(); i++) {
                if (cards_suits[i]/13 == flush_suit) {
                    isFlushVec.push_back(cards_suits[i]%13);
                }
            }
            sort(isFlushVec.begin(), isFlushVec.end());
            for (int i=isFlushVec.size()-1; i>=4; i--) {
                if (isFlushVec[i]==isFlushVec[i-1]+1 && isFlushVec[i-1]==isFlushVec[i-2]+1 && isFlushVec[i-2]==isFlushVec[i-3]+1 && isFlushVec[i-3]==isFlushVec[i-4]+1) {
                    return 8*pow13[5] + isFlushVec[i]*pow13[4] + isFlushVec[i-1]*pow13[3] + isFlushVec[i-2]*pow13[2] + isFlushVec[i-3]*pow13[1] + isFlushVec[i-4];
                }
            }
        }
        if (arr[4].size() >= 1) {
            int quad = arr[4][arr[4].size()-1];
            int kicker = cards[cards.size()-1] == quad ? cards[cards.size()-5] : cards[cards.size()-1];
            return 7*pow13[5] + quad*pow13[4] + quad*pow13[3] + quad*pow13[2] + quad*pow13[1] + kicker;
        }
        if (arr[3].size() >= 2 || (arr[3].size() == 1 && arr[2].size() >= 1)) {
            int trip = arr[3][arr[3].size()-1];
            int pair;
            if (arr[3].size() >= 2) {
                pair = arr[3][0];
                if (arr[2].size() >= 1) {
                    pair = max(pair, arr[2][arr[2].size()-1]);
                }
            }
            else {
                pair = arr[2][arr[2].size()-1];
            }
            return 6*pow13[5] + trip*pow13[4] + trip*pow13[3] + trip*pow13[2] + pair*pow13[1] + pair;
        }
        if (isFlush) {
            int x = isFlushVec.size();
            return 5*pow13[5] + isFlushVec[x-1]*pow13[4] + isFlushVec[x-2]*pow13[3] + isFlushVec[x-3]*pow13[2] + isFlushVec[x-4]*pow13[1] + isFlushVec[x-5];
        }
        vector<int> cardsNoDup;
        cardsNoDup.push_back(cards[0]);
        for (int i=1; i<cards.size(); i++) {
            if (cards[i] != cardsNoDup[cardsNoDup.size()-1]) {
                cardsNoDup.push_back(cards[i]);
            }
        }
        for (int i=cardsNoDup.size()-1; i>=4; i--) {
            if (cardsNoDup[i] == cardsNoDup[i-1]+1 && cardsNoDup[i-1] == cardsNoDup[i-2]+1 && cardsNoDup[i-2] == cardsNoDup[i-3]+1 && cardsNoDup[i-3] == cardsNoDup[i-4]+1) {
                return 4*pow13[5] + cardsNoDup[i]*pow13[4] + cardsNoDup[i-1]*pow13[3] + cardsNoDup[i-2]*pow13[2] + cardsNoDup[i-3]*pow13[1] + cardsNoDup[i-4];
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

    //5 card board, 2 or 3 card hand (this will detect which care it is and make the opponent the other)
    float get_equity(const vector<int>& board, const vector<int>& hand) {
        if (board.size() != 5) cout << "ERROR: board size is not 5 in get_equity" << endl;
        if (hand.size() != 2 && hand.size() != 3) cout << "ERROR: hand size is not 2 or 3 in get_equity" << endl;

        vector<int> deck;
        for (int i=0; i<52; i++) {
            if (find(board.begin(), board.end(), i) != board.end() ||
                find(hand.begin(), hand.end(), i) != hand.end()) {
                    continue;
            }
            deck.push_back(i);
        }
        vector<int> hand1;
        for (int x : board) {
            hand1.push_back(x);
        }
        for (int x : hand) {
            hand1.push_back(x);
        }
        int hand1_strength = eight_eval_suit(hand1);

        vector<int> hand2;
        for (int x : board) {
            hand2.push_back(x);
        }
        float win;
        float total;
        for (int i = 0; i < 500; i++) {
            int a = random_number(0, deck.size()-1);
            hand2.push_back(deck[a]);
            int b = random_number(0, deck.size()-1);
            while (a == b) {
                b = random_number(0, deck.size()-1);
            }
            hand2.push_back(deck[b]);
            if (hand.size() == 2) {
                int c = random_number(0, deck.size()-1);
                while (c == a || c == b) {
                    c = random_number(0, deck.size()-1);
                }
                hand2.push_back(deck[c]);
            }

            int hand2_strength = eight_eval_suit(hand2);
            if (hand1_strength > hand2_strength) win++;
            if (hand1_strength == hand2_strength) win += 0.5;
            total++;

            if (hand.size() == 2) {
                hand2.pop_back();
                hand2.pop_back();
                hand2.pop_back();
            }
            if (hand.size() == 3) {
                hand2.pop_back();
                hand2.pop_back();
            }
        }
        
        return win/total;
    }
}

// int main() {
//     int c1 = 13*0 + 1;
//     int c2 = 13*0 + 2;
//     int c3 = 13*0 + 3;
//     int c4 = 13*0 + 4;
//     int c5 = 13*0 + 5;
//     int c6 = 13*1 + 1;
//     int c7 = 13*1 + 2;
//     int c8 = 13*1 + 3;
//     int x = helper_func::eight_eval_suit({c1,c2,c3,c4,c5,c6,c7,c8});
//     while (x > 0) {
//         cout << x%13 << " ";
//         x /= 13;
//     }
// }


//testing equity_vs_3
int mc(int rank, int suit) {
    return rank + 13*suit;
}

int main() {
    cout << helper_func::get_equity(vector<int>{mc(3,0), mc(3,1), mc(9,2), mc(5,3), mc(1,0)}, vector<int>{mc(5,1), mc(4,2), mc(0,3)}) << endl;

    /*
    int a = helper_func::eight_eval_suit(vector<int>{mc(3,0), mc(3,1), mc(9,2), mc(5,3), mc(1,3), mc(2,2), mc(4,3)});
    while (a > 0) {
        cout << a%13 << " ";
        a /= 13;
    }
    cout << endl;

    cout << helper_func::eight_eval_suit(vector<int>{mc(3,0), mc(3,1), mc(9,2), mc(5,3), mc(1,3), mc(2,2), mc(4,3)}) << endl;
    cout << helper_func::eight_eval_suit(vector<int>{mc(3,0), mc(3,1), mc(9,2), mc(5,3), mc(1,3), mc(2,2), mc(8,3)}) << endl;
    */

    /*
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 2000; i++) {
        helper_func::get_equity(vector<int>{mc(3,0), mc(3,1), mc(9,3), mc(0,3), mc(1,3)}, vector<int>{mc(3,2), mc(9,3)});
        //helper_func::eight_eval_suit(vector<int>{mc(3,0), mc(3,1), mc(9,2), mc(0,3), mc(0,0), mc(3,2), mc(9,3)});
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Time taken: " << elapsed.count() << " ms\n";
    */
}
