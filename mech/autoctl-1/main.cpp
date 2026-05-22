#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>

#define IS_INSIDE(x, min, max) ((x) >= (min) && (x) <= (max))

#define ROOM_W 800
#define ROOM_H 600

bool ctrl_bit[10]; // mcu output
bool output_bit[10]; // 1 = +12V, 0 = GND

struct signal {
    bool *posi;
    bool *nega;
    int status() {
        if (*posi == *nega)return 0;
        if (*posi)return 1;
        if (*nega)return -1;
        return 0;
    }
};
struct blade {
    float pos;
    signal sig;
};
blade bld;
struct transfer {
    signal sig;
};
transfer tsf;
struct uvlight {
    signal sig;
};
uvlight uv;

struct sensor {
    float pos;
    bool on;
};
sensor photo_1; // photo
sensor photo_2;
sensor pos_sw_1; // switch start
sensor pos_sw_2; // switch end
sensor wet_1; // wet sensor

bool ground = 0;

void update() {
    for (int i = 0; i < 10; i++) {
        if ((i % 2) == 0) {
            output_bit[i] = !ctrl_bit[i];
        } else {
            output_bit[i] = ctrl_bit[i];
        }
    }

    if (bld.pos < 0.f)bld.pos = 0.f;

    photo_1.on = 0;
    photo_2.on = 0;
    pos_sw_1.on = 0;
    pos_sw_2.on = 0;
    if (IS_INSIDE(bld.pos, photo_1.pos - 2, photo_1.pos + 2)) {
        photo_1.on = 1;
    }
    if (IS_INSIDE(bld.pos, photo_2.pos - 2, photo_2.pos + 2)) {
        photo_2.on = 1;
    }
    if (IS_INSIDE(bld.pos, pos_sw_1.pos, pos_sw_1.pos + 10)) {
        pos_sw_1.on = 1;
    }
    if (IS_INSIDE(bld.pos, pos_sw_2.pos, pos_sw_2.pos + 10)) {
        pos_sw_2.on = 1;
    }
}
void move() {
    if (bld.sig.status() == 1) {
        bld.pos += 3.f;
    } else if (bld.sig.status() == -1) {
        bld.pos -= 3.f;
    }
}
void draw_status(sf::RenderWindow &w) {
    sf::CircleShape led;
    led.setRadius(3.f);
    for (int i = 0; i < 10; i++) {
        if (ctrl_bit[i]) {
            led.setFillColor(sf::Color::Red);
        } else {
            led.setFillColor(sf::Color(40, 0, 0));
        }
        led.setPosition(500.f + i * 20, 50.f);
        w.draw(led);

        if (output_bit[i]) {
            led.setFillColor(sf::Color::Red);
        } else {
            led.setFillColor(sf::Color(40, 0, 0));
        }
        led.setPosition(500.f + i * 20, 120.f);
        w.draw(led);
    }

    if (photo_1.on) {
        led.setFillColor(sf::Color::Green);
    } else {
        led.setFillColor(sf::Color(20, 30, 0));
    }
    led.setPosition(500.f, 160.f);
    w.draw(led);

    if (photo_2.on) {
        led.setFillColor(sf::Color::Green);
    } else {
        led.setFillColor(sf::Color(20, 30, 0));
    }
    led.setPosition(530.f, 160.f);
    w.draw(led);

    if (pos_sw_1.on) {
        led.setFillColor(sf::Color::Green);
    } else {
        led.setFillColor(sf::Color(20, 30, 0));
    }
    led.setPosition(560.f, 160.f);
    w.draw(led);

    if (pos_sw_2.on) {
        led.setFillColor(sf::Color::Green);
    } else {
        led.setFillColor(sf::Color(20, 30, 0));
    }
    led.setPosition(590.f, 160.f);
    w.draw(led);
}
void draw_base(sf::RenderWindow &w) {
    sf::RectangleShape base;
    base.setPosition(50.f, 220.f);
    base.setSize(sf::Vector2f(500.f, 20.f));
    base.setFillColor(sf::Color::Transparent);
    base.setOutlineColor(sf::Color::White);
    base.setOutlineThickness(1.f);

    w.draw(base);

    sf::VertexArray line(sf::Lines, 2);
    line[0].position = sf::Vector2f(50.f + bld.pos, 220.f);
    line[0].color = sf::Color::White;
    line[1].position = sf::Vector2f(50.f + bld.pos, 250.f);
    line[1].color = sf::Color::White;

    w.draw(line);

    sf::RectangleShape ssor;
    ssor.setPosition(50.f + photo_1.pos - 3, 200.f);
    ssor.setSize(sf::Vector2f(6, 30.f));
    ssor.setFillColor(sf::Color::Red);
    w.draw(ssor);
    ssor.setPosition(50.f + photo_2.pos - 3, 200.f);
    ssor.setSize(sf::Vector2f(6, 30.f));
    ssor.setFillColor(sf::Color::Red);
    w.draw(ssor);
    ssor.setPosition(50.f + pos_sw_1.pos, 217.f);
    ssor.setSize(sf::Vector2f(10, 3));
    ssor.setFillColor(sf::Color::White);
    w.draw(ssor);
    ssor.setPosition(50.f + pos_sw_2.pos, 217.f);
    ssor.setSize(sf::Vector2f(10, 3));
    ssor.setFillColor(sf::Color::White);
    w.draw(ssor);

    sf::RectangleShape uvl;
    uvl.setPosition(500.f, 300.f);
    uvl.setSize(sf::Vector2f(5.f, 50.f));
    if (*uv.sig.posi) {
        uvl.setFillColor(sf::Color(0, 255, 255));
    } else {
        uvl.setFillColor(sf::Color(0, 10, 3));
    }
    w.draw(uvl);
}

