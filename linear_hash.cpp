/*
 * Linear Hashing Implementation in C++.
 *
 * Insertion Logic: take hash(key) and insert into the corresponding bucket.
 * 
 * 1. out = hash(key)
 * 2. idx = get_bucket_index(out)
 * 3. table[idx].push_back(key)
 *                          buckets 
 *               hash table ┌─────┐ 
 *   split_ptr   ┌─────┐ ┌─►│  0  │ 
 *   ───────────►│  0  ├─┘  │     │ 
 *               ───────    ┼─────┼ 
 *               │  1  ├──┐ │  1  │ 
 *  hash(key)    ──────── └►│     │ 
 *               │  2  ├─┐  ┼─────┼ 
 *               ─────── │  │  2  │ 
 *               │  3  │┐└─►│     │ 
 *               └─────┴│   ┼─────┼─
 *                      │   │  3  │ 
 *                      └──►│     │ 
 *                          └─────┘ 
 * TO BE IMPROVED
 */

#include <algorithm>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <cmath>
#include <cstdint>

#define MAX_BUCKET_SIZE 64

struct OverflowBitmap {
    std::vector<uint64_t> bitmap;
    uint64_t num_bits;

    explicit OverflowBitmap(uint64_t size) : num_bits(size) {
        auto num_of_blocks = num_bits + 63 / 64; // round up
        bitmap.resize(num_of_blocks);
    }

    void set(uint64_t idx) {
        auto block = idx / bitmap.size();
        auto offset = idx % bitmap.size();
        bitmap[block] |= 1 << offset;
    }

    void clear(uint64_t idx) {
        auto block = idx / bitmap.size();
        auto offset = idx % bitmap.size();
        bitmap[block] &= ~(1 << offset);
    }

    bool is_overflow() {
        auto it = std::find_if(bitmap.begin(), bitmap.end(), 
                               [](uint64_t& val) { return val != 0; });
        if (it != bitmap.end()) {
            return true;
        }
        return false;
    }
};

class LinearHash {
private:
    size_t _table_size;                             // initial number of buckets
    size_t _bucket_size{2};
    std::vector<std::list<uint64_t>> _hash_table;   // holding buckets
    int _split_ptr{0};                              // pointer to the next bucket to split

    OverflowBitmap _overflow_bitmap;
    int _round {0};

    LinearHash() = delete;

    size_t _hash(const uint64_t key, size_t round = 0) const {
        // return a hash value for the given key,
        // hash function is modulo the size of the hash table
        return key % (_table_size * (1ULL << round));
    }

    void _redistribute_bucket(uint64_t idx) {
        std::list<uint64_t> keys_to_rehash = _hash_table[idx];
        _hash_table[idx].clear();

        size_t new_idx = -1;
        for (auto const& key : keys_to_rehash) {
            new_idx = _hash(key, _round + 1);

            std::cout << "By redistribution, key " << key << " goes into " << new_idx << std::endl;
            _hash_table[new_idx].push_back(key);
        }
        _hash_table[idx].size() > _bucket_size ? _overflow_bitmap.set(idx)
                                               : _overflow_bitmap.clear(idx);
        _hash_table[new_idx].size() > _bucket_size ? _overflow_bitmap.set(new_idx)
                                                : _overflow_bitmap.clear(new_idx);
    }

    void _update_split_ptr() {
        if ((_split_ptr + 1) % _table_size) {
            _split_ptr++;
        } else {
            _split_ptr = 0;
            _table_size = _table_size * 2; // update table size when completed the round.
        }
        std::cout << "New hash table size: " << _hash_table.size()
                  << "\nsplit ptr points to the bucket: " << _split_ptr
                  << "\nfixed table size: " << _table_size << std::endl;
    }

public: 
    explicit LinearHash(size_t table_size = 2)
        : _table_size(table_size)
        , _hash_table(table_size)
        , _overflow_bitmap(MAX_BUCKET_SIZE) {
            // allocate hash table with initial size
        }
    ~LinearHash() {
        // destructor
    }

    void insert(uint64_t key) {
        std::cout <<"Inserting key: " << key << std::endl;

        size_t idx = _hash(key);
        if (_split_ptr > idx) {
            // check if the key goes into the new bucket or old bucket
            idx = _hash(key, _round + 1);
        }
        _hash_table[idx].push_back(key);
        std::cout <<"Key " << key << " goes into bucket " << idx << std::endl;

        if (_hash_table[idx].size() > _bucket_size) {
            _overflow_bitmap.set(idx);
        }

        // there is at least one bucket overflowing, we need to split the bucket
        // which split pointer points to, and add a new bucket to the hash table
        if (_overflow_bitmap.is_overflow()) {
            // add a new bucket and re-distribute the keys in the bucket pointed
            // by the split pointer
            _hash_table.push_back(std::list<uint64_t>());
            _redistribute_bucket(_split_ptr);

            _update_split_ptr();
        }
}
};

int main() {
    LinearHash table(2);
    table.insert(1);   // goes into bucket 1
    table.insert(2);   // goes into bucket 0
    table.insert(3);   // goes into bucket 1
    table.insert(4);   // goes into bucket 0

    // 5 goes into bucket 1, triggers overflow (bucket_size=3)
    // splitting bucket 0 and redistribute:
    // 2 -> 2
    // 4 -> 0
    table.insert(5);

    // 6 goes into bucket 2, triggers overflow (bucket_size=4)
    // splitting bucket 1 and redistribute:
    // 1 -> 1
    // 3 -> 3
    // 5 -> 1
    table.insert(6);

    table.insert(7);   // goes into bucket 3
    table.insert(8);   // goes into bucket 0

    // 9 goes into bucket 1, overflow (bucket_size=5)
    // splitting bucket 0 and redistribute:
    // 4 -> 4
    // 8 -> 0
    table.insert(9);

    // 10 goes into bucket 2, overflow (bucket_size=6)
    // splitting bucket 1 and redistribute:
    // 1 -> 1
    // 5 -> 5
    // 9 -> 1
    table.insert(10);

    // 11 goes into bucket 3, overflow (bucket_size=7)
    // splitting bucket 2 and redistribute:
    // 2 ->  2
    // 6 ->  6
    // 10 -> 2
    table.insert(11);

    // 12 goes into bucket 4, overflow (bucket_size=8)
    // splitting bucket 3 and redistribute:
    // 3 ->  3
    // 7 ->  7
    // 11 -> 3
    table.insert(12);
}
