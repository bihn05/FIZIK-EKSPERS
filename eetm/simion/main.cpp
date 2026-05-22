#include <SFML/Graphics.hpp>
#include <chrono>
#include <string>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include "render/scope.h"
#include "render/schematic.h"

// CD-IS: simplified neon tube model (no PIC)
// Vb = breakdown voltage (100V), Vm = maintain voltage (70V)
struct TubeIS {
    static constexpr float V_BREAK = 100.0f;
    static constexpr float V_MAINTAIN = 70.0f;

    bool conducting = false;
    float current = 0.0f;

    float update(float v_across) {
        if (!conducting) {
            if (v_across >= V_BREAK) conducting = true;
        } else {
            if (v_across < V_MAINTAIN) {
                conducting = false;
                current = 0.0f;
                return 0.0f;
            }
        }
        if (conducting) {
            // tube clamps at V_MAINTAIN, excess drives current through circuit
            // current determined externally; here just return voltage drop
            return V_MAINTAIN;
        }
        return v_across; // not conducting: no current, full voltage across tube
    }
};

int main() {
    // circuit parameters
    constexpr float V_SUPPLY = 150.0f;
    constexpr float R1 = 100000.0f;   // 100k
    constexpr float R2 = 103000.0f;   // 103k (asymmetry breaks symmetry)
    constexpr float R3 = 10000.0f;    // 10k cathode
    constexpr float R4 = 10000.0f;    // 10k cathode
    constexpr float CAP = 0.2e-6f;    // 0.2 uF
    constexpr float DT_CIRCUIT = 1e-6f; // 1us circuit timestep
    constexpr int STEPS_PER_FRAME = 500; // 500us per frame

    constexpr int CIRCUIT_W = 600;
    constexpr int WIN_W = 1100;
    constexpr int WIN_H = 500;
    constexpr int SCOPE_X = CIRCUIT_W;

    TubeIS tube1, tube2;
    float v_cap = 0.0f;
    // start below breakdown - nodes will charge up through R
    float va1 = 0.0f, va2 = 0.0f;
    float i1 = 0.0f, i2 = 0.0f;

    Scope scope1, scope2;

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WIN_W, WIN_H)), "Neon Oscillator (CD-IS)");
    window.setFramerateLimit(60);

    sf::Font font("/usr/share/fonts/gsfonts/NimbusMonoPS-Regular.otf");
    sf::Text stats(font, "", 12);
    stats.setFillColor(sf::Color::Green);
    stats.setPosition(sf::Vector2f(5.f, 5.f));

    float sim_time = 0.0f;
    auto wall_start = std::chrono::steady_clock::now();
    int frame_count = 0;
    float fps = 0.0f;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        for (int s = 0; s < STEPS_PER_FRAME; s++) {
            // charge nodes toward Vs through their resistors
            if (!tube1.conducting) {
                va1 += (V_SUPPLY - va1) * DT_CIRCUIT / (R1 * CAP);
            }
            if (!tube2.conducting) {
                va2 += (V_SUPPLY - va2) * DT_CIRCUIT / (R2 * CAP);
            }

            // check for breakdown
            bool t1_was_off = !tube1.conducting;
            bool t2_was_off = !tube2.conducting;

            float vk1 = tube1.conducting ? i1 * R3 : 0.0f;
            float vk2 = tube2.conducting ? i2 * R4 : 0.0f;
            tube1.update(va1 - vk1);
            tube2.update(va2 - vk2);

            // if a tube just fired, apply capacitor coupling immediately
            // only one tube can fire per timestep
            if (t1_was_off && tube1.conducting) {
                float va1_clamped = (TubeIS::V_MAINTAIN * R1 + V_SUPPLY * R3) / (R1 + R3);
                float drop = va1 - va1_clamped;
                va1 = va1_clamped;
                va2 -= drop; // capacitor couples the drop to other node
                // the drop may extinguish tube2
                tube2.update(va2);
            } else if (t2_was_off && tube2.conducting) {
                float va2_clamped = (TubeIS::V_MAINTAIN * R2 + V_SUPPLY * R4) / (R2 + R4);
                float drop = va2 - va2_clamped;
                va2 = va2_clamped;
                va1 -= drop;
                tube1.update(va1);
            }

            // when conducting, clamp anode voltage
            if (tube1.conducting) {
                float va1_c = (TubeIS::V_MAINTAIN * R1 + V_SUPPLY * R3) / (R1 + R3);
                va1 = va1_c;
                i1 = (V_SUPPLY - va1) / R1;
            } else {
                i1 = 0.0f;
            }
            if (tube2.conducting) {
                float va2_c = (TubeIS::V_MAINTAIN * R2 + V_SUPPLY * R4) / (R2 + R4);
                va2 = va2_c;
                i2 = (V_SUPPLY - va2) / R2;
            } else {
                i2 = 0.0f;
            }

            // re-check extinction after coupling
            vk1 = tube1.conducting ? i1 * R3 : 0.0f;
            vk2 = tube2.conducting ? i2 * R4 : 0.0f;
            tube1.update(va1 - vk1);
            tube2.update(va2 - vk2);

            va1 = std::clamp(va1, 0.0f, V_SUPPLY * 2.0f);
            va2 = std::clamp(va2, 0.0f, V_SUPPLY * 2.0f);
            v_cap = va1 - va2;

            sim_time += DT_CIRCUIT;
        }

        tube1.current = i1;
        tube2.current = i2;

        scope1.push(va1, va2);
        scope2.push(i1 * 1e3f, i2 * 1e3f);

        float glow1 = tube1.conducting ? std::min(1.0f, i1 * 300.0f) : 0.0f;
        float glow2 = tube2.conducting ? std::min(1.0f, i2 * 300.0f) : 0.0f;

        frame_count++;
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - wall_start).count();
        if (elapsed > 0.5f) {
            fps = frame_count / elapsed;
            frame_count = 0;
            wall_start = now;
        }

        char buf[256];
        std::snprintf(buf, sizeof(buf),
            "FPS:%d  t=%.2fms  Vs=%.0fV  C=%.1fuF\n"
            "V1[%s]: Va=%.1fV I=%.3fmA\n"
            "V2[%s]: Va=%.1fV I=%.3fmA  Vc=%.1fV",
            int(fps), sim_time*1e3f, V_SUPPLY, CAP*1e6f,
            tube1.conducting ? "ON" : "off", va1, i1*1e3f,
            tube2.conducting ? "ON" : "off", va2, i2*1e3f, v_cap);
        stats.setString(buf);

        window.clear(sf::Color(10, 10, 10));

        float b1x = 180.f, b2x = 380.f;
        Schematic::Branch branches[2] = {
            {b1x, R1, va1, i1 * 1e3f, 1, glow1},
            {b2x, R2, va2, i2 * 1e3f, 2, glow2},
        };
        Schematic::draw(window, font, V_SUPPLY, branches, 2, CIRCUIT_W, WIN_H);

        // capacitor symbol
        float cap_y = 40.f + 30.f + 50.f + 20.f;
        sf::VertexArray cap_lines(sf::PrimitiveType::Lines);
        float cap_mid = (b1x + b2x) * 0.5f;
        cap_lines.append({{b1x, cap_y}, sf::Color(100,100,100)});
        cap_lines.append({{cap_mid - 8, cap_y}, sf::Color(100,100,100)});
        cap_lines.append({{cap_mid + 8, cap_y}, sf::Color(100,100,100)});
        cap_lines.append({{b2x, cap_y}, sf::Color(100,100,100)});
        cap_lines.append({{cap_mid - 4, cap_y - 10}, sf::Color(200,200,100)});
        cap_lines.append({{cap_mid - 4, cap_y + 10}, sf::Color(200,200,100)});
        cap_lines.append({{cap_mid + 4, cap_y - 10}, sf::Color(200,200,100)});
        cap_lines.append({{cap_mid + 4, cap_y + 10}, sf::Color(200,200,100)});
        window.draw(cap_lines);
        sf::Text clbl(font, "C1 0.2uF", 10);
        clbl.setFillColor(sf::Color(200, 200, 100));
        clbl.setPosition(sf::Vector2f(cap_mid - 20, cap_y - 22));
        window.draw(clbl);

        window.draw(stats);

        scope1.draw_trace(window, float(SCOPE_X), 10.f,
                          scope1.buf_I, sf::Color::Yellow, "Va1 (V)", font);
        scope1.draw_trace(window, float(SCOPE_X), 130.f,
                          scope1.buf_ph, sf::Color(255, 200, 50), "Va2 (V)", font);
        scope2.draw_trace(window, float(SCOPE_X), 260.f,
                          scope2.buf_I, sf::Color::Green, "I1 (mA)", font);
        scope2.draw_trace(window, float(SCOPE_X), 380.f,
                          scope2.buf_ph, sf::Color(100, 255, 100), "I2 (mA)", font);

        window.display();
    }

    return 0;
}
