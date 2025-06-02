// xoshiro256.hpp - Public domain portable PRNG
#pragma once
#include <cstdint>
#include <array>

class xoshiro256pp {
public:
    using state_t = std::array<uint64_t, 4>;

    explicit xoshiro256pp(uint64_t seed) {
        seed_splitmix64(seed);
    }

    uint64_t next() {
        const uint64_t result = rotl(s[0] + s[3], 23) + s[0];

        const uint64_t t = s[1] << 17;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = rotl(s[3], 45);

        return result;
    }

private:
    state_t s;

    static uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    void seed_splitmix64(uint64_t seed) {
        uint64_t z = seed;
        for (int i = 0; i < 4; ++i) {
            s[i] = splitmix64(z);
            z += 0x9E3779B97F4A7C15;
        }
    }

    static uint64_t splitmix64(uint64_t& z) {
        uint64_t result = z;
        result = (result ^ (result >> 30)) * 0xbf58476d1ce4e5b9;
        result = (result ^ (result >> 27)) * 0x94d049bb133111eb;
        return result ^ (result >> 31);
    }
};
