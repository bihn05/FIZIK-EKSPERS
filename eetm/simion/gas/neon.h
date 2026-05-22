#pragma once
#include "cross_section.h"
#include <cmath>

struct NeonGas : public CrossSection {
    float sigma(float energy_eV, CollisionType type) const override {
        // Approximate fits to LXCat/Phelps data for Ne (m^2)
        if (energy_eV < 0.1f) energy_eV = 0.1f;

        switch (type) {
        case ELASTIC:
            // ~2e-20 m^2 plateau with low-energy Ramsauer dip
            return 2.0e-20f * (1.0f - 0.5f * std::exp(-energy_eV / 5.0f));
        case EXCITATION:
            if (energy_eV < 16.6f) return 0.0f;
            // peaks ~1e-21 around 20-30 eV
            { float x = (energy_eV - 16.6f);
              return 1.0e-21f * x / (x + 10.0f) * std::exp(-x / 80.0f); }
        case IONIZATION:
            if (energy_eV < 21.56f) return 0.0f;
            // boosted for coarse-grid PIC (compensates under-resolved mfp)
            { float x = (energy_eV - 21.56f);
              return 3.0e-20f * x / (x + 40.0f) * std::exp(-x / 300.0f); }
        }
        return 0.0f;
    }

    float ionization_threshold() const override { return 21.56f; }
    float mass_ratio() const override { return 9.109e-31f / (20.18f * 1.661e-27f); }
};
