#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <math.h>
#include <bitset>
#include <limits>
#include <cstdint>

using namespace std;

struct Cache_mem1
{
    int cache_size, block_size, cache_associativity, T, cache_size_pow, block_size_pow, mem_addr_int;
    int blocks_num, sets_num, sets_num_pow, tag_count, cache_associativity_pow, set_index, offset;
    int total_access, total_reads, total_writes, total_read_hits, total_read_misses, total_write_hits, total_write_misses, cycle;
    string tag;
    bool flag;
    float hit_ratio;
    vector<int> main_mem;
    vector<vector<int>> data_arr;
    vector<string> tag_arr;
    vector<int> dirty_bit_arr;
    vector<int> valid_bit_arr;
    vector<int> access_arr;
    vector<int> accessNo_arr;
    vector<int> not_access_arr;
    vector<int> priority_arr;

    Cache_mem1()
    {
        flag = 0;
        total_access = 0;
        total_reads = 0;
        total_writes = 0;
        total_read_hits = 0;
        total_read_misses = 0;
        total_write_hits = 0;
        total_write_misses = 0;
        cycle = 0;
        hit_ratio = 0.0;

        for (int i = 0; i < 4096; i++)
        {
            main_mem.push_back(i);
        }
        /*for (int i = 0; i < 16; i++)
        {
            int mmIndex = i * 256;
            int mmData = 0;
            for (int j = mmIndex; j < mmIndex + 256; j++)    //To enable the condition of 1 int = 1 byte
            {
                main_mem.push_back(mmData);
                mmData += 1;
            }
        }*/
    }

    int startCache(int cache_size1, int block_size1, int cache_associativity1, int T1)
    {
        cache_size = cache_size1;
        block_size = block_size1;
        cache_associativity = cache_associativity1;
        T = T1;

        cache_size_pow = powerOfTwo(cache_size);
        block_size_pow = powerOfTwo(block_size);
        cache_associativity_pow = powerOfTwo(cache_associativity);

        if (cache_size_pow == -1 || block_size_pow == -1 || cache_associativity_pow == -1)
        {
            cout << "One of the input parameters is not a power of 2" << endl;
            return 0;
        }

        blocks_num = pow(2, cache_size_pow - block_size_pow);
        sets_num = blocks_num / cache_associativity;
        sets_num_pow = powerOfTwo(sets_num);
        tag_count = 32 - sets_num_pow - block_size_pow;

        data_arr.resize(blocks_num);
        for (int i = 0; i < blocks_num; i++)
        {
            data_arr[i].resize(block_size, -1);
        }

        tag_arr.resize(blocks_num, "-1");
        string istr = "";
        for (int i = 0; i < tag_count; i++)
        {
            istr = istr + "-1";
        }
        for (int i = 0; i < blocks_num; i++)
        {
            tag_arr[i] = istr;
        }
        dirty_bit_arr.resize(blocks_num, 0);
        valid_bit_arr.resize(blocks_num, 0);
        access_arr.resize(blocks_num, 0);
        accessNo_arr.resize(blocks_num, 0);
        not_access_arr.resize(blocks_num, 0);
        priority_arr.resize(blocks_num, 0);
        return 1;
    }

