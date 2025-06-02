// pakhomov_gsc.cl with xoshiro256++
inline ulong rotl(const ulong x, int k) {
    return (x << k) | (x >> (64 - k));
}

ulong xoshiro_next(ulong *state) {
    ulong result = rotl(state[0] + state[3], 23) + state[0];

    ulong t = state[1] << 17;

    state[2] ^= state[0];
    state[3] ^= state[1];
    state[1] ^= state[2];
    state[0] ^= state[3];

    state[2] ^= t;

    state[3] = rotl(state[3], 45);

    return result;
}

__kernel void seed_search(__global ulong *result, __global ulong *target, ulong bit_count, ulong start_seed, __global int *found_flag) {
    size_t gid = get_global_id(0);
    ulong seed = start_seed + gid;

    // Seed the state with splitmix64 for now
    ulong state[4];
    ulong z = seed;
    for (int i = 0; i < 4; i++) {
        ulong v = z;
        v = (v ^ (v >> 30)) * 0xbf58476d1ce4e5b9;
        v = (v ^ (v >> 27)) * 0x94d049bb133111eb;
        v = v ^ (v >> 31);
        state[i] = v;
        z += 0x9E3779B97F4A7C15;
    }

    size_t word_count = (bit_count + 63) / 64;
    int match = 1;

    for (size_t i = 0; i < word_count; ++i) {
        ulong generated = xoshiro_next(state);

        // If it's the last word, mask unused bits
        if (i == word_count - 1 && bit_count % 64 != 0) {
            ulong mask = ((1UL << (bit_count % 64)) - 1) << (64 - (bit_count % 64));
            generated &= mask;
        }

        if (generated != target[i]) {
            match = 0;
            break;
        }
    }

    if (match) {
        result[0] = seed;

        found_flag[0] = 1;
    }
}
