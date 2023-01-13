#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <math.h>
#include <stdbool.h>
using namespace std;

// change decimal to binary
int log_2(int n){
    int answer = 0;
    while (n != 1) {
        n = n / 2;
        answer ++;
    }
    return answer;
}

// class Address define
class address{
public:
    vector<long long> v_Data;
    long long tag;
    long long index;
    string ori_data;
    address(string a){
        ori_data = a;
        v_Data.resize(a.length());
        for (int i = 0; i < a.length(); i++) {
            if (a[i] == '1') {
                v_Data[i] = 1;
            }
            else{
                v_Data[i] = 0;
            }
        }
    }
    address(){}

    void get_index(vector<int> S,int n){
        index = 0;
        for (int i = 0; i < n; i++) {   // 0 until index length
            index *= 2;
            index = index + v_Data[S[i]];
        }
    }

    void get_tag(vector<int> S,int n,int x){
        int B[x];
        int index = 0;
        bool k;
        for (int i = 0; i < n + x; i++) {       //index
            k = 0;
            for (int j = 0; j < n; j++) {
                if (S[j] == i) {
                    k = 1;
                }
            }
            if (k == 0) {   //if k = false then tag +
                B[index] = i;
                index ++;
            }
        }

        tag = 0;
        for (int i = 0; i < x; i++) {
            tag *= 2;
            tag += v_Data[B[i]];
        }
    }
};

// class Cache define       //resize and check lru
class Cache{
public:
    vector<vector<long long> > block;
    vector<vector<bool> > LRU;
    int cache_set;
    int associativity;

    bool LRU_update(address b){
        // Check for cache hit
        for (int i = 0; i < associativity; i++) {
            if (block[b.index][i] == b.tag) {
                // Update the corresponding entry in the LRU array to 0
                LRU[b.index][i] = 0;
                // cout << "Cache hit" << endl;
                // cout << "block address index = " << b.index << " in way [" << i << "] address tag =  " << b.tag << endl;
                // cout << "LRU[" << b.index << "][" << i << "] = " << b.tag << endl;

                return false;
            }
        }

        for (int i = 0; i < associativity; i++) {
            if (LRU[b.index][i] == 1) {
                // Update the cache line and the LRU array
                block[b.index][i] = b.tag;
                LRU[b.index][i] = 0;

                // cout << "Cache miss and empty cache line" << endl;
                // cout << "block address index = " << b.index << " in way [" << i << "] address tag =  " <<  block[b.index][i] << endl;
                // cout << "LRU[" << b.index<< "][" << i << "] = " <<LRU[b.index][i] << endl;

                return true;
            }
        }
        
        // Update the LRU array for the remaining cache lines
        for (int i = 0; i < associativity; i++){
            LRU[b.index][i] = 1;
        }

        // Check for cache miss and full cache set
        for (int i = 0; i < associativity; i++) {
            if (LRU[b.index][i] == 1) {
                // Update the cache line and the LRU array
                block[b.index][i] = b.tag;
                LRU[b.index][i] = 0;

                // cout << "Cache2 miss and empty cache line" << endl;
                // cout << "block2 address index = " << b.index << " in way [" << i << "] address tag =  " <<  block[b.index][i] << endl;
                // cout << "LRU[" << b.index<< "][" << i << "] = " <<LRU[b.index][i] << endl;

                return true;
            }
        }
        return 1;
    }

    // cache organization
    Cache(int x, int y){
        cache_set = x;
        associativity = y;
        block.resize(x);    // resize block
        LRU.resize(x);      // resize lru
        for (int i = 0; i < cache_set; i++) {
            block[i].resize(associativity);
            LRU[i].resize(associativity);
            for (int j = 0; j < associativity; j++) {
                block[i][j] = -1;
                LRU[i][j] = 1;
            }
        }
    }
    Cache(){}
};

// global variable
int cache_set, addr_bit, block_size, associativity;
int index_len, tag_len, B_offset;
int n ,m;
int inputsize = 0;