    vector<int> cached(string instruction)
    {
        total_access += 1;
        cycle += 1;

        cout << "Instruction: " << instruction << endl;
        string delimiter = ",";
        int pos = instruction.find(delimiter);

        string token1 = instruction.substr(0, pos);
        mem_addr_int = atoi(token1.c_str());
        bitset<32> mem_addr(mem_addr_int);
        string mem_addr_str = mem_addr.to_string();
        string mem_addr_str_dup = mem_addr_str;

        string token2 = instruction.substr(pos + 1, 1);

        tag = mem_addr_str.substr(0, tag_count); //tag
        string index1 = mem_addr_str.substr(tag_count, sets_num_pow);
        bitset<32> index2(index1);
        set_index = index2.to_ulong(); // Set Index
        string offset1 = mem_addr_str_dup.erase(0, tag_count + sets_num_pow);
        bitset<32> offset2(offset1);
        offset = offset2.to_ulong(); // In block index

        vector<int> new_block(block_size, -1);
        vector<int> dummy_block(block_size, -1);

        if (token2 == "R")
        {
            total_reads += 1;
            int read_hit_miss = readHM();
            int read_result;

            if (read_hit_miss != -1)
            {
                total_read_hits += 1;
                read_result = data_arr[read_hit_miss][offset];
                cout << "Read Hit" << endl;
                cout << "Block Idex: " << read_hit_miss << endl;
                cout << "Block: ";
                printVector(data_arr[read_hit_miss]);
                cout << "Offset: " << offset << endl;
                cout << "Read Result: " << read_result << endl;
                cout << " " << endl;
                return data_arr[read_hit_miss];
            }
            else if (read_hit_miss == -1)
            {
                total_read_misses += 1;
                get_block_main_mem(new_block);
                read_result = new_block[offset];
                flag = 0;
                cout << "Read Miss" << endl;
                cout << "New Block: ";
                printVector(new_block);
                insert(new_block, index1);
                cout << "Read Result: " << read_result << endl;
                cout << " " << endl;
                return new_block;
            }
        }
        else if (token2 == "W")
        {
            total_writes += 1;
            instruction.erase(0, pos + delimiter.length() + 2);
            string write_data1 = instruction;
            int write_data = atoi(write_data1.c_str());

            /*if (write_data > 255)
            {
                cout << "Write Data: " << instruction << endl;
                cout << "Write Data Out of Range" << endl;       //To enable the condition of 1 int = 1 byte
                cout << " " << endl;
                return;
            }*/

            int write_hit_miss = readHM();

            if (write_hit_miss != -1)
            {
                total_write_hits += 1;
                data_arr[write_hit_miss][offset] = write_data;
                dirty_bit_arr[write_hit_miss] = 1;
                cout << "Write Hit" << endl;
                cout << "Block Idex: " << write_hit_miss << endl;
                cout << "Offset: " << offset << endl;
                cout << " " << endl;
                return dummy_block;
            }
            else if (write_hit_miss == -1)
            {
                total_write_misses += 1;
                get_block_main_mem(new_block);
                new_block[offset] = write_data;
                flag = 1;
                cout << "Write Miss" << endl;
                cout << "New Block: ";
                printVector(new_block);
                insert(new_block, index1);
                cout << " " << endl;
                return dummy_block;
            }
        }
        return dummy_block;
    }

    int readHM()
    {
        for (int i = 0; i < sets_num; i++)
        {
            if (i != set_index)
            {
                int block_index1 = i * cache_associativity;
                for (int j = block_index1; j < block_index1 + cache_associativity; j++)
                {
                    if (valid_bit_arr[j] == 1)
                    {
                        if (priority_arr[j] == 1)
                        {
                            not_access_arr[j] += 1;
                            if (not_access_arr[j] >= T)
                            {
                                priority_arr[j] = 0;
                                not_access_arr[j] = 0;
                            }
                        }
                    }
                }
            }
        }

        int block_index = set_index * cache_associativity;
        bool indexFlag = false;
        int hit_block_index = 0;

        for (int i = block_index; i < block_index + cache_associativity; i++)
        {
            if (valid_bit_arr[i] == 1)
            {
                if (tag_arr[i] == tag)
                {
                    if (access_arr[i] == 0)
                    {
                        access_arr[i] += 1;
                        accessNo_arr[i] = cycle;
                        priority_arr[i] = 1;
                        not_access_arr[i] = 0;
                        hit_block_index = i;
                        indexFlag = true;
                    }
                    else
                    {
                        access_arr[i] += 1;
                        accessNo_arr[i] = cycle;
                        hit_block_index = i;
                        indexFlag = true;
                    }
                }
                else
                {
                    if (priority_arr[i] == 1)
                    {
                        not_access_arr[i] += 1;
                        if (not_access_arr[i] >= T)
                        {
                            priority_arr[i] = 0;
                            not_access_arr[i] = 0;
                        }
                    }
                }
            }
        }
        if (indexFlag)
        {
            return hit_block_index;
        }
        else
        {
            return -1;
        }
    }

