#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include "common.hpp"

/** Overloading so that we can receive vector packet */
sf::Packet &operator<<(sf::Packet &packet, Positions &m)
{
    packet << m.ball_pos.x << m.ball_pos.y << m.p1_pos.x << m.p1_pos.y << m.p2_pos.x << m.p2_pos.y;
    return packet;
}

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

int main()
{
    sf::TcpListener listener;

    if (listener.listen(5000) != sf::Socket::Done)
    {
        std::cerr << "Listen failed";
        return EXIT_FAILURE;
    }

    // accept blocks, meaning the server will pause execution until both
    // clients are connected.
    sf::TcpSocket client1;
    if (listener.accept(client1) != sf::Socket::Done)
    {
        std::cerr << "Accept failed for client1";
        return EXIT_FAILURE;
    }

    // sf::TcpSocket client2;
    // if (listener.accept(client2) != sf::Socket::Done)
    // {
    //     std::cerr << "Accept failed for client2";
    //     return EXIT_FAILURE;
    // }

    sf::SocketSelector selector;
    selector.add(client1);
    // selector.add(client2);

    bool running = true;
    constexpr int window_width = 800;
    constexpr int window_height = 400;
    constexpr float player_speed = 5.0f;

    int p1score = 0;
    int p2score = 0;

    /** Since shape is used for both drawing and physics, I use graphics module
     * on both client and server.
     */

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
    int p1move = 0; // 0-no move. -1-up. 1=down.
    int p2move = 0;

    Positions positions;
    sf::Packet send_packet;
    sf::Packet receive_packet;

    while (running)
    {
        /* receive data */
        if (selector.wait())
        {
            if (selector.isReady(client1))
            {
                // std::cout << "ready" << std::endl;
                sf::Socket::Status status = client1.receive(receive_packet);
                if (status == sf::Socket::Disconnected)
                {
                    std::cout << "Disconnected" << std::endl;
                    selector.remove(client1);
                    running = false;
                }
                else if (status == sf::Socket::Done)
                {
                    receive_packet >> p1move;
                    receive_packet.clear();
                }
            }
        }

        /* game logic below */
        std::cout << p1move << std::endl;

        ball.move(velocity);
        player1.move(0.0f, player_speed * p1move);

        const sf::Vector2f &pos = ball.getPosition();

        if (pos.x + radius < 0) // we count score if ball completely disappears.
        {
            p2score++;
            ball.setPosition(window_width / 2, window_height / 2);
        }

        if (pos.x - radius >= window_width)
        {
            p1score++;
            ball.setPosition(window_width / 2, window_height / 2);
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

        send_packet.clear();
        positions.ball_pos = ball.getPosition();
        positions.p1_pos = player1.getPosition();
        positions.p2_pos = player2.getPosition();
        send_packet << positions;

        client1.send(send_packet);
    }

    listener.close();
}