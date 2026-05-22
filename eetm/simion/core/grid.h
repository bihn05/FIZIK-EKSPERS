#pragma once
#include <vector>
#include <cstdint>

struct Grid2D {
    int nx, ny;
    float dx;
    std::vector<float> phi;
    std::vector<float> rho;
    std::vector<uint8_t> mask; // 0=gas, 1=electrode

    Grid2D(int nx, int ny, float dx)
        : nx(nx), ny(ny), dx(dx),
          phi(nx * ny, 0.0f),
          rho(nx * ny, 0.0f),
          mask(nx * ny, 0) {}

    float& at_phi(int i, int j) { return phi[j * nx + i]; }
    float& at_rho(int i, int j) { return rho[j * nx + i]; }
    uint8_t& at_mask(int i, int j) { return mask[j * nx + i]; }

    void clear_rho() {
        std::fill(rho.begin(), rho.end(), 0.0f);
    }
};
