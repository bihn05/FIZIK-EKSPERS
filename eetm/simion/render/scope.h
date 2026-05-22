#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <algorithm>
#include <cmath>

struct Scope {
    static constexpr int WIDTH = 490;
    static constexpr int HEIGHT = 110;
    static constexpr int HISTORY = 490;

    float buf_I[HISTORY] = {};
    float buf_ph[HISTORY] = {};
    int head = 0;

    void push(float current, float photons) {
        buf_I[head] = current;
        buf_ph[head] = photons;
        head = (head + 1) % HISTORY;
    }

    void draw_trace(sf::RenderWindow& win, float x0, float y0,
                    float* buf, sf::Color color, const std::string& label,
                    const sf::Font& font) {
        // background
        sf::RectangleShape bg(sf::Vector2f(WIDTH, HEIGHT));
        bg.setPosition(sf::Vector2f(x0, y0));
        bg.setFillColor(sf::Color(15, 15, 15));
        bg.setOutlineColor(sf::Color(40, 40, 40));
        bg.setOutlineThickness(1.f);
        win.draw(bg);

        // grid lines
        sf::VertexArray grid(sf::PrimitiveType::Lines);
        for (int i = 1; i < 5; i++) {
            float y = y0 + HEIGHT * i / 5.0f;
            grid.append({{x0, y}, sf::Color(30, 30, 30)});
            grid.append({{x0 + WIDTH, y}, sf::Color(30, 30, 30)});
        }
        for (int i = 1; i < 5; i++) {
            float x = x0 + WIDTH * i / 5.0f;
            grid.append({{x, y0}, sf::Color(30, 30, 30)});
            grid.append({{x, y0 + HEIGHT}, sf::Color(30, 30, 30)});
        }
        win.draw(grid);

        // find max for auto-scale
        float max_val = 1.0f;
        for (int i = 0; i < HISTORY; i++) {
            if (buf[i] > max_val) max_val = buf[i];
        }

        // trace
        sf::VertexArray line(sf::PrimitiveType::LineStrip, HISTORY);
        for (int i = 0; i < HISTORY; i++) {
            int idx = (head + i) % HISTORY;
            float x = x0 + float(i);
            float y = y0 + HEIGHT - (buf[idx] / max_val) * (HEIGHT - 20);
            line[i] = {{x, y}, color};
        }
        win.draw(line);

        // label
        sf::Text lbl(font, label, 12);
        lbl.setFillColor(color);
        lbl.setPosition(sf::Vector2f(x0 + 5, y0 + 3));
        win.draw(lbl);

        // max value
        sf::Text maxlbl(font, std::to_string(int(max_val)), 11);
        maxlbl.setFillColor(sf::Color(100, 100, 100));
        maxlbl.setPosition(sf::Vector2f(x0 + WIDTH - 60, y0 + 3));
        win.draw(maxlbl);
    }
};
