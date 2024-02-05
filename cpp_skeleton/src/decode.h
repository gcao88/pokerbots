#include <iostream>
#include <fstream> 
#include <unordered_map> 

// #include <ext/pb_ds/assoc_container.hpp>

using namespace std; 




unordered_map<string, string> get_data_69() {
    unordered_map<string, string> mp;

    string files[] = {
"src/cfr_data/strategy1_4JK_OP.txt",
"src/cfr_data/strategy1_4QA_IP.txt",
"src/cfr_data/strategy1_5JJ_OP.txt",
"src/cfr_data/strategy1_6TJ_IP.txt",
"src/cfr_data/strategy1_8QA_OP.txt",
"src/cfr_data/strategy1_9JA_OP.txt",
"src/cfr_data/strategy1_24J_OP.txt",
"src/cfr_data/strategy1_24Q_OP.txt",
"src/cfr_data/strategy1_38T_IP.txt",
"src/cfr_data/strategy1_45K_IP.txt",
"src/cfr_data/strategy1_45T_OP.txt",
"src/cfr_data/strategy1_55Q_IP.txt",
"src/cfr_data/strategy1_56A_IP.txt",
"src/cfr_data/strategy1_56K_OP.txt",
"src/cfr_data/strategy1_57A_OP.txt",
"src/cfr_data/strategy1_78K_IP.txt",
"src/cfr_data/strategy1_78Q_OP.txt",
"src/cfr_data/strategy1_79A_OP.txt",
"src/cfr_data/strategy1_257_OP.txt",
"src/cfr_data/strategy1_378_IP.txt",
"src/cfr_data/strategy1_479_OP.txt",
"src/cfr_data/strategy1_999_IP.txt",
"src/cfr_data/strategy1_JAA_OP.txt",
"src/cfr_data/strategy1_KK7_IP.txt",
"src/cfr_data/strategy1_Q75_IP.txt",
"src/cfr_data/strategy1_QT8_IP.txt",
"src/cfr_data/strategy1_TJA_IP.txt",
"src/cfr_data/strategy2_4JK_OP.txt",
"src/cfr_data/strategy2_4QA_IP.txt",
"src/cfr_data/strategy2_5JJ_OP.txt",
"src/cfr_data/strategy2_6TJ_IP.txt",
"src/cfr_data/strategy2_8QA_OP.txt",
"src/cfr_data/strategy2_9JA_OP.txt",
"src/cfr_data/strategy2_24J_OOP.txt",
"src/cfr_data/strategy2_24Q_OP.txt",
"src/cfr_data/strategy2_38T_IP.txt",
"src/cfr_data/strategy2_45K_IP.txt",
"src/cfr_data/strategy2_45T_OP.txt",
"src/cfr_data/strategy2_55Q_IP.txt",
"src/cfr_data/strategy2_56A_IP.txt",
"src/cfr_data/strategy2_56K_OP.txt",
"src/cfr_data/strategy2_57A_OP.txt",
"src/cfr_data/strategy2_78K_IP.txt",
"src/cfr_data/strategy2_78Q_OP.txt",
"src/cfr_data/strategy2_79A_OP.txt",
"src/cfr_data/strategy2_257_OP.txt",
"src/cfr_data/strategy2_378_IP.txt",
"src/cfr_data/strategy2_479_OP.txt",
"src/cfr_data/strategy2_999_IP.txt",
"src/cfr_data/strategy2_JAA_OP.txt",
"src/cfr_data/strategy2_KK7_IP.txt",
"src/cfr_data/strategy2_Q75_IP.txt",
"src/cfr_data/strategy2_QT8_IP.txt",
"src/cfr_data/strategy2_TJA_IP.txt"
    };
    // ofstream fout("FUCK.txt");
    // fout << "FUCK" << endl;

    for (string file : files) {
        cout << file << "\n";
        ifstream fin(file);
        string key1 = file.substr(23, 3) + file.substr(27, 2); 
        string curline = "";
        while (getline(fin, curline)) {
            // cout << curline << endl;
            if (curline.size() > 2) { // should not be empty
                string action;
                for (int i = curline.size() - 1; i >= 0; i--) {
                    if (curline[i] != ' ') {
                        action = curline.substr(i, 1);
                        curline = curline.substr(0, i); 
                        break;
                    }
                }
                if ((file[14] == 'i' && file[8] == '1') || (file[14] == 'o' && file[8] == '2')) { // p1 has 2 cards p2 has 3
                    bool happened = false;
                    for (int i = 0; i < curline.size(); i++) {
                        if (curline[i] != ' ' && (curline[i] < '0' || curline[i] > '9')) {
                            curline = curline.substr(0, i - 5) + curline.substr(i);
                            happened = true;
                            break;
                        }
                    }
                    if (!happened) {
                        curline = curline.substr(0, curline.size() - 6) + " ";
                    }
                    curline = curline.substr(0, curline.size() - 1);
                    // cout << key1 + curline << " " << action<< "\n";
                } else {
                    curline = curline.substr(3); 
                    curline = curline.substr(0, curline.size() - 1);
                }
                mp[key1 + curline] = action; 
            }
        }
    }
    return mp; 
} 
// int main() {
//     get_data(); 
// }