    void insert(vector<int> &new_block, string index1)
    {
        int block_index = set_index * cache_associativity;
        for (int i = block_index; i < block_index + cache_associativity; i++)
        {
            if (valid_bit_arr[i] == 0)
            {
                valid_bit_arr[i] = 1;
                if (flag)
                {
                    dirty_bit_arr[i] = 1;
                }
                else
                {
                    dirty_bit_arr[i] = 0;
                }
                tag_arr[i] = tag;
                for (int j = 0; j < block_size; j++)
                {
                    data_arr[i][j] = new_block[j];
                }
                access_arr[i] = 0;
                accessNo_arr[i] = cycle;
                priority_arr[i] = 0;
                not_access_arr[i] = 0;
                return;
            }
        }
        int replacement_candidate = replacement();
        cout << "Replacement Candidate Index: " << replacement_candidate << endl;

        if (dirty_bit_arr[replacement_candidate] == 0)
        {
            valid_bit_arr[replacement_candidate] = 1;
            if (flag)
            {
                dirty_bit_arr[replacement_candidate] = 1;
            }
            else
            {
                dirty_bit_arr[replacement_candidate] = 0;
            }
            tag_arr[replacement_candidate] = tag;
            for (int j = 0; j < block_size; j++)
            {
                data_arr[replacement_candidate][j] = new_block[j];
            }
            access_arr[replacement_candidate] = 0;
            accessNo_arr[replacement_candidate] = cycle;
            priority_arr[replacement_candidate] = 0;
            not_access_arr[replacement_candidate] = 0;
            return;
        }
        else if (dirty_bit_arr[replacement_candidate] == 1)
        {
            eviction(replacement_candidate, index1);
            valid_bit_arr[replacement_candidate] = 1;
            if (flag)
            {
                dirty_bit_arr[replacement_candidate] = 1;
            }
            else
            {
                dirty_bit_arr[replacement_candidate] = 0;
            }
            tag_arr[replacement_candidate] = tag;
            for (int j = 0; j < block_size; j++)
            {
                data_arr[replacement_candidate][j] = new_block[j];
            }
            access_arr[replacement_candidate] = 0;
            accessNo_arr[replacement_candidate] = cycle;
            priority_arr[replacement_candidate] = 0;
            not_access_arr[replacement_candidate] = 0;
            return;
        }
    }

    void eviction(int replacement_candidate, string index1)
    {
        cout << "Eviction Happened" << endl;
        string replace_tag = tag_arr[replacement_candidate];
        string initial_index = replace_tag + index1;

        for (int i = 0; i < block_size_pow; i++)
        {
            initial_index = initial_index + "0";
        }
        bitset<32> index_bit(initial_index);
        int index_int = index_bit.to_ulong();

        for (int i = 0; i < block_size; i++)
        {
            main_mem[index_int] = data_arr[replacement_candidate][i];
            index_int += 1;
        }
        return;
    }

    int replacement()
    {
        cout << "Replacement Happened" << endl;
        int block_index = set_index * cache_associativity;
        int lowest_access1 = 2147483647;
        int lowest_access2 = 2147483647;
        int index1 = 0;
        int index2 = 0;
        int count = 0;

        for (int i = block_index; i < block_index + cache_associativity; i++)
        {
            if (priority_arr[i] == 0)
            {
                count += 1;
                int low_access1 = accessNo_arr[i];
                int low_index1 = i;
                if (low_access1 < lowest_access1)
                {
                    lowest_access1 = low_access1;
                    index1 = low_index1;
                }
            }
            int low_access2 = accessNo_arr[i];
            int low_index2 = i;
            if (low_access2 < lowest_access2)
            {
                lowest_access2 = low_access2;
                index2 = low_index2;
            }
        }
        if (count != 0)
        {
            return index1;
        }
        else if (count == 0)
        {
            return index2;
        }
        return 0;
    }

