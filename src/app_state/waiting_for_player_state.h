#ifndef WAITING_FOR_PLAYER_STATE_H
#define WAITING_FOR_PLAYER_STATE_H
#include "appstate.h"
#include "../network/network_manager.h"
#include "game/game.h"
#include "menu.h"
#include "../appconfig.h"
#include <thread>
#include <atomic>

class WaitingForPlayerState : public AppState
{
public:
    WaitingForPlayerState(InteractiveComponents interactive_components, StateMachine *state_machine, NetworkManager *net)
        : AppState(interactive_components, state_machine)
        , m_network_manager(net)
        , m_connected(false)
        , m_failed(false)
    {}

    ~WaitingForPlayerState()
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
            renderer.drawText({150, 200}, "Connection failed!", {255, 0, 0, 255}, FontSize::BIG);
        else
            renderer.drawText({100, 200}, "Waiting for player...", {255, 255, 255, 255}, FontSize::BIG);
        renderer.flush();
    }

    void update(const UpdateState &updateState) override
    {
        if (m_connected)
        {
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
                transiteTo(new Menu(m_interactive_components, m_state_machine));
            }
        }
    }

protected:
    void onInitialize() override
    {
        // Start handshake in background thread
        m_thread = std::thread([this]()
        {
            if (m_network_manager->host(12345))
                m_connected = true;
            else
                m_failed = true;
        });
    }

private:
    NetworkManager *m_network_manager;
    std::atomic<bool> m_connected;
    std::atomic<bool> m_failed;
    std::thread m_thread;
};
#endif // WAITING_FOR_PLAYER_STATE_H