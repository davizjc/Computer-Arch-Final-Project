#include <iostream>
#include <algorithm>
#include <climits>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int log2(int n){
    int answer = 0;
    while (n != 1) {
        n = n / 2;
        answer ++;
    }
    return answer;
}

int toDecimal(string x) {
    // return decimal(x)
    int y = 0;
    int pow2 = 1;
    for (int i = x.size() - 1; i >= 0; i--) {
        if (x[i] == '1') y += pow2;
        pow2 <<= 1;  // pow2 *= 2
    }
    return y;
}

//global variable
int addr_bits,block_size,cache_sets,associativity;
int B_offset,index_length;
int miss_count;
bool cache_hit = false;

class Tag { 
   public:
    string tag_addr;
    int last;
    Tag() {
        tag_addr = "";
        last = INT_MAX;
    }
};

vector<vector<Tag>> sets;   // allocation , sets, way, tags
vector<string> inputs;      // inputaddress
vector<int> outputs;        // output cache
vector<int> indexing_bits;  // {3, 2}
string testcase;  


void readCache(string path) {   
    string temp;
    ifstream fin(path);

    for(int i=0; i<4; i++){
    fin >> temp;
    if (temp=="Address_bits:") fin >> addr_bits;
    else if (temp=="Cache_sets:") fin >> cache_sets;
    else if (temp=="Block_size:") fin >> block_size;
    else if (temp=="Associativity:") fin >> associativity;
    else cout << "Error" << endl;
    }
    fin.close();

    B_offset = log2(block_size);                   // block size,  2 ^ offset   to get offset of blocksize
    index_length = log2(cache_sets);                   //cache set 
    sets.resize(cache_sets);
    for (int i = 0; i < cache_sets; i++) sets[i].resize(associativity); 
}

void readRef(string path) {
    string line;
    ifstream fin(path);
    fin >> line;      // .benchmark
    fin >> testcase;  // testcase1:
    int miss_count = 0;
    while (true) {
        fin >> line;
        if (line == ".end") break;
        inputs.emplace_back(line);
    }
    outputs.resize(inputs.size());
    fin.close();
}

void output(string path) {
        ofstream fout(path);
        fout << "Address bits: " << addr_bits << endl;
        fout << "Block size: " << block_size << endl;
        fout << "Cache sets: " << cache_sets << endl;
        fout << "Associativity: " << associativity << endl << endl;
        fout << "Indexing bit count: " << index_length << endl;
        fout << "Indexing bits: ";
        for (auto i : indexing_bits) fout << i << " ";
        fout << "\nOffset bit count: " << B_offset << endl << endl;

        fout << ".benchmark " << testcase << endl;
        for (int i = 0; i < inputs.size(); i++) {
            fout << inputs[i] << " ";
            if (outputs[i]) {
                fout << "hit" << endl;
            } else {
                fout << "miss" << endl;
            }
        }
        fout << ".end" << endl << endl;
        fout << "Total cache miss count: " << miss_count << endl;
        fout.close();
    }

class Cache {
   public:
    Cache() {       //initialize
        addr_bits = 0;
        block_size = 0;
        cache_sets = 0;
        associativity = 0;
        miss_count = 0;
        testcase = "";
        inputs.clear();
        outputs.clear();
        indexing_bits.clear();
        sets.clear();   
    }
        
    void pickIndex() {
        //cout << B_offset + index_length << endl;
        for (int i = B_offset; i < B_offset + index_length; i++) {
            indexing_bits.emplace_back(i);  // insert a new element into the vector container, the new element is added to the end of the vector.
            //cout << i << endl ; // { 2 , 3}
        }
        reverse(indexing_bits.begin(), indexing_bits.end());    //reverse so print from msb to lsb { 3 , 2}
    }

    void LRUsimu() {
        for (int i = 0; i < inputs.size(); i++) {   // input[i] = address
            string index_addr, tag_addr;
            miss_count++;     // assummed that no cache in the addresss so increment every input
            string TAG_AND_INDEX = inputs[i].substr(0, addr_bits - B_offset);    //string without offset
            //cout << TAG_AND_INDEX << endl;

            for (int j : indexing_bits) {
                index_addr += TAG_AND_INDEX[addr_bits - j - 1];
                TAG_AND_INDEX[addr_bits - j - 1] = 'A';
                // cout << "index_addr " << index_addr << endl;
                // cout << "TAG_AND_INDEX " << TAG_AND_INDEX[addr_bits - j - 1] << endl;
            }

            for (char j : TAG_AND_INDEX) {
                if (j != 'A') tag_addr += j; //count how many bit is the tag 
                //cout << "tag_addr " << tag_addr << endl;
            }

            int index_addr_int = toDecimal(index_addr); //change string to decimal
            // cout << "index_addr_int = " << index_addr_int << endl;
 
            // check whether set is not full or tag already existed
            for (Tag &t : sets[index_addr_int]) {
                // if there is space
                if (t.tag_addr == "") { 
                    t.tag_addr = tag_addr; //allocation = tag
                    t.last = -1;   //update that it was the last added to
                    cache_hit = true;
                    break;
                }
                // tag already existed
                if (t.tag_addr == tag_addr) {
                    outputs[i] = 1;  // output [i] hit
                    miss_count--;    // found so cancel the increment by subtracting
                    t.last = -1;
                    cache_hit = true;
                    break;
                }
            }

            // if cache miss, 
            for (Tag &t : sets[index_addr_int]) {
                if (t.last == associativity - 1) { //replace Tag with last == associativity-1
                    t.tag_addr = tag_addr; 
                    t.last = -1;
                    break;
                }
            }

            // update the last by adding 1 to them
            for (Tag &t : sets[index_addr_int]) {
                t.last += 1;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    Cache Cache;
    readCache(string(argv[1]));
    readRef(string(argv[2]));
    Cache.pickIndex();
    Cache.LRUsimu();
    output(string(argv[3]));
    return 0;
}


// ./project  ../testcases/config/cache1.org ../testcases/bench/reference1.lst ../output/index.rpt
// ./project  ../testcases/config/cache2.org ../testcases/bench/reference2.lst ../output/index.rpt