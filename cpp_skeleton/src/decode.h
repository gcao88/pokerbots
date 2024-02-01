#include <iostream>
#include <fstream> 
#include <unordered_map> 

// #include <ext/pb_ds/assoc_container.hpp>

using namespace std; 




namespace data {
unordered_map<string, string> get_data() {
    unordered_map<string, string> mp;

    string files[] = {
        "strategy1_6TJ_ip.txt", 
        "strategy1_38T_ip.txt" 
        "strategy2_6TJ_ip.txt", 
        "strategy2_38T_ip.txt" 
    };
    // ofstream fout("FUCK.txt");
    // fout << "FUCK" << endl;

    for (string file : files) {
        ifstream fin("src/cfr_data/" + file); 
        string key1 = file.substr(10, 3) + file.substr(14, 2); 
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
};
// int main() {
//     get_data(); 
// }
