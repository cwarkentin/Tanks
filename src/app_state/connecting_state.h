#ifndef CONNECTING_STATE_H
#define CONNECTING_STATE_H
#include "appstate.h"
#include "../appconfig.h"
#include "../network/network_manager.h"
#include "game/game.h"
#include "menu.h"
#include <string>
#include <thread>
#include <atomic>
#include <iostream>

class ConnectingState : public AppState
{
public:
    ConnectingState(InteractiveComponents interactive_components, StateMachine *state_machine, const std::string &ip)
        : AppState(interactive_components, state_machine)
        , m_ip(ip)
        , m_connected(false)
        , m_failed(false)
        , m_network_manager(nullptr)
    {}

    ~ConnectingState()
    {
        if (m_thread.joinable())
            m_thread.join();
    }

    void draw(Renderer &renderer) override
    {
        renderer.clear();
        renderer.drawRect(AppConfig::map_rect, {0, 0, 0, 255}, true);
        renderer.drawRect(AppConfig::status_rect, {0, 0, 0, 255}, true);
        if (m_failed)
        {
            renderer.drawText({100, 200}, "Failed to connect to:", {255, 0, 0, 255}, FontSize::BIG);
            renderer.drawText({100, 240}, m_ip, {255, 0, 0, 255}, FontSize::BIG);
            renderer.drawText({100, 300}, "Press ESC to go back", {150, 150, 150, 255}, FontSize::NORMAL);
        }
        else
        {
            renderer.drawText({100, 200}, "Connecting to:", {255, 255, 255, 255}, FontSize::BIG);
            renderer.drawText({100, 240}, m_ip, {255, 255, 0, 255}, FontSize::BIG);
            renderer.drawText({100, 300}, "Press ESC to cancel", {150, 150, 150, 255}, FontSize::NORMAL);
        }
        renderer.flush();
    }

    void update(const UpdateState &updateState) override
    {
        if (m_connected)
        {
            std::cout << "Seed: " << m_network_manager->syncSeed() << std::endl;
            Game *game = new Game(2, m_interactive_components, m_state_machine);
            game->m_network_manager = m_network_manager;
            transiteTo(game);
        }
    }

    void eventProcess(const Event &event) override
    {
        if (event.type() == Event::KEYBOARD)
        {
            const KeyboardEvent &ev = static_cast<const KeyboardEvent &>(event);
            if (ev.isPressed(KeyCode::KEY_ESCAPE))
            {
                if (m_thread.joinable())
                    m_thread.join();
                if (m_network_manager)
                    delete m_network_manager;
                transiteTo(new Menu(m_interactive_components, m_state_machine));
            }
        }
    }

protected:
    void onInitialize() override
    {
        m_network_manager = new NetworkManager();
        m_thread = std::thread([this]()
        {
            if (m_network_manager->join(m_ip.c_str(), 12345))
                m_connected = true;
            else
                m_failed = true;
        });
    }

private:
    std::string m_ip;
    std::atomic<bool> m_connected;
    std::atomic<bool> m_failed;
    NetworkManager *m_network_manager;
    std::thread m_thread;
};
#endif // CONNECTING_STATE_H