#ifndef IP_INPUT_STATE_H
#define IP_INPUT_STATE_H
#include "appstate.h"
#include "../appconfig.h"
#include "../network/network_manager.h"
#include "connecting_state.h"
#include "game/game.h"
#include "menu.h"
#include <string>
#include <iostream>

class IPInputState : public AppState
{
public:
    IPInputState(InteractiveComponents interactive_components, StateMachine *state_machine)
        : AppState(interactive_components, state_machine)
        , m_ip("")
        , m_error("")
    {}

    void draw(Renderer &renderer) override
    {
        renderer.clear();
        renderer.drawRect(AppConfig::map_rect, {0, 0, 0, 255}, true);
        renderer.drawRect(AppConfig::status_rect, {0, 0, 0, 255}, true);
        renderer.drawText({100, 160}, "Enter Host IP:", {255, 255, 255, 255}, FontSize::BIG);
        renderer.drawText({100, 210}, m_ip + "_", {255, 255, 0, 255}, FontSize::BIG);
        renderer.drawText({100, 260}, "Press ENTER to connect", {150, 150, 150, 255}, FontSize::NORMAL);
        renderer.drawText({100, 290}, "Press ESC to go back", {150, 150, 150, 255}, FontSize::NORMAL);
        if (!m_error.empty())
            renderer.drawText({100, 340}, m_error, {255, 0, 0, 255}, FontSize::NORMAL);
        renderer.flush();
    }

    void update(const UpdateState &updateState) override {}

    void eventProcess(const Event &event) override
    {
        if (event.type() == Event::KEYBOARD)
        {
            const KeyboardEvent &ev = static_cast<const KeyboardEvent &>(event);

            if (ev.isPressed(KeyCode::KEY_ESCAPE))
            {
                transiteTo(new Menu(m_interactive_components, m_state_machine));
                return;
            }

            if (ev.isPressed(KeyCode::KEY_RETURN))
            {
                if (m_ip.empty())
                {
                    m_error = "Please enter an IP address";
                    return;
                }
                NetworkManager *net = new NetworkManager();
                if (net->join(m_ip.c_str(), 12345))
                {
                    std::cout << "Seed: " << net->syncSeed() << std::endl;
                    Game *game = new Game(2, m_interactive_components, m_state_machine);
                    game->m_network_manager = net;
                    transiteTo(game);
                }
                else
                {
                    delete net;
                    m_error = "Failed to connect to " + m_ip;
                }
                return;
            }

            if (ev.isPressed(KeyCode::KEY_BACKSPACE))
            {
                if (!m_ip.empty())
                    m_ip.pop_back();
                return;
            }

            // Map number keys and dot to characters
            struct { KeyCode key; char ch; } keymap[] = {
                {KEY_0, '0'}, {KEY_1, '1'}, {KEY_2, '2'}, {KEY_3, '3'}, {KEY_4, '4'},
                {KEY_5, '5'}, {KEY_6, '6'}, {KEY_7, '7'}, {KEY_8, '8'}, {KEY_9, '9'},
                {KEY_DOT, '.'}
            };

            for (auto &k : keymap)
            {
                if (ev.isPressed(k.key))
                {
                    m_ip += k.ch;
                    return;
                }
            }
        }
    }
protected:
    void onInitialize() override {}

private:
    std::string m_ip;
    std::string m_error;
};
#endif // IP_INPUT_STATE_H