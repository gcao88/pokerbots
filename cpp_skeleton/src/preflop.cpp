#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

vector<vector<bool>> sb_rfi;

void import_preflop() {
    ifstream inputFile("preflop - btn rfi.csv");
    string line;
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string field;
        sb_rfi.push_back(vector<bool>());

        // Split the line into fields based on comma delimiter
        while (getline(ss, field, ',')) {
            sb_rfi.back().push_back(field == "1");
        }
    }

    inputFile.close();
    return;
}

int main(int argc, char *argv[]) {
    import_preflop();
    for (vector<bool> vec : sb_rfi) {
        for (bool i : vec) {
            cout << i;
        }
        cout << "\n";
    }
    cout << "fuck";
    return 0;
}
