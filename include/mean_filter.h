#pragma once

// Very simple moving-average (mean) filter.
// Keeps the last N samples in a ring buffer and returns their average.
// Larger N → smoother output but slower response.

template <uint8_t N>
struct MeanFilter {
    float   buf[N] = {0};
    uint8_t count  = 0;   // how many valid samples so far (until buffer fills)
    uint8_t head   = 0;   // next write position

    void reset(float seed = 0.0f) {
        for (uint8_t i = 0; i < N; i++) buf[i] = seed;
        count = 0;
        head  = 0;
    }

    float update(float sample) {
        buf[head] = sample;
        head = (head + 1) % N;
        if (count < N) count++;

        float sum = 0.0f;
        for (uint8_t i = 0; i < count; i++) sum += buf[i];
        return sum / count;
    }
};
