#pragma once

// Scalar 1-D Kalman filter (constant-position model).
// Works independently on each sensor channel.
//
// Tuning:
//   Q  – process noise variance: how much the true value can change per sample.
//         Larger Q → filter tracks fast changes but lets through more noise.
//   R  – measurement noise variance: variance of the raw sensor readings.
//         Larger R → heavier smoothing, slower response.
//
// At steady state the Kalman gain settles to K ≈ sqrt(Q/R).
// Time constant ≈ 1 / (K * sample_rate).

struct KalmanFilter {
    float Q;   // process noise variance
    float R;   // measurement noise variance
    float P;   // estimate error covariance
    float x;   // filtered state estimate

    void init(float process_noise, float meas_noise, float x0 = 0.0f) {
        Q = process_noise;
        R = meas_noise;
        P = 1.0f;   // start uncertain; filter converges within a few samples
        x = x0;
    }

    float update(float z) {
        P     += Q;                // predict: covariance grows by process noise
        float K = P / (P + R);    // Kalman gain
        x     += K * (z - x);     // correct estimate with measurement
        P     *= (1.0f - K);      // updated covariance
        return x;
    }
};
