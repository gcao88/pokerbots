#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <random>
#include <algorithm>

using namespace std;

vector<vector<bool>> sb_rfi;
vector<vector<pair<float, float>>> bb_vs_2bet;
vector<vector<pair<float, float>>> sb_vs_3bet;
vector<vector<pair<float, float>>> bb_vs_4bet;

unordered_map<string, int> sb_2bet_2card;
unordered_map<string, int> bb_2bet_2card;
unordered_map<string, int> sb_2bet_3card;
unordered_map<string, int> bb_2bet_3card;
int total_2bet_runs = 0;

unordered_map<string, int> sb_3bet_2card;
unordered_map<string, int> bb_3bet_2card;
unordered_map<string, int> sb_3bet_3card;
unordered_map<string, int> bb_3bet_3card;
int total_3bet_runs = 0;

void import_preflop_floats(string filepath, vector<vector<pair<float, float>>>* vec_ptr) {
    ifstream inputFile(filepath);
    string line;
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string field;
        vec_ptr->push_back(vector<pair<float, float>>());

        while (getline(ss, field, ',')) {
            stringstream sss(field);
            string probability;
            int i = 0;
            pair<float, float> prob_pair;
            while (getline(sss, probability, ';')) {
                if (i == 0) {
                    prob_pair.first = stof(probability);
                }
                else {
                    prob_pair.second = stof(probability);
                }
                i++;
            }
            vec_ptr->back().push_back(prob_pair);
        }
    }
    inputFile.close();
}

void import_preflop() {
    ifstream inputFile("preflop - btn rfi.csv");
    string line;
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string field;
        sb_rfi.push_back(vector<bool>());

        while (getline(ss, field, ',')) {
            sb_rfi.back().push_back(field == "1");
        }
    }
    inputFile.close();

    import_preflop_floats("preflop - bb vs. 2bet.csv", &bb_vs_2bet);
    import_preflop_floats("preflop - btn vs. 3bet.csv", &sb_vs_3bet);
    import_preflop_floats("preflop - bb vs. 4bet.csv", &bb_vs_4bet);

    return;
}

void export_preflop() {
    ofstream outputFile;
    outputFile.open("2bet_sb_2cards.txt");
    for (const auto& pair : sb_2bet_2card) {
        double prob = 1.0*pair.second/total_2bet_runs;
        outputFile << pair.first << ":" << to_string(prob) << endl;
    }
    outputFile.close();

    outputFile.open("2bet_bb_2cards.txt");
    for (const auto& pair : bb_2bet_2card) {
        double prob = 1.0*pair.second/total_2bet_runs;
        outputFile << pair.first << ":" << to_string(prob) << endl;
    }
    outputFile.close();

    outputFile.open("2bet_sb_3cards.txt");
    for (const auto& pair : sb_2bet_3card) {
        double prob = 1.0*pair.second/total_2bet_runs;
        outputFile << pair.first << ":" << to_string(prob) << endl;
    }
    outputFile.close();

    outputFile.open("2bet_bb_3cards.txt");
    for (const auto& pair : bb_2bet_3card) {
        double prob = 1.0*pair.second/total_2bet_runs;
        outputFile << pair.first << ":" << to_string(prob) << endl;
    }
    outputFile.close();

    outputFile.open("3bet_sb_2cards.txt");
    for (const auto& pair : sb_3bet_2card) {
        double prob = 1.0*pair.second/total_3bet_runs;
        outputFile << pair.first << ":" << to_string(prob) << endl;
    }
    outputFile.close();

    outputFile.open("3bet_bb_2cards.txt");
    for (const auto& pair : bb_3bet_2card) {
        double prob = 1.0*pair.second/total_3bet_runs;
        outputFile << pair.first << ":" << to_string(prob) << endl;
    }
    outputFile.close();

    outputFile.open("3bet_sb_3cards.txt");
    for (const auto& pair : sb_3bet_3card) {
        double prob = 1.0*pair.second/total_3bet_runs;
        outputFile << pair.first << ":" << to_string(prob) << endl;
    }
    outputFile.close();

    outputFile.open("3bet_bb_3cards.txt");
    for (const auto& pair : bb_3bet_3card) {
        double prob = 1.0*pair.second/total_3bet_runs;
        outputFile << pair.first << ":" << to_string(prob) << endl;
    }
    outputFile.close();
}

