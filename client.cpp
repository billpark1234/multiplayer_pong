#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include "common.hpp"

/** Overloading so that we can send vector packet */
sf::Packet &operator>>(sf::Packet &packet, Positions &m)
{
    packet >> m.ball_pos.x >> m.ball_pos.y >> m.p1_pos.x >> m.p1_pos.y >> m.p2_pos.x >> m.p2_pos.y;
    return packet;
}

int main()
{
    // Parsing config stuff
    std::string config = "config.txt";
    std::ifstream istrm(config, std::ios::binary);
    if (!istrm.is_open())
    {
        std::cerr << "Can't open config.txt";
        return EXIT_FAILURE;
    }

    std::string server_ip;
    unsigned short port = 0;
    std::string line;

    while (std::getline(istrm, line))
    {
        if (line.find("=") != std::string::npos)
        {
            std::istringstream iss(line);
            std::string target;
            std::string value;
            std::getline(iss, target, '=');
            iss >> std::ws;
            std::getline(iss, value);
            // remove white space
            target.erase(std::remove_if(target.begin(), target.end(), isspace), target.end());
            if (target == "SERVER_IP")
                server_ip.assign(value);

            if (target == "PORT")
                port = std::stoi(value);
        }
    }

    // std::cout << server_ip << std::endl;
    // std::cout << port << std::endl;

    // Connect to the server
    sf::TcpSocket socket;
    sf::Socket::Status status = socket.connect(server_ip, port);
    if (status != sf::Socket::Done)
    {
        std::cerr << "Failed to connect to server";
        return EXIT_FAILURE;
    }
    socket.setBlocking(false);

    // Game starts here:
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
    sf::RectangleShape player1(sf::Vector2f(20.0f, 100.0f));
    sf::RectangleShape player2(sf::Vector2f(20.0f, 100.0f));

    bool inFocus = true;

    sf::Packet send_packet;
    sf::Packet receive_packet;
    Positions positions;
    int move = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::GainedFocus)
                inFocus = true;
            else if (event.type == sf::Event::LostFocus)
                inFocus = false;
        }

        move = 0;
        if (inFocus)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
                move = -1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
                move = 1;
        }

        if (socket.receive(receive_packet) == sf::Socket::Done)
        {
            receive_packet >> positions;
            ball.setPosition(positions.ball_pos);
            player1.setPosition(positions.p1_pos);
            player2.setPosition(positions.p2_pos);
            receive_packet.clear();
        }

        send_packet.clear();
        send_packet << move;
        sf::Socket::Status status = socket.send(send_packet);

        window.clear();
        window.draw(ball);
        window.draw(player1);
        window.draw(player2);
        window.draw(p1text);
        window.draw(p2text);
        window.display();
    }
}