/** bits:
 * 0, 1 - blade X azis
 * 2, 3 - press Z azis
 * 4, 5 - reserved if any relay broke
 * 6, G - reserved
 * 7, G - PWM0 POWER
 * 8, G - transfer
 * 9, G - UV light
 * PWM0 - door Y rotate
 * */
// codes down here must be c code for stc89c52rc
int status = 0;
int timer = 0;
bool time_en = 0; // enable timer
bool mask[10] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
#define PWR_ON 1
#define PWR_OFF 0
// 0 = init, to zero
// 1 = to zero, but blade in start region
// 2 = to zero, but blade in other region
// 3 = complete to zero
// 4 = open door, start timer
// 5 = if it is time, stop time, close door
// 6 = transfer running, until wet-sensor-I on
void init() {
    for (int i = 0; i < 10; i++) {
        ctrl_bit[i] = PWR_OFF ^ mask[i];
    }
    status = 0;
    timer = 0;
}
void time_add() {
    if (time_en)timer++;
}
void time_start() {
    time_en = 1;
    timer = 0;
}
void time_stop() {
    time_en = 0;
    timer = 0;
}
bool is_time() {
    if (timer >= 100) return 1;
    return 0;
}
void process() {
    std::cout << status;
    if (time_en) {
        std::cout << " timer=" << timer << std::endl;
    } else {
        std::cout << std::endl;
    }
    time_add();
    switch (status) {
        case 0: {
            if (pos_sw_1.on == 1) {
                status = 1;
                break;
            }
            status = 2;
            break;
        }
        case 1: {
            ctrl_bit[0] = PWR_ON ^ mask[0];
            ctrl_bit[1] = PWR_OFF ^ mask[1];
            if (!pos_sw_1.on) {
                ctrl_bit[0] = PWR_OFF ^ mask[0];
                ctrl_bit[1] = PWR_OFF ^ mask[1];
                status = 3;
            }
            break;
        }
        case 2: {
            ctrl_bit[0] = PWR_OFF ^ mask[0];
            ctrl_bit[1] = PWR_ON ^ mask[1];
            if (pos_sw_1.on) {
                ctrl_bit[0] = PWR_OFF ^ mask[0];
                ctrl_bit[1] = PWR_OFF ^ mask[1];
                status = 3;
            }
            break;
        }
        case 3: { // maybe a buffer zone
            status = 4;
            // might beep?
            break;
        }
        case 4: {
            ctrl_bit[7] = PWR_ON ^ mask[7];
            // pwm to open_angle
            time_start();
            status = 5;
            break;
        }
        case 5: {
            if (is_time()) {
                ctrl_bit[7] = PWR_OFF ^ mask[7];
                status = 6;
                time_stop();
                break;
            }
            break;
        }
        case 6: {

        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "AUTOCTL-1");
    window.setFramerateLimit(30);

    bld.sig.posi = &output_bit[0];
    bld.sig.nega = &output_bit[1];
    bld.pos = 200.f;

    uv.sig.posi = &output_bit[9];
    uv.sig.nega = &ground;

    photo_1.pos = 30.f;
    photo_2.pos = 450.f;
    pos_sw_1.pos = 0.f;
    pos_sw_2.pos = 490.f;

    for (int i = 0; i < 10; i++) {
        ctrl_bit[i] = 1; // mcu would init them all high
    }

    init(); // soft init

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear();

        update();
        process();
        move();

        draw_status(window);
        draw_base(window);
        window.display();
    }
}