    void get_block_main_mem(vector<int> &new_block)
    {
        int block_mem_addr_int1 = mem_addr_int - 1;
        int block_mem_addr_int2 = mem_addr_int + 1;

        if (offset != 0)
        {
            for (int i = offset - 1; i >= 0; i--)
            {
                new_block[i] = main_mem[block_mem_addr_int1];
                block_mem_addr_int1 -= 1;
            }
        }
        new_block[offset] = main_mem[mem_addr_int];

        if (offset != block_size - 1)
        {
            for (int i = offset + 1; i < block_size; i++)
            {
                new_block[i] = main_mem[block_mem_addr_int2];
                block_mem_addr_int2 += 1;
            }
        }
    }

    int powerOfTwo(int n)
    {
        int pow = 0;

        if (n == 0)
            return -1;

        while (n != 1)
        {
            if (n % 2 != 0)
                return -1;
            pow += 1;
            n = n / 2;
        }
        return pow;
    }

    void printVector(vector<int> &new_block)
    {
        for (int i = 0; i < new_block.size(); i++)
        {
            if (i == new_block.size() - 1)
            {
                cout << new_block[i] << endl;
            }
            else
            {
                cout << new_block[i] << ",";
            }
        }
        return;
    }

    void printCache()
    {
        cout << " " << endl;
        cout << "V ";
        cout << "D ";
        cout << "NA ";
        cout << "C ";
        cout << "P ";
        cout << "T" << endl;

        for (int i = 0; i < blocks_num; i++)
        {
            cout << valid_bit_arr[i] << " ";
            cout << dirty_bit_arr[i] << " ";
            cout << not_access_arr[i] << " ";
            cout << accessNo_arr[i] << " ";
            cout << priority_arr[i] << " ";
            cout << tag_arr[i] << endl;
        }

        cout << " " << endl;
        cout << "data_Array" << endl;

        for (int i = 0; i < blocks_num; i++)
        {
            for (int j = 0; j < block_size; j++)
            {
                if (j == block_size - 1)
                {
                    cout << data_arr[i][j] << endl;
                }
                else
                {
                    cout << data_arr[i][j] << ",";
                }
            }
        }

        cout << " " << endl;
        cout << "Cache Statistics" << endl;

        cout << "Number of Access = " << total_access << endl;
        cout << "Number of Reads = " << total_reads << endl;
        cout << "Number of Read Hits = " << total_read_hits << endl;
        cout << "Number of Read Misses = " << total_read_misses << endl;
        cout << "Number of Writes = " << total_writes << endl;
        cout << "Number of Write Hits = " << total_write_hits << endl;
        cout << "Number of Write Misses = " << total_write_misses << endl;
        hit_ratio = ((total_read_hits + total_write_hits) / (float)total_access);
        cout << fixed;
        cout.precision(3);
        cout << "Hit Ratio = " << hit_ratio << endl;
        cout << " " << endl;
        return;
    }
    void printMainMem()
    {
        for (int i = 0; i < 200; i++)
        {
            cout << main_mem[i] << endl;
        }
        return;
    }
};

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        cout << "NO INPUT FILE PROVIDED!!\n";
        exit(1);
    }
    ifstream file;
    file.open(argv[1]);
    if (!file.is_open())
    {
        cout << "FILE COULD NOT OPENED\n";
        exit(2);
    }
    string cache_size1, block_size1, cache_associativity1, T1;
    getline(file, cache_size1);
    getline(file, block_size1);
    getline(file, cache_associativity1);
    getline(file, T1);

    int cache_size = atoi(cache_size1.c_str());
    int block_size = atoi(block_size1.c_str());
    int cache_associativity = atoi(cache_associativity1.c_str());
    int T = atoi(T1.c_str());

    Cache_mem1 cm = Cache_mem1();
    int start = cm.startCache(cache_size, block_size, cache_associativity, T);

    if (start == 0)
    {
        return 0;
    }
    else
    {
        string str;

        while (getline(file, str))
        {
            string delimiter = ",";
            int pos = str.find(delimiter);
            string token1 = str.substr(0, pos);
            int mem_addr_int1 = atoi(token1.c_str());
            if (mem_addr_int1 > 4095)
            {
                cout << "Instruction: " << str << endl;
                cout << "Memory Address Out of Range" << endl;
                cout << " " << endl;
                continue;
            }
            cm.cached(str);
            cm.printCache();
        }
    }
    //cm.printCache();
    //cm.printMainMem();
    return 1;
}