vector <int> V;
vector <bool> miss;
int miss_num;
vector <int> ans_index;
vector <string> input;
vector <address> block;
string name;

void simu(vector<int> C,int n){

    for (int i = 0; i < inputsize; i++) {
        block[i].get_index(C, n); // V[i],index length
        block[i].get_tag(C, n, tag_len);  // V[i],index_ength,tag,len
    }

    Cache cache(cache_set,associativity); //cache organization

    for (int i = 0; i < inputsize; i++) {                   //check if there is cache or not 
        if (cache.LRU_update(block[i]) == true) {
            miss_num++;
            miss[i] = true;
        }
        else{
            miss[i] = false;
        }
    }

    for (int i = 0; i < n; i++){
        ans_index[i] = addr_bit-C[i]-1;
        //cout << "ans_index["<< i << "]= " <<  ans_index[i] << " addr_Bit = " << addr_bit << " c["<< i << "]= " << C[i] << endl;
    }
}

void reading(char** argv){
	string temp;
	fstream fin;
	fin.open(argv[1], ios::in);
	
	for(int i=0; i<4; i++){
		fin >> temp;
		if (temp=="Address_bits:") fin >> addr_bit;
		else if (temp=="Cache_sets:") fin >> cache_set;
        else if (temp=="Block_size:") fin >> block_size;
		else if (temp=="Associativity:") fin >> associativity;
		else cout << "Error" << endl;
	}
	fin.close();
	
    B_offset = log_2(block_size);                   // block size,  2 ^ offset   to get offset of blocksize
    index_len = log_2(cache_set);                   //cache set 
    tag_len = (addr_bit - B_offset - index_len);    // tag length = address bit - offset - index

    freopen(argv[2], "r", stdin);
	
    while(cin >> temp){

        if (temp == ".end" || temp == ".end\n"){
            break;
        }
        else if (temp == ".benchmark"){
            cin >> name;
        }
        else if ((int)temp[0] <= 57 && (int)temp[0] >= 48){
            inputsize++;
            // cout << inputsize << endl;
            input.push_back(temp);
        }
    }
    fclose(stdin);
	

    block.resize(inputsize);
    for (int i = 0; i < inputsize; i++) {
        block[i] = address(input[i]);              // insert bit into address
    }

    n = addr_bit - B_offset;
    m = index_len;

    V.resize(n);
    miss.resize(inputsize);
    ans_index.resize(m);
}

void output(char** argv){
    freopen(argv[3], "w", stdout);

    cout << "Address bits: " << addr_bit << endl;
    cout << "Number of sets: " << cache_set << endl;
    cout << "Associativity: " << associativity << endl;
    cout << "Block size: " << block_size << endl;
    cout << endl;
    cout << "Indexing bit count: " << index_len << endl;
    cout << "Indexing bits:";
    for (int i = 0; i < index_len; i++) {
        cout << " " << ans_index[i];
    }
    cout << endl;

    cout << "Offset bit count: " << B_offset << endl;
    cout << endl;

    cout << name << endl;
    for (int i = 0; i < inputsize; i++) {
        cout << input[i];
        if (miss[i] == 1) {
            cout << " miss" << endl;
        }
        else{
            cout << " hit" << endl;
        }
    }
    cout << ".end" << endl << endl;
    cout <<"Total cache miss count: " << miss_num << endl;

    fclose(stdout);
}

int main(int argc, char **argv) {
    reading(argv);

    for (int i = 0; i < index_len; i++) {    //tag + index
        V[i] = tag_len + i;
        //cout << "V[i] = " << V[i] << endl;
    }
    simu(V, m);

    output(argv);
    return 0;
}

//  ssh u109006201@nthucad.cs.nthu.edu.tw
//  arch_final_lsb
// ./arch_final_lsb  ../testcases/config/cache1.org ../testcases/bench/reference1.lst ../output/index.rpt