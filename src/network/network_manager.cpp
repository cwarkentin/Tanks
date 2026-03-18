#include "network_manager.h"
#include <cstring>
#include <iostream>

NetworkManager::NetworkManager() {
    SDLNet_Init();
    m_packet = SDLNet_AllocPacket(512);
}

NetworkManager::~NetworkManager() {
    if (m_packet) SDLNet_FreePacket(m_packet);
    if (m_socket) SDLNet_UDP_Close(m_socket);
    SDLNet_Quit();
}

bool NetworkManager::host(int port) {
    m_socket = SDLNet_UDP_Open(port);
    if (!m_socket) {
        std::cerr << "Host failed: " << SDLNet_GetError() << std::endl;
        return false;
    }
    m_is_host = true;
    std::cout << "Waiting for client to connect..." << std::endl;

    // Wait for handshake packet from client
    while (true) {
        int result = SDLNet_UDP_Recv(m_socket, m_packet);
        if (result > 0) {
            m_peer = m_packet->address;  // learn client's address
            // Send ack back
            m_packet->address = m_peer;
            m_packet->data[0] = 'A';
            m_packet->len = 1;
            SDLNet_UDP_Send(m_socket, -1, m_packet);
            m_connected = true;
            std::cout << "Client connected!" << std::endl;
            return true;
        }
        SDL_Delay(100);
    }
}

bool NetworkManager::join(const char* ip, int port) {
    m_socket = SDLNet_UDP_Open(0);
    if (!m_socket) {
        std::cerr << "Join failed: " << SDLNet_GetError() << std::endl;
        return false;
    }
    if (SDLNet_ResolveHost(&m_peer, ip, port) < 0) {
        std::cerr << "Resolve failed: " << SDLNet_GetError() << std::endl;
        return false;
    }
    m_is_host = false;

    // Send handshake packets until we get an ack
    std::cout << "Connecting to host..." << std::endl;
    int attempts = 0;
    while (attempts < 50) {  // try for 5 seconds
        m_packet->address = m_peer;
        m_packet->data[0] = 'H';
        m_packet->len = 1;
        SDLNet_UDP_Send(m_socket, -1, m_packet);

        SDL_Delay(100);

        int result = SDLNet_UDP_Recv(m_socket, m_packet);
        if (result > 0 && m_packet->data[0] == 'A') {
            m_connected = true;
            std::cout << "Connected to host!" << std::endl;
            return true;
        }
        attempts++;
    }
    std::cerr << "Connection timed out" << std::endl;
    return false;
}

bool NetworkManager::sendInput(const InputPacket &input) {
    memcpy(m_packet->data, &input, sizeof(InputPacket));
    m_packet->len = sizeof(InputPacket);
    m_packet->address = m_peer;
    return SDLNet_UDP_Send(m_socket, -1, m_packet) > 0;
}

bool NetworkManager::receiveInput(InputPacket &input) {
    int result = SDLNet_UDP_Recv(m_socket, m_packet);
    if (result > 0) {
        // first packet from client tells us their address (host side)
        if (m_is_host) m_peer = m_packet->address;
        memcpy(&input, m_packet->data, sizeof(InputPacket));
        return true;
    }
    return false;
}