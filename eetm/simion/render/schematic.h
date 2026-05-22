#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cstdio>

struct Schematic {
    struct Branch {
        float x;          // center x of this branch
        float r_ohm;
        float v_anode;
        float i_ma;
        int id;           // branch number
        float glow;       // 0.0~1.0 normalized photon intensity
    };

    static void draw(sf::RenderWindow& win, const sf::Font& font,
                     float v_supply, const Branch* branches, int n_branches,
                     float area_w, float area_h) {
        sf::Color wire(100, 100, 100);
        sf::Color comp_r(200, 180, 80);
        sf::Color comp_t(140, 180, 220);
        sf::Color lbl_c(160, 160, 160);

        sf::VertexArray lines(sf::PrimitiveType::Lines);
        auto line = [&](float x1, float y1, float x2, float y2, sf::Color c) {
            lines.append({{x1, y1}, c});
            lines.append({{x2, y2}, c});
        };

        // layout: top=V+, bottom=GND, vertical branches
        float rail_top = 40.f;
        float rail_bot = area_h - 40.f;
        float left_margin = 40.f;
        float right_margin = area_w - 20.f;

        // horizontal rails
        line(left_margin, rail_top, right_margin, rail_top, wire);
        line(left_margin, rail_bot, right_margin, rail_bot, wire);

        // V+ and GND symbols
        // V+ : circle with +
        float vx = left_margin - 15;
        line(vx, rail_top - 8, vx, rail_top + 8, sf::Color(150, 150, 255));
        line(vx - 6, rail_top, vx + 6, rail_top, sf::Color(150, 150, 255));
        // GND: three lines
        line(left_margin - 20, rail_bot, left_margin - 10, rail_bot, wire);
        line(left_margin - 18, rail_bot + 3, left_margin - 12, rail_bot + 3, wire);
        line(left_margin - 16, rail_bot + 6, left_margin - 14, rail_bot + 6, wire);

        for (int b = 0; b < n_branches; b++) {
            float bx = branches[b].x;
            float r_y1 = rail_top + 30;
            float r_y2 = r_y1 + 50;
            float t_y1 = r_y2 + 40;
            float t_y2 = t_y1 + 60;

            // wire from top rail down to resistor
            line(bx, rail_top, bx, r_y1, wire);

            // resistor (vertical zigzag)
            int segs = 6;
            float seg_h = (r_y2 - r_y1) / segs;
            for (int i = 0; i < segs; i++) {
                float y1 = r_y1 + i * seg_h;
                float y2 = y1 + seg_h;
                float x1 = bx + ((i % 2 == 0) ? -5.f : 5.f);
                float x2 = bx + ((i % 2 == 0) ? 5.f : -5.f);
                line(x1, y1, x2, y2, comp_r);
            }

            // wire from resistor to tube
            line(bx, r_y2, bx, t_y1, wire);

            // tube symbol: circle with glow fill
            float tcx = bx, tcy = (t_y1 + t_y2) * 0.5f;
            float tr = (t_y2 - t_y1) * 0.4f;

            // filled glow circle (orange-red, brightness = photon intensity)
            float g = branches[b].glow;
            if (g > 1.0f) g = 1.0f;
            sf::CircleShape glow_circle(tr);
            glow_circle.setOrigin(sf::Vector2f(tr, tr));
            glow_circle.setPosition(sf::Vector2f(tcx, tcy));
            uint8_t rr = uint8_t(255 * g);
            uint8_t gg = uint8_t(80 * g);
            uint8_t bb = uint8_t(20 * g);
            glow_circle.setFillColor(sf::Color(rr, gg, bb));
            glow_circle.setOutlineColor(comp_t);
            glow_circle.setOutlineThickness(1.f);
            win.draw(glow_circle);

            // anode line (top)
            line(bx, t_y1, bx, tcy - tr, comp_t);
            // cathode line (bottom)
            line(bx, tcy + tr, bx, t_y2, comp_t);
            // internal: small anode plate
            line(bx - 6, tcy - 5, bx + 6, tcy - 5, sf::Color(255, 100, 100));
            // internal: cathode bar
            line(bx - 8, tcy + 8, bx + 8, tcy + 8, sf::Color(100, 200, 100));
            line(bx - 8, tcy + 8, bx - 8, tcy + 12, sf::Color(100, 200, 100));
            line(bx + 8, tcy + 8, bx + 8, tcy + 12, sf::Color(100, 200, 100));

            // wire from tube to cathode resistor
            float rk_y1 = t_y2 + 15;
            float rk_y2 = rk_y1 + 40;
            line(bx, t_y2, bx, rk_y1, wire);

            // cathode resistor (vertical zigzag)
            for (int i = 0; i < segs; i++) {
                float ry1 = rk_y1 + i * (rk_y2 - rk_y1) / segs;
                float ry2 = ry1 + (rk_y2 - rk_y1) / segs;
                float rx1 = bx + ((i % 2 == 0) ? -4.f : 4.f);
                float rx2 = bx + ((i % 2 == 0) ? 4.f : -4.f);
                line(rx1, ry1, rx2, ry2, comp_r);
            }

            // wire from cathode resistor to bottom rail
            line(bx, rk_y2, bx, rail_bot, wire);

            // store tube center for sprite placement
            // (caller uses branches[b].tube_cx/cy)

            win.draw(lines);
            lines.clear();

            // labels
            char buf[64];
            auto label = [&](float x, float y, const std::string& s, sf::Color c) {
                sf::Text t(font, s, 10);
                t.setFillColor(c);
                t.setPosition(sf::Vector2f(x, y));
                win.draw(t);
            };

            // resistor label
            std::snprintf(buf, sizeof(buf), "R%d", branches[b].id);
            label(bx + 10, r_y1, buf, lbl_c);
            std::snprintf(buf, sizeof(buf), "%.0fk", branches[b].r_ohm / 1000.f);
            label(bx + 10, r_y1 + 12, buf, comp_r);

            // tube label
            std::snprintf(buf, sizeof(buf), "V%d", branches[b].id);
            label(bx + tr + 4, tcy - 8, buf, lbl_c);

            // values
            std::snprintf(buf, sizeof(buf), "%.1fV", branches[b].v_anode);
            label(bx + tr + 4, tcy + 4, buf, sf::Color::Yellow);
            std::snprintf(buf, sizeof(buf), "%.2fmA", branches[b].i_ma);
            label(bx + 10, r_y1 + 24, buf, sf::Color::Green);

            // cathode resistor label
            std::snprintf(buf, sizeof(buf), "R%d", branches[b].id + 2);
            label(bx + 8, rk_y1, buf, lbl_c);
            label(bx + 8, rk_y1 + 12, "10k", comp_r);
        }

        // V supply label
        char buf[64];
        std::snprintf(buf, sizeof(buf), "V+ %.0fV", v_supply);
        sf::Text vt(font, buf, 10);
        vt.setFillColor(sf::Color(150, 150, 255));
        vt.setPosition(sf::Vector2f(left_margin + 5, rail_top - 15));
        win.draw(vt);

        sf::Text gt(font, "COM", 10);
        gt.setFillColor(sf::Color(100, 100, 100));
        gt.setPosition(sf::Vector2f(left_margin + 5, rail_bot + 3));
        win.draw(gt);
    }
};
