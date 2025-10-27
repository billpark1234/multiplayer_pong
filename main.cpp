#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

constexpr bool debug = false;

// Note: upward is negative y. rightward is positive x
// I.e., left-up = (0,0)
constexpr int window_width = 800;
constexpr int window_height = 400;
constexpr float player_speed = 5.0f;

/** [out_of_window] returns true if [shape] is out of the window */
template <typename T>
bool out_of_window(const T &shape)
{
    const sf::Vector2f &pos = shape.getPosition();

    if constexpr (std::is_same_v<T, sf::RectangleShape>)
    {
        const sf::Vector2f &size = shape.getSize();
        return pos.x < 0 || pos.x + size.x >= window_width ||
               pos.y < 0 || pos.y + size.y >= window_height;
    }
    else if constexpr (std::is_same_v<T, sf::CircleShape>)
    {
        float radius = shape.getRadius();
        return pos.x - radius < 0 || pos.x + radius >= window_width ||
               pos.y - radius < 0 || pos.y + radius >= window_height;
    }
}

/** [is_colliding] returns a number if a rectangle and circle is colliding
 * -1 -> not colliding
 * 0 -> left
 * 1 -> top
 * 2 -> right
 * 3 -> bottom
 */
int is_colliding(const sf::RectangleShape &rect, const sf::CircleShape &circ)
{
    float cx = circ.getPosition().x;
    float cy = circ.getPosition().y;
    float rx = rect.getPosition().x;
    float ry = rect.getPosition().y;
    float rw = rect.getSize().x;
    float rh = rect.getSize().y;
    float radius = circ.getRadius();

    float testX = cx;
    float testY = cy;
    if (cx < rx)
        testX = rx; // left edge
    else if (cx > rx + rw)
        testX = rx + rw; // right edge

    if (cy < ry)
        testY = ry; // top edge
    else if (cy > ry + rh)
        testY = ry + rh; // bottom edge

    float distX = cx - testX;
    float distY = cy - testY;
    float distance = sqrt((distX * distX) + (distY * distY));

    if (distance <= radius)
    {
        if (cx < rx)
            return 0;
        if (cy < ry)
            return 1;
        if (cx > rx + rw)
            return 2;
        if (cy > ry + rh)
            return 3;
    }
    return -1;
}

inline float dot(const sf::Vector2f &u, const sf::Vector2f &v)
{
    return u.x * v.x + u.y * v.y;
}

inline sf::Vector2f get_reflection(const sf::Vector2f &incoming, const sf::Vector2f &normal)
{
    return incoming - ((2 * dot(incoming, normal)) * normal);
}

void reset(sf::CircleShape &ball)
{
    ball.setPosition(window_width / 2, window_height / 2);
}

