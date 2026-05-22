#pragma once
#include <SFML/Graphics.hpp>
#include "../core/grid.h"
#include "../core/particle.h"
#include <cmath>
#include <optional>

struct TubeView {
    sf::Image img;
    sf::Texture texture;
    sf::Sprite sprite;
    int scale;

    TubeView(int nx, int ny, int scale)
        : img(sf::Vector2u(nx, ny), sf::Color::Black),
          texture(img),
          sprite(texture),
          scale(scale) {
        sprite.setScale(sf::Vector2f(float(scale), float(scale)));
        sprite.setOrigin(sf::Vector2f(nx * 0.5f, ny * 0.5f));
    }

    void set_pos(float x, float y, float rotation_deg) {
        sprite.setPosition(sf::Vector2f(x, y));
        sprite.setRotation(sf::degrees(rotation_deg));
    }

    void render(const Grid2D& grid, const ParticleList& plist) {
        float max_phi = 1.0f;
        for (auto v : grid.phi)
            if (std::abs(v) > max_phi) max_phi = std::abs(v);

        for (int j = 0; j < grid.ny; j++) {
            for (int i = 0; i < grid.nx; i++) {
                float val = grid.phi[j * grid.nx + i] / max_phi;
                uint8_t m = grid.mask[j * grid.nx + i];
                sf::Color c;
                if (m == 2) c = sf::Color(60, 60, 60);
                else if (m == 1) c = sf::Color(180, 180, 180);
                else if (val > 0) c = sf::Color(uint8_t(255 * val), 0, 0);
                else c = sf::Color(0, 0, uint8_t(255 * (-val)));
                img.setPixel(sf::Vector2u(i, j), c);
            }
        }

        for (auto& p : plist.particles) {
            if (!p.active || p.species == ION) continue;
            int px = int(p.x / grid.dx);
            int py = int(p.y / grid.dx);
            if (px >= 0 && px < grid.nx && py >= 0 && py < grid.ny) {
                sf::Color c = (p.species == FLASH) ? sf::Color::White : sf::Color::Cyan;
                img.setPixel(sf::Vector2u(px, py), c);
            }
        }

        texture.update(img);
    }
};
