#pragma once
#include "particle.h"
#include "grid.h"
#include "../gas/cross_section.h"
#include <cmath>
#include <random>

inline std::mt19937& rng() {
    static std::mt19937 gen(42);
    return gen;
}

inline float randf() {
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng());
}

constexpr float MCC_E_CHARGE = 1.602e-19f;
constexpr float MCC_E_MASS   = 9.109e-31f;

inline void collide(ParticleList& plist, const Grid2D& /*grid*/,
                    const CrossSection& gas, float n_gas, float dt) {
    std::vector<Particle> new_particles;

    for (auto& p : plist.particles) {
        if (!p.active || p.species != ELECTRON) continue;

        float v = std::sqrt(p.vx * p.vx + p.vy * p.vy);
        float energy_eV = 0.5f * MCC_E_MASS * v * v / MCC_E_CHARGE;

        float sigma_ion = gas.sigma(energy_eV, IONIZATION);
        float sigma_el  = gas.sigma(energy_eV, ELASTIC);
        float sigma_ex  = gas.sigma(energy_eV, EXCITATION);
        float sigma_tot = sigma_ion + sigma_el + sigma_ex;

        float prob = 1.0f - std::exp(-n_gas * sigma_tot * v * dt);
        if (randf() > prob) continue;

        float r = randf() * sigma_tot;
        if (r < sigma_ion && energy_eV > gas.ionization_threshold()) {
            float remaining = energy_eV - gas.ionization_threshold();
            float e1 = remaining * randf();
            float speed1 = std::sqrt(2.0f * e1 * MCC_E_CHARGE / MCC_E_MASS);
            float speed2 = std::sqrt(2.0f * (remaining - e1) * MCC_E_CHARGE / MCC_E_MASS);

            float angle1 = randf() * 2.0f * 3.14159265f;
            p.vx = speed1 * std::cos(angle1);
            p.vy = speed1 * std::sin(angle1);

            float angle2 = randf() * 2.0f * 3.14159265f;
            Particle ne;
            ne.x = p.x; ne.y = p.y;
            ne.vx = speed2 * std::cos(angle2);
            ne.vy = speed2 * std::sin(angle2);
            ne.weight = p.weight;
            ne.species = ELECTRON;
            ne.active = true;
            ne.life = -1.0f;
            new_particles.push_back(ne);

            // ion at rest (thermal velocity negligible)
            Particle ion;
            ion.x = p.x; ion.y = p.y;
            ion.vx = 0; ion.vy = 0;
            ion.weight = p.weight;
            ion.species = ION;
            ion.active = true;
            ion.life = -1.0f;
            new_particles.push_back(ion);
        } else if (r < sigma_ion + sigma_ex) {
            // excitation: Ne* formed, will emit photon
            float remaining = energy_eV - 16.6f;
            if (remaining < 0) remaining = 0.1f;
            float speed = std::sqrt(2.0f * remaining * MCC_E_CHARGE / MCC_E_MASS);
            float angle = randf() * 2.0f * 3.14159265f;
            p.vx = speed * std::cos(angle);
            p.vy = speed * std::sin(angle);

            // photon from Ne* de-excitation (~20ns lifetime)
            Particle ph;
            ph.x = p.x; ph.y = p.y;
            ph.vx = 0; ph.vy = 0;
            ph.weight = 1;
            ph.species = FLASH;
            ph.active = true;
            ph.life = 20e-9f;
            new_particles.push_back(ph);
        } else {
            // elastic: random scatter, small energy loss
            float angle = randf() * 2.0f * 3.14159265f;
            p.vx = v * std::cos(angle);
            p.vy = v * std::sin(angle);
        }
    }

    for (auto& np : new_particles) {
        plist.add(np);
    }
}
