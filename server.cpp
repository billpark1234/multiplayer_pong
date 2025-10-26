#include <SFML/Network.hpp>
#include <iostream>

int main()
{
    sf::TcpListener listener;

    if (listener.listen(5000) != sf::Socket::Done)
    {
        std::cerr << "Listen failed";
        return EXIT_FAILURE;
    }

    sf::TcpSocket client;
    if (listener.accept(client) != sf::Socket::Done)
    {
        std::cerr << "Accept failed";
        return EXIT_FAILURE;
    }

    char data[6];
    std::size_t received;
    if (client.receive(data, 6, received) != sf::Socket::Done)
    {
        std::cerr << "Failed to receive";
        return EXIT_FAILURE;
    }
    std::cout << "Received " << received << " bytes" << std::endl;
    std::cout << data << std::endl;
}