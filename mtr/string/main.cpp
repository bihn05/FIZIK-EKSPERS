#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>

struct M {
    float x;
    float y;
    float vx;
    float vy;
};
const float dt = 0.5f; 
const float k = 4.0f;
void get_acc(M &a, M &b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;

    float ds = sqrt(dx*dx + dy*dy);

    if (ds < 1e-4)return;

    float f = k * pow(ds, 2) / 28;

    float fx = f * dx / ds;
    float fy = f * dy / ds;

    a.vx += -fx * dt;
    a.vy += -fy * dt;

    b.vx += fx * dt;
    b.vy += fy * dt;
}
void update(M &m) {
    if (fabs(m.vx) < 1e-5)m.vx=0;
    if (fabs(m.vy) < 1e-5)m.vy=0;

    m.x += m.vx * dt;
    m.y += m.vy * dt;
    m.vy += 0.08;
}
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "STR");
    window.setFramerateLimit(120);
    
    std::vector<M> chain;

    float px = 10.0f;
    for (int i = 0; i < 60; i++) {
        M m;
        m.x = px;
        m.y = 200.0f + pow(px - 300, 2) * 0.001f;
        m.vx = 0.0f;
        m.vy = 0.0f;
        chain.push_back(m);

        px += 10.0f;
    }

    bool turn = false;
    bool block = true;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                turn ^= 1;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::O ) {
                block ^= 1;
            }
        }

        if (turn) {
            for (int i = 0; i < chain.size() - 1; i++) {
                get_acc(chain[i], chain[i + 1]);
            }
            chain[0].vx = 0.0f;
            chain[0].vy = 0.0f;
            chain[chain.size() - 1].vx = 0.0f;
            chain[chain.size() - 1].vy = 0.0f;
            for (int i = 0; i < chain.size(); i++) {
                update(chain[i]);
            }
        }

        window.clear();

        for (int i = 0; i < chain.size(); i++) {
            sf::CircleShape circle(1.0f);
            circle.setFillColor(sf::Color::Green);
            circle.setPosition(sf::Vector2f(chain[i].x, chain[i].y));
            window.draw(circle);
        }
        
        window.display();


    }
}