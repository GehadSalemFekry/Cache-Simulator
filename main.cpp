#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <math.h>
using namespace std;

typedef double db;

int s, l; // s is the total cache size, l is the cache line size 
int m; // m is the number of lines per set (associativity level)
int hits = 0, misses = 0, accesses = 0, sets, d, i;
int cache_cycles;
char cache_org;
const int miss_penalty = 100;
vector<vector<pair<bool, int>>> cache[2]; // 0: instruction, 1: data 
ifstream fin;
ofstream fout;

bool is_power_two(int n) {
    return (n & (n - 1)) == 0;
}

string to_binary(int n) {
    string ans = "";
    if (n == 0) return "0";
    while (n) {
        ans += (n % 2 + '0');
        n /= 2;
    }
    reverse(ans.begin(), ans.end());
    return ans;
}

bool is_valid_cache_line_size(int cache_size, int line_size) {
    if (cache_size <= 0 || line_size <= 0) 
        cout << "The sizes can't be less than 1, please try again!\n";
    else if (!is_power_two(cache_size) || !is_power_two(line_size))
        cout << "The sizes have to be powers of 2, please try again!\n";
    else if (cache_size % line_size) 
        cout << "The line size has to divide the cache size, please try again!\n";
    else return true;

    return false;
}

bool is_valid_associativity(int associativity) {
    if (associativity <= 0) 
        cout << "The associativity level can't be less than 1, please try again!\n";
    else if (!is_power_two(associativity))
        cout << "The associativity level  have to be powers of 2, please try again!\n";
    else if (associativity > s / l) 
        cout << "The associtivity level has to be less than or equal number of lines in the cache, please try again!\n";
    else return true;

    return false;
}

void read_input() {
    cout << "Please, enter the cache total size in bytes, and the cache line size in bytes: ";
    cin >> s >> l;
    while (!is_valid_cache_line_size(s, l)) 
        cin >> s >> l;

    cout << "Please choose a cache organization:\n";
    cout << "D: direct mapping\n" << 
            "F: full associativity\n" << 
            "S: set associativity\n";

    cin >> cache_org;
    while (toupper(cache_org) != 'D' && toupper(cache_org) != 'F' && toupper(cache_org) != 'S') {
        cout << "This is an invalid cache type, please try again!\n";
        cin >> cache_org;
    }
    cache_org = toupper(cache_org);

    if (cache_org == 'D') m = 1;
    else if (cache_org == 'F') m = s / l;
    else {
        cout<<"Please, enter the associativity level.\nIt must be a power of two less than or equal the number of lines\n";
        cin >> m;
        while (!is_valid_associativity(m))
            cin >> m;
    }

    // Setting variables
    sets = (s / l) / m;
    d = log2(l), i = log2(sets);

    cout << "Enter the number of cycles needed to access the cache (an integer between 1 and 10  clock cycles): ";
    cin >> cache_cycles;
    while (cache_cycles < 1 || cache_cycles > 10) {
        cout << "The cycles have to be between 1 and 10, please try again!\n";
        cin >> cache_cycles;
    } 

    cout << "Please enter the file containing the memory addresses (in bytes) you want to access using this simulator.\n";
    string file; cin >> file;
    fin.open(file);

    while (fin.fail()) {
        cout << "The file is not found, please try again!\n";
        cin >> file;
        fin.open(file);
    }

    cout << "Please enter the file you want to have the output in.\n";
    cin >> file;
    fout.open(file);
}

void print_output(bool type, int address) {
    if (type) fout << "Data Access with Address " << address << "\n";
    else fout << "Instruction Acess with Address " << address << "\n";

    for (int i = 0; i < 2; i++) {
        if (i == 0) 
            fout << "---------------------- Instructions Cache Information: -------------------------\n";
        else 
            fout << "------------------------- Data Cache Information: ------------------------------\n";
        fout << "Valid Bit" << "\t" << "Tag" << "\t\t" << "Index" << "\n";
        for (int j = 0; j < sets; j++) 
            for (int k = 0; k < m; k++)
                fout << cache[i][j][k].first << "\t\t\t" << to_binary(cache[i][j][k].second)  << "\t\t" << j << "\n";
    }

    fout << "--------------------------------------------------------------------------------\n";
    fout << "Total Accesses are: " << accesses << "\n";
    fout << "The Hit ratio is: " << (db)hits / accesses << "\n";
    fout << "The Miss Ratio is: " << (db)misses / accesses << "\n";
    fout << "The Average Memory Access Time (AMAT) of the memory hierarchy (in cycles) is: " << cache_cycles + ((db)misses / accesses) * miss_penalty << "\n";
    fout << "--------------------------------------------------------------------------------\n";
}

void simulator() {
    cache[0] = cache[1] = vector<vector<pair<bool, int>>> (sets, vector<pair<bool, int>>(m));
    bool is_data;

    char type; int address;
    while (fin >> type >> address) {
        accesses++;
        if (type == 'd')  is_data = 1;
        else is_data = 0;

        int index = (address >> d) % sets, tag = (address >> d) >> i;

        int cur_pos;
        for (cur_pos = 0; cur_pos < m; cur_pos++) {
            if(cache[is_data][index][cur_pos].first && cache[is_data][index][cur_pos].second == tag) {
                hits++; break;
            } else if (cache[is_data][index][cur_pos].first == 0) break; // Empty place
        }

        if (cur_pos == m) { // No empty place
            misses++;
            int rand_pos = rand() % m; // Random Replacement Policy
            cache[is_data][index][rand_pos] = {1, tag};
        }
        else if (!cache[is_data][index][cur_pos].first) { 
            misses++;
            cache[is_data][index][cur_pos] = {1, tag};
        }

        print_output(type, address);
    }
}

int main() {
    srand(time(NULL));

    read_input();
    simulator();

    return 0;
}