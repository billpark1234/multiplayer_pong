#include <SFML/Network.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

int main()
{
    /* Parsing config stuff */
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

    /* Socket stuff begins here */
    sf::TcpSocket socket;
    sf::Socket::Status status = socket.connect(server_ip, port);
    if (status != sf::Socket::Done)
    {
        std::cerr << "Failed to connect to server";
        return EXIT_FAILURE;
    }

    char data[6] = "hello";

    if (socket.send(data, 6 * sizeof(char)) != sf::Socket::Done)
    {
        std::cerr << "Failed to send to server";
        return EXIT_FAILURE;
    }
}