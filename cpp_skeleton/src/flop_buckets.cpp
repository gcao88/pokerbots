#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

vector<vector<int>> computed_flops = {
    {8, 11, 11},
    {5, 6, 11},
    {3, 6, 7},
    {7, 10, 12},
    {8, 8, 11},
    {2, 3, 9},
    {0, 4, 7},
    {10, 10, 12},
    {0, 2, 4},
    {0, 4, 6},
    {1, 6, 8},
    {3, 4, 12},
    {11, 12, 12},
    {0, 3, 5},
    {6, 9, 10},
    {1, 7, 11},
    {1, 5, 12},
    {2, 3, 12},
    {4, 7, 9},
    {0, 8, 10},
    {2, 5, 7},
    {1, 2, 6},
    {3, 5, 10},
    {4, 8, 9},
    {1, 2, 9}
};
vector<vector<int>> computed_diffs;

int zeroDif = 16;
int oneDif = 3;

void compute_diffs() {
    for (int i=0; i<computed_flops.size(); i++) {
        sort(computed_flops[i].begin(), computed_flops[i].end());
        vector<int> diffs = {computed_flops[i][0]-(-1), computed_flops[i][1]-computed_flops[i][0], computed_flops[i][2]-computed_flops[i][1], computed_flops[i][0]+computed_flops[i][1]+computed_flops[i][2]};
        if (diffs[1] == 0) {
            diffs[1] -= zeroDif;
        }
        if (diffs[2] == 0) {
            diffs[2] -= zeroDif;
        }
        if (diffs[1] == 1) {
            diffs[1] -= zeroDif;
        }
        if (diffs[2] == 1) {
            diffs[2] -= zeroDif;
        }
        computed_diffs.push_back(diffs);
    }
}

pair<int, vector<pair<int, int>>> flopbuckets(vector<int> flop) {
    sort(flop.begin(), flop.end());
    vector<int> flop_diffs = {flop[0]-(-1), flop[1]-flop[0], flop[2]-flop[1], flop[0]+flop[1]+flop[2]};
    if (flop_diffs[1] == 0) {
        flop_diffs[1] -= zeroDif;
    }
    if (flop_diffs[2] == 0) {
        flop_diffs[2] -= zeroDif;
    }
    if (flop_diffs[1] == 1) {
        flop_diffs[1] -= oneDif;
    }
    if (flop_diffs[2] == 1) {
        flop_diffs[2] -= oneDif;
    }
    int closest_dist = INT_MAX;
    int best_flop_index = -1;
    for (int i=0; i<computed_diffs.size(); i++) {
        int dist = (flop_diffs[0]-computed_diffs[i][0])*(flop_diffs[0]-computed_diffs[i][0]);
        dist += (flop_diffs[1]-computed_diffs[i][1])*(flop_diffs[1]-computed_diffs[i][1]);
        dist += (flop_diffs[2]-computed_diffs[i][2])*(flop_diffs[2]-computed_diffs[i][2]);
        dist += 0.5*(flop_diffs[3]-computed_diffs[i][3])*(flop_diffs[3]-computed_diffs[i][3]);
        if (dist < closest_dist) {
            closest_dist = dist;
            best_flop_index = i;
        }
    }
    vector<pair<int, int>> swap_vec;
    if (find(flop_diffs.begin(), flop_diffs.end(), 0) == flop_diffs.end()) {
        // No pairs
        swap_vec.push_back({flop[0], computed_flops[best_flop_index][0]});
        swap_vec.push_back({flop[1], computed_flops[best_flop_index][1]});
        swap_vec.push_back({flop[2], computed_flops[best_flop_index][2]});
    }
    else if (flop_diffs[1] == 0 && flop_diffs[2] == 0) {
        // trips flop
        vector<pair<int,int>> vec;
        pair<int, vector<pair<int,int>>> p(-1, vec);
        return p;
    }
    else {
        // pair flop
        swap_vec.push_back({flop[1], computed_flops[best_flop_index][1]});
        int nonPair = (computed_flops[best_flop_index][0] == computed_flops[best_flop_index][1]) ? computed_flops[best_flop_index][2] : computed_flops[best_flop_index][0];
        int nonPairFlop = (flop[0] == flop[1]) ? flop[2] : flop[0];
        swap_vec.push_back({nonPairFlop, nonPair});
    }
    pair<int, vector<pair<int, int>>> ans = {best_flop_index, swap_vec};
    return ans;
}

int main() {
    compute_diffs();
    vector<int> flop = {8, 9, 10};
    sort(flop.begin(), flop.end());

    pair<int, vector<pair<int, int>>> ans = flopbuckets(flop);

    vector<int> best_flop = computed_flops[ans.first];
    vector<pair<int, int>> swap_vec = ans.second;

    for (int i=0; i<3; i++) {
        cout << flop[i] << " ";
    }
    cout << "is most similar to ";
    for (int i=0; i<3; i++) {
        cout << best_flop[i] << " ";
    }
    cout << endl;
    cout << "Swap the following:" << endl;
    for (pair<int, int> p : swap_vec) {
        cout << p.first << " to " << p.second << endl;
    }
}