string number_to_card(int i) {
    int number = i % 13;
    string card;
    switch (number) {
        case 0: card += "A"; break;
        case 1: card += "K"; break;
        case 2: card += "Q"; break;
        case 3: card += "J"; break;
        case 4: card += "T"; break;
        default: card += to_string(14-number); break;
    }
    int suit = i/13;
    card += to_string(suit);
    return card;
}

pair<int,int> hand_to_chart_pos(int x, int y) {
    bool suited = (x/13 == y/13);
    int num1 = x%13;
    int num2 = y%13;
    if (suited) {
        pair<int,int> p(min(num1, num2), max(num1, num2));
        return p;
    }
    else {
        pair<int,int> p(max(num1, num2), max(num1, num2));
        return p;
    }
}

string hand_2card_to_string(int x, int y) {
    string card1 = number_to_card(x);
    string card2 = number_to_card(y);
    string num1 = {card1[0]};
    string num2 = {card2[0]};
    if ('0' <= card1[0] && card1[0] <= '9') {
        num1 = to_string(card1[0] - '0');
    }
    if ('0' <= card2[0] && card2[0] <= '9') {
        num2 = to_string(card2[0] - '0');
    }
    string ans;
    if (x%13 < y%13) {
        ans = "" + num1 + num2;
    }
    else {
        ans = "" + num2 + num1;
    }
    bool suited = (card1[1] == card2[1]);
    if (suited) {
        ans += "s";
    }
    else {
        ans += "o";
    }
    return ans;
}

string hand_3card_to_string(int x, int y, int z) {
    vector<string> hand;
    hand.push_back(number_to_card(x));
    hand.push_back(number_to_card(y));
    hand.push_back(number_to_card(z));
    sort(hand.begin(), hand.end());
    return hand[0] + hand[1] + hand[2];
}

int main(int argc, char *argv[]) {
    import_preflop();

    vector<int> deck;
    for (int i=0; i<52; i++) {
        deck.push_back(i);
    }
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dis(0.0, 1.0);
    for (int i=0; i<1e9; i++) {
        if (i % 1000000 == 0) {
            cout << to_string(i) << " done" << endl;
        }
        shuffle(deck.begin(), deck.end(), gen);
        pair<int,int> hand1pos = hand_to_chart_pos(deck[0], deck[1]);
        pair<int,int> hand2pos = hand_to_chart_pos(deck[2], deck[3]);
        string hand1 = hand_2card_to_string(deck[0], deck[1]);
        string hand2 = hand_2card_to_string(deck[2], deck[3]);
        if (sb_rfi[hand1pos.first][hand1pos.second]) {
            // SB raised (2bet)
            float rand = dis(gen);
            float prob_3bet = bb_vs_2bet[hand2pos.first][hand2pos.second].first;
            float prob_call = bb_vs_2bet[hand2pos.first][hand2pos.second].second + prob_3bet;
            if (rand <= prob_3bet) {
                // BB raised (3bet)
                rand = dis(gen);
                float prob_4bet = sb_vs_3bet[hand1pos.first][hand1pos.second].first;
                float prob_call = sb_vs_3bet[hand1pos.first][hand1pos.second].second + prob_4bet;
                if (rand <= prob_4bet) {
                    // SB raised (4bet)
                    continue;
                }
                else if (rand <= prob_call) {
                    // 3 bet pot
                    total_3bet_runs++;
                    sb_3bet_2card[hand1]++;
                    bb_3bet_2card[hand2]++;
                    string hand = hand_3card_to_string(deck[0], deck[1], deck[4]);
                    sb_3bet_3card[hand]++;
                    hand = hand_3card_to_string(deck[2], deck[3], deck[4]);
                    bb_3bet_3card[hand]++;
                }
                else {
                    continue;
                }
            }
            else if (rand <= prob_call) {
                // 2 bet pot
                total_2bet_runs++;
                sb_2bet_2card[hand1]++;
                bb_2bet_2card[hand2]++;
                string hand = hand_3card_to_string(deck[0], deck[1], deck[4]);
                sb_2bet_3card[hand]++;
                hand = hand_3card_to_string(deck[2], deck[3], deck[4]);
                bb_2bet_3card[hand]++;
            }
            else {
                continue;
            }
        }
        else {
            continue;
        }
    }

    export_preflop();

    return 0;
}
