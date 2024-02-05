#include <iostream>
#include <fstream>
#include <string>
// #include "flop_buckets.h"
// #include "decode.h"
#include <vector>
// #include <ext/pb_ds/assoc_container.hpp>

using namespace std; 

string get_action(unordered_map<string, string> &mp, string history, vector<int> flop, bool ip) {
    auto new_flop = flop; 
    // cout << "HELLO\n";
    string names = "23456789TJQKA"; 
    // auto new_flop = computed_flops[transform_flop.first];
    sort(new_flop.begin(), new_flop.end());
    string filename = "";
    filename += names[new_flop[0]];
    filename += names[new_flop[1]];
    filename += names[new_flop[2]];

    // TODO 
    // string card = "";
    // string a = ""; 
    // string b = "";
    // bool seen = false;
    // for (char c : history) {
    //     if (c >= '0' && c <= '9') {
    //         card += c; 
    //         seen = true; 
    //     } else if (!seen) {
    //         a += c;
    //     } else {
    //         b += c; 
    //     }
    // }

    // string key = "";
    // if (card.size() > 0) {
    //     key = a + to_string(transform_flop.second[stoi(key)].first); 
    // }
    // int numcard = 0; 


    cout << filename << "ip" << history << "\n";
    cout << mp[filename + (ip ? "ip" : "op")] << "\n";
    return mp[filename + (ip ? "ip" : "op") + history]; 
    // + string(names[new_flop[1]]) + string(names[new_flop[2]]); 

}

// int main() {
//     auto mp = get_data(); 
//     cout << get_action(mp, "6 5CH", {4, 8, 9}, true) << "\n";
//     cout << get_action(mp, "11 11CH", {4, 8, 9}, true) << "\n";
//     cout << get_action(mp, "8 7CHC12CH", {4, 8, 9}, true) << "\n";
// }
