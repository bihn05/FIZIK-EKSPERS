#pragma once
#include "geometry.h"

namespace CD1 {
    constexpr float WIRE_RADIUS   = 0.5e-3f;   // 0.5 mm
    constexpr float WIRE_SPACING  = 3.0e-3f;   // 3 mm axis-to-axis
    constexpr float TUBE_DIAMETER = 5.0e-3f;   // 5 mm inner
    constexpr float SHELL_OUTER   = 6.0e-3f;   // 6 mm outer
    constexpr float TUBE_LENGTH   = 5.0e-3f;   // 5 mm (not used in 2D cross-section)
    constexpr float GAS_PRESSURE  = 5.0f;      // Torr
    constexpr float GAS_TEMP      = 300.0f;    // K
    constexpr float GAMMA         = 0.3f;      // boosted for coarse grid

    // derived: gas number density (m^-3)
    constexpr float N_GAS = (GAS_PRESSURE * 133.3f) / (1.38e-23f * GAS_TEMP);

    inline void setup(Grid2D& grid, float voltage) {
        float cx = grid.nx * grid.dx * 0.5f;
        float cy = grid.ny * grid.dx * 0.5f;
        float half_spacing = WIRE_SPACING * 0.5f;

        Electrode cathode{cx - half_spacing, cy, WIRE_RADIUS, 0.0f};
        Electrode anode{cx + half_spacing, cy, WIRE_RADIUS, voltage};

        apply_electrode(grid, cathode);
        apply_electrode(grid, anode);

        // insulating glass shell
        apply_shell(grid, cx, cy, TUBE_DIAMETER * 0.5f, SHELL_OUTER * 0.5f);
    }

    inline void update_anode_voltage(Grid2D& grid, float new_voltage) {
        float cx = grid.nx * grid.dx * 0.5f;
        float cy = grid.ny * grid.dx * 0.5f;
        float anode_cx = cx + WIRE_SPACING * 0.5f;
        for (int j = 0; j < grid.ny; j++) {
            for (int i = 0; i < grid.nx; i++) {
                if (grid.at_mask(i, j) != 1) continue;
                float x = i * grid.dx;
                float y = j * grid.dx;
                float dx = x - anode_cx;
                float dy = y - cy;
                if (dx * dx + dy * dy <= WIRE_RADIUS * WIRE_RADIUS) {
                    grid.at_phi(i, j) = new_voltage;
                }
            }
        }
    }
}
