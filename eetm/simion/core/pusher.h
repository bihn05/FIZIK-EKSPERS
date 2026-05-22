#pragma once
#include "particle.h"
#include "grid.h"
#include <cmath>
#include <random>
#include <vector>

constexpr float E_CHARGE = 1.602e-19f;
constexpr float E_MASS   = 9.109e-31f;
constexpr float NE_MASS  = 20.18f * 1.661e-27f;

inline std::mt19937& push_rng() {
    static std::mt19937 gen(123);
    return gen;
}
inline float push_randf() {
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(push_rng());
}

inline void deposit_charge(ParticleList& plist, Grid2D& grid) {
    grid.clear_rho();
    float cell_vol = grid.dx * grid.dx;
    for (auto& p : plist.particles) {
        if (!p.active || p.species == FLASH) continue;
        int i = int(p.x / grid.dx);
        int j = int(p.y / grid.dx);
        if (i < 0 || i >= grid.nx || j < 0 || j >= grid.ny) continue;
        float q = (p.species == ELECTRON) ? -E_CHARGE : E_CHARGE;
        grid.at_rho(i, j) += q * p.weight / cell_vol;
    }
}

inline float push(ParticleList& plist, Grid2D& grid, float dt,
                 float cathode_voltage, float gamma) {
    std::vector<Particle> secondaries;
    float anode_current = 0.0f;

    for (auto& p : plist.particles) {
        if (!p.active || p.species == FLASH) continue;

        int i = int(p.x / grid.dx);
        int j = int(p.y / grid.dx);
        if (i < 1 || i >= grid.nx - 1 || j < 1 || j >= grid.ny - 1) {
            p.active = false;
            continue;
        }

        float Ex = -(grid.at_phi(i+1, j) - grid.at_phi(i-1, j)) / (2.0f * grid.dx);
        float Ey = -(grid.at_phi(i, j+1) - grid.at_phi(i, j-1)) / (2.0f * grid.dx);

        float mass = (p.species == ELECTRON) ? E_MASS : NE_MASS;
        float q    = (p.species == ELECTRON) ? -E_CHARGE : E_CHARGE;
        float ax = q * Ex / mass;
        float ay = q * Ey / mass;

        p.vx += ax * dt;
        p.vy += ay * dt;
        p.x  += p.vx * dt;
        p.y  += p.vy * dt;

        if (p.x < 0 || p.x >= grid.nx * grid.dx ||
            p.y < 0 || p.y >= grid.ny * grid.dx) {
            p.active = false;
            continue;
        }

        int ni = int(p.x / grid.dx);
        int nj = int(p.y / grid.dx);
        if (ni < 0 || ni >= grid.nx || nj < 0 || nj >= grid.ny) {
            p.active = false;
            continue;
        }

        uint8_t mask = grid.at_mask(ni, nj);
        if (mask == 2) {
            if (p.species == ELECTRON) {
                // electrons absorbed by glass (surface charges up)
                p.active = false;
            } else {
                // ions reflect off sheath near insulating wall
                p.x -= p.vx * dt;
                p.y -= p.vy * dt;
                float wall_cx = grid.nx * grid.dx * 0.5f;
                float wall_cy = grid.ny * grid.dx * 0.5f;
                float rnx = p.x - wall_cx;
                float rny = p.y - wall_cy;
                float rm = std::sqrt(rnx * rnx + rny * rny);
                if (rm > 0) { rnx /= rm; rny /= rm; }
                float vn = p.vx * rnx + p.vy * rny;
                if (vn > 0) {
                    p.vx -= 2.0f * vn * rnx;
                    p.vy -= 2.0f * vn * rny;
                }
            }
        } else if (mask == 1) {
            // conductor hit
            if (p.species == ELECTRON) {
                float hit_phi = grid.at_phi(ni, nj);
                if (hit_phi > cathode_voltage + 10.0f) {
                    anode_current += p.weight;
                }
            }
            if (p.species == ION) {
                // any electrode hit by ion: check if it's cathode (lower voltage)
                float hit_phi = grid.at_phi(ni, nj);
                if (hit_phi <= cathode_voltage + 10.0f) {
                    // ion hits cathode -> secondary electron emission
                    if (push_randf() < gamma) {
                        Particle se;
                        se.x = p.x; se.y = p.y;
                        float angle = push_randf() * 2.0f * 3.14159265f;
                        float speed = 3.0e5f; // ~0.5 eV
                        se.vx = speed * std::cos(angle);
                        se.vy = speed * std::sin(angle);
                        se.weight = p.weight;
                        se.species = ELECTRON;
                        se.active = true;
                        se.life = -1.0f;
                        secondaries.push_back(se);
                    }
                }
            }
            p.active = false;
        }
    }

    for (auto& s : secondaries) plist.add(s);
    return anode_current;
}
