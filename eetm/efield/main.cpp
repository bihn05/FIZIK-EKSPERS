#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#define IS_INSIDE(x, min, max) ((x) >= (min) && (x) <= (max))

#define ROOM_W 800
#define ROOM_H 600

#define VOLT_MAX 1000 // v_max = 1000 V

float v[ROOM_W + 2][ROOM_H + 2];
float v_past[ROOM_W + 2][ROOM_H + 2];

struct PLATE {
    float x;
    float y;
    float l; // length
    float a; // thickness
    float d; // direction
    float v; // voltage
};

void clear() {
    for (int i = 0; i < ROOM_W + 2; i++) {
        for (int j = 0; j < ROOM_H + 2; j++) {
            v[i][j] = 0.0f;
            v_past[i][j] = 0.0f;
        }
    }
}

void v_add_plate(const PLATE &p) {
    float px_l = p.l;
    float px_a = p.a;
    float rad = p.d * 3.14159265f / 180.0f; // 角度转弧度

    float cos_d = cos(rad);
    float sin_d = sin(rad);

    float bound = sqrt(px_l * px_l + px_a * px_a) / 2.0f;
    
    int x_start = std::max(1, (int)(p.x - bound));
    int x_end = std::min(ROOM_W, (int)(p.x + bound));
    int y_start = std::max(1, (int)(p.y - bound));
    int y_end = std::min(ROOM_H, (int)(p.y + bound));

    for (int i = x_start; i <= x_end; i++) {
        for (int j = y_start; j <= y_end; j++) {
            float dx = i - p.x;
            float dy = j - p.y;

            float local_x = dx * cos_d + dy * sin_d;
            float local_y = -dx * sin_d + dy * cos_d;

            if (std::abs(local_x) <= px_l / 2.0f && std::abs(local_y) <= px_a / 2.0f) {
                v_past[i][j] = p.v;
            }
        }
    }
}
void v_transfer() {
    for (int i = 1; i < ROOM_W + 1; i++) {
        for (int j = 1; j < ROOM_H + 1; j++) {
            float temp = v_past[i-1][j] + v_past[i+1][j] + v_past[i][j-1] + v_past[i][j+1];
            v[i][j] = temp / 4.0f;
        }
    }
        
    for (int i = 1; i < ROOM_W + 1; i++) {
        for (int j = 1; j < ROOM_H + 1; j++) {
            v_past[i][j] = v[i][j];
        }
    }
}

sf::Color v_color(float v) {
    float normalized = v / VOLT_MAX;
    
    if (v > 0) {
        return sf::Color(int(255 * normalized), 0, 0);
    }
    else if (v < 0) {
        normalized = -normalized;
        return sf::Color(0, 0, int(255 * normalized));
    }
    return sf::Color(0, 0, 0);
}

void draw_e_vector(int x, int y, sf::RenderWindow &w) {
    float dv_x = v_past[x + 1][y] - v_past[x - 1][y];
    float dv_y = v_past[x][y + 1] - v_past[x][y - 1];

    float mag = std::sqrt(dv_x * dv_x + dv_y * dv_y);

    float visual_length = 10.0f; 
    sf::Vector2f start_pos(static_cast<float>(x), static_cast<float>(y));
    sf::Vector2f end_pos;

    sf::Color line_color;

    if (mag > 0.001f) {
        float dir_x = dv_x / mag;
        float dir_y = dv_y / mag;

        end_pos.x = start_pos.x + dir_x * visual_length;
        end_pos.y = start_pos.y + dir_y * visual_length;

        float normalized_mag = std::min(1.0f, mag / 50.0f);
        sf::Uint8 r = static_cast<sf::Uint8>(255 * normalized_mag);
        sf::Uint8 b = static_cast<sf::Uint8>(255 * (1.0f - normalized_mag));
        line_color = sf::Color(0xff, 0xff, 0xff, 0x7f);
    } else {
        return; 
    }

    sf::VertexArray line(sf::Lines, 2);
    line[0].position = start_pos;
    line[0].color = line_color;
    line[1].position = end_pos;
    line[1].color = line_color;

    w.draw(line);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "EF");
    sf::Image img;
    sf::Texture texture;
    sf::Sprite sprite;

    window.setFramerateLimit(30);

    img.create(800, 600, sf::Color::Black);
    
    clear();
    PLATE p1;
    p1.a = 1.0f;
    p1.d = 45.0f;
    p1.l = 150.0f;
    p1.v = 800.0f;
    p1.x = 350.0f;
    p1.y = 300.0f;

    PLATE p2;
    p2.a = 1.0f;
    p2.d = 90.0f;
    p2.l = 300.0f;
    p2.v = -800.0f;
    p2.x = 450.0f;
    p2.y = 300.0f;

    int count = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        for (int t = 0; t < 400; t++) {
            v_add_plate(p1);
            v_add_plate(p2);
            v_transfer();
        }

        for (int i = 0; i < ROOM_W; i++) {
            for (int j = 0; j < ROOM_H; j++) {
                img.setPixel(i, j, v_color(v[i][j]));
            }
        }

        texture.loadFromImage(img);
        sprite.setTexture(texture);

        window.clear();
        window.draw(sprite);

        for (int i = 0; i < ROOM_W; i+=20) {
            for (int j = 0; j < ROOM_H; j+=20) {
                draw_e_vector(i + 11, j + 11, window);
            }
        }

        window.display();

    }

    return 0;
}