void run()
{
    sf::Font font;
    if (!font.loadFromFile("LiberationSans-Regular.ttf"))
    {
        throw std::runtime_error("File not found: LiberationSans-Regular.ttf");
    }

    sf::RenderWindow window(sf::VideoMode(800, 400), "Hello From SFML");
    window.setFramerateLimit(60);

    int p1score = 0;
    int p2score = 0;

    sf::Text p1text;
    p1text.setFont(font);
    p1text.setString(std::to_string(p1score));
    p1text.setPosition(200.0f, 0.0f);
    p1text.setCharacterSize(24);
    p1text.setFillColor(sf::Color::White);

    sf::Text p2text;
    p2text.setFont(font);
    p2text.setString(std::to_string(p2score));
    p2text.setPosition(600.0f, 0.0f);
    p2text.setCharacterSize(24);
    p2text.setFillColor(sf::Color::White);

    sf::CircleShape ball(15.f);
    ball.setOrigin(15.0f, 15.0f);
    ball.setPosition(window_width / 2, window_height / 2);
    const float radius = ball.getRadius();

    sf::Vector2f velocity(0.0f, 0.0f);
    velocity.x = 1.0f;
    velocity.y = 1.0f;

    sf::RectangleShape player1(sf::Vector2f(20.0f, 100.0f));
    sf::RectangleShape player2(sf::Vector2f(20.0f, 100.0f));
    player2.setPosition(780, 0);

    bool inFocus = false;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::GainedFocus)
            {
                inFocus = true;
            }
            else if (event.type == sf::Event::LostFocus)
            {
                inFocus = false;
            }
        }

        ball.move(velocity);

        if (inFocus)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
                player1.move(0.0f, -player_speed);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                player1.move(0.0f, player_speed);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
                player2.move(0.0f, -player_speed);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
                player2.move(0.0f, player_speed);
        }

        const sf::Vector2f &pos = ball.getPosition();

        if (pos.x + radius < 0) // we count score if ball completely disappears.
        {
            p2score++;
            p2text.setString(std::to_string(p2score));
            reset(ball);
        }

        if (pos.x - radius >= window_width)
        {
            p1score++;
            p1text.setString(std::to_string(p1score));
            reset(ball);
        }

        if (pos.y + radius >= window_height) // Bottom wall
            velocity = get_reflection(velocity, sf::Vector2f(0.0f, -1.0f));

        if (pos.y - radius < 0) // Top wall
            velocity = get_reflection(velocity, sf::Vector2f(0.0f, 1.0f));

        int collision_p1 = is_colliding(player1, ball);
        int collision_p2 = is_colliding(player2, ball);

        if (collision_p1 == 0 || collision_p2 == 0)
            velocity = get_reflection(velocity, sf::Vector2f(-1.0f, 0.0f));

        if (collision_p1 == 1 || collision_p2 == 1)
            velocity = get_reflection(velocity, sf::Vector2f(0.0f, -1.0f));

        if (collision_p1 == 2 || collision_p2 == 2)
            velocity = get_reflection(velocity, sf::Vector2f(1.0f, 0.0f));

        if (collision_p1 == 3 || collision_p2 == 3)
            velocity = get_reflection(velocity, sf::Vector2f(0.0f, 1.0f));

        window.clear();
        window.draw(ball);
        window.draw(player1);
        window.draw(player2);
        window.draw(p1text);
        window.draw(p2text);
        window.display();
    }
}

/** run_debug allows you to control the ball using YGHJ */
void run_debug()
{
    sf::RenderWindow window(sf::VideoMode(800, 400), "Hello From SFML");
    window.setFramerateLimit(60);
    int p1score = 0;
    int p2score = 0;
    sf::CircleShape ball(15.f);
    const float radius = ball.getRadius();
    ball.setOrigin(15.0f, 15.0f);
    ball.setPosition(window_width / 2, window_height / 2);
    sf::Vector2f velocity(0.0f, 0.0f);
    sf::RectangleShape player1(sf::Vector2f(20.0f, 100.0f));
    sf::RectangleShape player2(sf::Vector2f(20.0f, 100.0f));
    player2.setPosition(780, 0);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // ball.move(velocity);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        {
            player1.move(0.0f, -player_speed);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        {
            player1.move(0.0f, player_speed);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        {
            player2.move(0.0f, -player_speed);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        {
            player2.move(0.0f, player_speed);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Y))
        {
            ball.move(0.0f, -1.0f);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H))
        {
            ball.move(0.0f, 1.0f);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G))
        {
            ball.move(-1.0f, 0.0f);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
        {
            ball.move(1.0f, 0.0f);
        }

        const sf::Vector2f &pos = ball.getPosition();

        if (pos.y + radius >= window_height) // Bottom wall
        {
            velocity = get_reflection(velocity, sf::Vector2f(0.0f, -1.0f));
        }
        if (pos.y - radius < 0) // Top wall
        {
            velocity = get_reflection(velocity, sf::Vector2f(0.0f, 1.0f));
        }

        int collision_p1 = is_colliding(player1, ball);
        int collision_p2 = is_colliding(player2, ball);

        if (collision_p1 != -1)
        {
            std::cout << "p1: " << collision_p1 << std::endl;
        }

        if (collision_p2 != -1)
        {
            std::cout << "p2: " << collision_p2 << std::endl;
        }

        if (pos.x + radius < 0) // we count score if ball completely disappears.
        {
            p2score++;
            reset(ball);
        }

        if (pos.x - radius >= window_width)
        {
            p1score++;
            reset(ball);
        }

        window.clear();
        window.draw(ball);
        window.draw(player1);
        window.draw(player2);
        window.display();
    }
}

int main(int argc, char const *argv[])
{
    if (debug)
        run_debug();
    else
        run();
    return EXIT_SUCCESS;
}