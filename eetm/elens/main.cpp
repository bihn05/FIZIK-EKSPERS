#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#define IS_INSIDE(x, min, max) ((x) >= (min) && (x) <= (max))

float dt = 0.05f;
float v0 = 100;
int tms = 10000;

struct E {
    float x;
    float y;
    float vx;
    float vy;
    bool active;
};
struct L {
    float y;
    float l;
    float r;
    float k;
};
float get_acc(E *e, L &len) {
    float a = - len.k * e->x;
    return a;
}
void simulator(E *e, L &len) {
    if (IS_INSIDE(e->y, len.y, len.y + len.l)) {
        if (IS_INSIDE(e->x, -len.r, len.r)) {
            e->vx += get_acc(e, len) * dt;
        } else {
            e->active = false;
        }
    }
    e->x += e->vx * dt;
    e->y += e->vy * dt;
}
void draw_len(L &len, sf::RenderWindow &w) {
    sf::VertexArray lines(sf::LineStrip);
    lines.resize(2);
    lines[0].position = sf::Vector2f(400 + len.r, 600 - len.y);
    lines[0].color = sf::Color::White;
    lines[1].position = sf::Vector2f(400 + len.r, 600 - len.y - len.l);
    lines[1].color = sf::Color::White;
    w.draw(lines);
    lines[0].position = sf::Vector2f(400 - len.r, 600 - len.y);
    lines[0].color = sf::Color::White;
    lines[1].position = sf::Vector2f(400 - len.r, 600 - len.y - len.l);
    lines[1].color = sf::Color::White;
    w.draw(lines);
}
// 生成电子的函数 - 方向向上，扇形展开
std::vector<E> generateElectrons(float sourceX, float sourceY, 
                                        float spreadAngle = 60.0f,  // 总张角（度）
                                        int count = 19) {           // 电子数量
    std::vector<E> electrons;
    
    // 将角度转换为弧度
    float spreadRad = spreadAngle * M_PI / 180.0f;
    
    // 中心方向是向上（-90° 或 270°，因为屏幕Y轴向下）
    // 在SFML中，向上是负Y方向，所以角度是 -90° 或 270°
    float centerAngle = M_PI/2;  // -90度，指向屏幕上方
    
    // 计算起始角度和步长
    float startAngle = centerAngle - spreadRad/2;
    float angleStep = spreadRad / (count - 1);
    
    for (int i = 0; i < count; ++i) {
        float angle = startAngle + i * angleStep;
        
        E e;
        e.x = sourceX;
        e.y = sourceY;
        e.vx = v0 * cos(angle);    // 水平分量
        e.vy = v0 * sin(angle);    // 垂直分量（负值表示向上）
        e.active = true;
        
        electrons.push_back(e);
    }
    
    return electrons;
}
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "ELEN");
    sf::Image img;
    img.create(800, 600, sf::Color::Black);

    std::vector<E> es;
    es = generateElectrons(0.0f, 0.0f, 15.0f, 200);
    /*
    for (float i = 0; i < 360; ++i) {
        float angle = i * (M_PI / 360);
        E te;
        te.x = 0;
        te.y = 50;
        te.vx = v0 * cosf(angle);
        te.vy = v0 * sinf(angle);
        te.active = true;
        es.push_back(te);
    }*/

    std::vector<L> lens;

    L len1;
    len1.k = 2.0f;
    len1.l = 180;
    len1.r = 15;
    len1.y = 80;

    lens.push_back(len1);

    len1.k = -17.0f;
    len1.l = 50;
    len1.r = 30;
    len1.y = 300;

    lens.push_back(len1);

    std::cout<<"LENS COUNT = "<<lens.size()<<std::endl;
    std::cout<<"ELECTRONS COUNT = "<<es.size()<<std::endl;

    for (int i = 0; i < es.size(); i++) {
        for (int j = 0; j <= tms; j++) {
            if (es[i].active) {
                E te = es[i];
                for (int k = 0; k < lens.size(); k++) {
                    simulator(&te, lens[k]);
                }
                es[i] = te;
                int x = 400 + int(es[i].x);
                int y = 600 - int(es[i].y);
                if ((x >= 0) && (x <= 800) && (y >= 0) && (y <= 600))
                    img.setPixel(x, y, sf::Color::Green);
            }
        }
    }
    int count = 0;
    for (int i=0;i<es.size();i++) {
        if (es[i].active)count++;
    }

    std::cout<<"ENERGY LOSS = "<<(1.0 - 1.0 * count / es.size())<<std::endl;

    sf::Texture texture;
    texture.loadFromImage(img);
    sf::Sprite sprite(texture);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(sprite);
        for (int i = 0; i < lens.size(); i++) {
            draw_len(lens[i], window);
        }
        window.display();
    }

    return 0;
}