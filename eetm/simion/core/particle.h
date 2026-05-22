#pragma once
#include <vector>
#include <algorithm>

enum Species { ELECTRON = 0, ION = 1, FLASH = 2 };

struct Particle {
    float x, y;
    float vx, vy;
    float weight;
    Species species;
    bool active;
    float life; // remaining lifetime (s), <0 means infinite
};

struct ParticleList {
    std::vector<Particle> particles;

    void add(const Particle& p) {
        particles.push_back(p);
    }

    void remove_inactive() {
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return !p.active; }),
            particles.end());
    }

    void decay(float dt) {
        for (auto& p : particles) {
            if (!p.active || p.life < 0) continue;
            p.life -= dt;
            if (p.life <= 0) p.active = false;
        }
    }

    size_t size() const { return particles.size(); }
};
