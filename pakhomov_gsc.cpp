#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <sstream>
#include <iterator>

using namespace std;

// --- Config ---
const int THREAD_COUNT = thread::hardware_concurrency(); // use all cores
// ---------------

mutex output_mutex;
atomic<bool> found(false);
uint64_t found_seed = 0;

// Read entire binary file
vector<unsigned char> read_file(const string& filename) {
    ifstream file(filename, ios::binary);
    return vector<unsigned char>((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

// Write binary file
void write_file(const string& filename, const vector<unsigned char>& data) {
    ofstream file(filename, ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

// Convert bytes -> packed uint64_t bit vector
vector<uint64_t> file_to_bits_u64(const vector<unsigned char>& data, size_t& out_bit_count) {
    size_t bit_count = data.size() * 8;
    vector<uint64_t> bits((bit_count + 63) / 64, 0);

    for (size_t i = 0; i < bit_count; ++i) {
        if (data[i / 8] & (1 << (7 - (i % 8)))) {
            bits[i / 64] |= (uint64_t(1) << (63 - (i % 64)));
        }
    }
    out_bit_count = bit_count;
    return bits;
}

// Generate PRNG bits into uint64_t vector
vector<uint64_t> generate_bits_u64(uint64_t seed, size_t bit_count) {
    mt19937 rng(seed);
    vector<uint64_t> bits((bit_count + 63) / 64, 0);

    for (size_t i = 0; i < bit_count; ++i) {
        if (rng() % 2) {
            bits[i / 64] |= (uint64_t(1) << (63 - (i % 64)));
        }
    }
    return bits;
}

// Convert uint64_t bit vector -> bytes
vector<unsigned char> bits_to_bytes_u64(const vector<uint64_t>& bits, size_t bit_count) {
    vector<unsigned char> bytes((bit_count + 7) / 8, 0);

    for (size_t i = 0; i < bit_count; ++i) {
        if (bits[i / 64] & (uint64_t(1) << (63 - (i % 64)))) {
            bytes[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    return bytes;
}

// Worker thread function
void search_seed(const vector<uint64_t>& target_bits, size_t bit_count, uint64_t start_seed, uint64_t step) {
    uint64_t seed = start_seed;
    while (!found) {
        vector<uint64_t> gen = generate_bits_u64(seed, bit_count);
        if (gen == target_bits) {
            lock_guard<mutex> lock(output_mutex);
            if (!found) {
                found = true;
                found_seed = seed;
            }
            break;
        }
        seed += step;
    }
}

// Compress: find seed
void compress(const string& input_file, const string& output_file) {
    vector<unsigned char> data = read_file(input_file);
    size_t bit_count;
    vector<uint64_t> target_bits = file_to_bits_u64(data, bit_count);

    cout << "Target: " << bit_count << " bits. Using " << THREAD_COUNT << " threads." << endl;

    vector<thread> threads;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(search_seed, cref(target_bits), bit_count, i, THREAD_COUNT);
    }

    for (auto& t : threads) {
        t.join();
    }

    if (found) {
        ofstream out(output_file);
        out << found_seed << "," << bit_count;
        out.close();
        cout << "Seed found: " << found_seed << endl;
        cout << "Saved to: " << output_file << endl;
    } else {
        cout << "No seed found (unexpected)." << endl;
    }
}

// Decompress: regen file
void decompress(const string& input_file, const string& output_file) {
    ifstream in(input_file);
    uint64_t seed;
    size_t bit_count;
    char comma;
    in >> seed >> comma >> bit_count;

    vector<uint64_t> bits = generate_bits_u64(seed, bit_count);
    vector<unsigned char> bytes = bits_to_bytes_u64(bits, bit_count);
    write_file(output_file, bytes);

    cout << "Decompressed file with seed " << seed << " (" << bit_count << " bits)." << endl;
}

// Main
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Pakhomov GSC (Generative Seed Compression) (2025)\n";
        cout << "Usage:\n";
        cout << "  " << argv[0] << " compress <input_file> [compressed_file]\n";
        cout << "  " << argv[0] << " decompress <compressed_file> [output_file]\n";
        return 1;
    }

    string command = argv[1];
    if (command == "compress") {
        string input = argv[2];
        string output = (argc > 3) ? argv[3] : "compressed.txt";
        compress(input, output);
    } else if (command == "decompress") {
        string input = argv[2];
        string output = (argc > 3) ? argv[3] : "output.bin";
        decompress(input, output);
    } else {
        cout << "Unknown command: " << command << endl;
        return 1;
    }

    return 0;
}
