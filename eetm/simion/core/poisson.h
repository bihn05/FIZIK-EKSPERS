#pragma once
#include "grid.h"
#include <cmath>

inline void solve_poisson(Grid2D& grid, int max_iter = 500, float tol = 1e-4f) {
    constexpr float omega = 1.85f;
    float dx2 = grid.dx * grid.dx;
    constexpr float eps0 = 8.854e-12f;

    for (int iter = 0; iter < max_iter; iter++) {
        float max_diff = 0.0f;
        for (int j = 1; j < grid.ny - 1; j++) {
            for (int i = 1; i < grid.nx - 1; i++) {
                if (grid.at_mask(i, j) == 1) continue; // conductor: fixed
                if (grid.at_mask(i, j) == 2) continue; // insulator: skip
                float phi_new = 0.25f * (
                    grid.at_phi(i+1, j) + grid.at_phi(i-1, j) +
                    grid.at_phi(i, j+1) + grid.at_phi(i, j-1) +
                    dx2 * grid.at_rho(i, j) / eps0
                );
                float diff = phi_new - grid.at_phi(i, j);
                grid.at_phi(i, j) += omega * diff;
                float ad = std::abs(diff);
                if (ad > max_diff) max_diff = ad;
            }
        }
        if (max_diff < tol) break;
    }
}
