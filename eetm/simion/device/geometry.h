#pragma once
#include "../core/grid.h"

struct Electrode {
    float cx, cy;   // center position (m)
    float radius;   // wire radius (m)
    float voltage;  // applied voltage (V)
};

inline void apply_electrode(Grid2D& grid, const Electrode& elec) {
    for (int j = 0; j < grid.ny; j++) {
        for (int i = 0; i < grid.nx; i++) {
            float x = i * grid.dx;
            float y = j * grid.dx;
            float dx = x - elec.cx;
            float dy = y - elec.cy;
            if (dx * dx + dy * dy <= elec.radius * elec.radius) {
                grid.at_mask(i, j) = 1;
                grid.at_phi(i, j) = elec.voltage;
            }
        }
    }
}

inline void apply_shell(Grid2D& grid, float cx, float cy,
                        float inner_r, float outer_r) {
    for (int j = 0; j < grid.ny; j++) {
        for (int i = 0; i < grid.nx; i++) {
            float x = i * grid.dx;
            float y = j * grid.dx;
            float dx = x - cx;
            float dy = y - cy;
            float r2 = dx * dx + dy * dy;
            if (r2 >= inner_r * inner_r && r2 <= outer_r * outer_r) {
                grid.at_mask(i, j) = 2; // insulating shell
            }
        }
    }
}
