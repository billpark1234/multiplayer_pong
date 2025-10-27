#include <SFML/Network.hpp>
#include <cmath>
#include <iostream>
#include <thread>
#include <atomic>

struct Ball
{
    float radius;
    float x, y, vx, vy;
    void move()
    {
        x += vx;
        y += vy;
    };
    void setPos(float x, float y)
    {
        this->x = x;
        this->y = y;
    }
};

struct Paddle
{
    float width, height, x, y;
    void move(float dx, float dy)
    {
        x += dx;
        y += dy;
    }
};

int is_colliding(Paddle &rect, Ball &ball)
{
    float cx = ball.x;
    float cy = ball.y;
    float rx = rect.x;
    float ry = rect.y;
    float rw = rect.width;
    float rh = rect.height;
    float radius = ball.radius;

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

int main()
{
    sf::Socket::Status status;
    sf::TcpListener listener;

    if (listener.listen(5000) != sf::Socket::Done)
    {
        std::cerr << "Listen failed";
        return EXIT_FAILURE;
    }

    sf::TcpSocket client1;
    if (listener.accept(client1) != sf::Socket::Done)
    {
        std::cerr << "Accept failed for client2";
        return EXIT_FAILURE;
    }
    // sf::TcpSocket client2;
    // if (listener.accept(client2) != sf::Socket::Done)
    // {
    //     std::cerr << "Accept failed for client2";
    //     return EXIT_FAILURE;
    // }

    bool running = true;
    constexpr int window_width = 800;
    constexpr int window_height = 400;
    constexpr float player_speed = 5.0f;

    int p1score = 0;
    int p2score = 0;

    Ball ball{15.0f, window_width / 2, window_height / 2, 1.0f, 1.0f};
    Paddle p1{20.0f, 100.0f, 0.0f, 0.0f};
    Paddle p2{20.0f, 100.0f, 780.0f, 0.0f};

    sf::SocketSelector selector;
    sf::Packet send_packet;
    selector.add(client1);

    // Moving ball and sending packets to the clients
    while (running)
    {
        ball.move();
        p1.move(0.0f, 0.0f);

        if (ball.x + ball.radius < 0) // we count score if ball completely disappears.
        {
            p2score++;
            ball.setPos(window_width / 2, window_height / 2);
        }

        if (ball.x - ball.radius >= window_width)
        {
            p1score++;
            ball.setPos(window_width / 2, window_height / 2);
        }

        if (ball.y + ball.radius >= window_height || ball.y - ball.radius < 0) // Bottom wall
            ball.vy = -ball.vy;

        int collision_p1 = is_colliding(p1, ball);
        int collision_p2 = is_colliding(p2, ball);

        if (collision_p1 == 0 || collision_p2 == 0)
            ball.vx = -ball.vx;

        if (collision_p1 == 1 || collision_p2 == 1)
            ball.vy = -ball.vy;

        if (collision_p1 == 2 || collision_p2 == 2)
            ball.vx = -ball.vx;

        if (collision_p1 == 3 || collision_p2 == 3)
            ball.vy = -ball.vy;

        // Send positions
        send_packet.clear();
        send_packet << ball.x << ball.y << p1.x << p1.y << p2.x << p2.y;

        status = client1.send(send_packet);
        if (status == sf::Socket::Disconnected)
        {
            running = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    listener.close();
}