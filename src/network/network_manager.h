#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include "input_packet.h"

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool host(int port);          // called by player 1
    bool join(const char* ip, int port);  // called by player 2
    uint32_t syncSeed() const { return m_sync_seed; }

    bool sendInput(const InputPacket &input);
    bool receiveInput(InputPacket &input);

    bool isConnected() const { return m_connected; }
    bool isHost() const { return m_is_host; }   

private:
    UDPsocket m_socket;
    UDPpacket *m_packet;
    IPaddress m_peer;
    bool m_connected = false;
    bool m_is_host = false;
    uint32_t m_sync_seed = 0;
};