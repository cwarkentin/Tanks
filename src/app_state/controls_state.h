#ifndef CONTROLS_STATE_H
#define CONTROLS_STATE_H
#include "appstate.h"
#include "../appconfig.h"
#include "menu.h"
#include <string>
#include <vector>

class ControlsState : public AppState
{
public:
    ControlsState(InteractiveComponents interactive_components, StateMachine *state_machine)
        : AppState(interactive_components, state_machine)
        , m_selected(0)
        , m_listening(false)
    {
        m_bindings = {
            {"P1 Up",    &AppConfig::player_1_keys, 0},
            {"P1 Down",  &AppConfig::player_1_keys, 1},
            {"P1 Left",  &AppConfig::player_1_keys, 2},
            {"P1 Right", &AppConfig::player_1_keys, 3},
            {"P1 Fire",  &AppConfig::player_1_keys, 4},
            {"P2 Up",    &AppConfig::player_2_keys, 0},
            {"P2 Down",  &AppConfig::player_2_keys, 1},
            {"P2 Left",  &AppConfig::player_2_keys, 2},
            {"P2 Right", &AppConfig::player_2_keys, 3},
            {"P2 Fire",  &AppConfig::player_2_keys, 4},
        };
    }

    void draw(Renderer &renderer) override
    {
        renderer.clear();
        renderer.drawRect(AppConfig::map_rect, {0, 0, 0, 255}, true);
        renderer.drawRect(AppConfig::status_rect, {0, 0, 0, 255}, true);

        renderer.drawText({150, 20}, "CONTROLS", {255, 255, 255, 255}, FontSize::BIG);

        for (int i = 0; i < (int)m_bindings.size(); i++)
        {
            Color color = (i == m_selected) ? Color{255, 255, 0, 255} : Color{255, 255, 255, 255};
            std::string label = m_bindings[i].name + ": " + keyName((*m_bindings[i].keys)[m_bindings[i].index]);

            if (i == m_selected && m_listening)
                label = m_bindings[i].name + ": [press a key]";

            renderer.drawText({80, 55 + i * 28}, label, color, FontSize::NORMAL);
        }

        renderer.drawText({80, 340}, "ENTER - rebind   ESC - back", {150, 150, 150, 255}, FontSize::NORMAL);
        renderer.flush();
    }

    void update(const UpdateState &updateState) override {}

    void eventProcess(const Event &event) override
    {
        if (event.type() == Event::KEYBOARD)
        {
            const KeyboardEvent &ev = static_cast<const KeyboardEvent &>(event);

            if (m_listening)
            {
                // Any key pressed becomes the new binding
                // Get the raw keycode from the event by checking all known keys
                KeyCode newKey = getKeyFromEvent(ev);
                if (newKey != KeyCode::KEY_UNKNOWN && newKey != KeyCode::KEY_ESCAPE)
                {
                    (*m_bindings[m_selected].keys)[m_bindings[m_selected].index] = newKey;
                    m_listening = false;
                }
                else if (newKey == KeyCode::KEY_ESCAPE)
                {
                    m_listening = false;
                }
                return;
            }

            if (ev.isPressed(KeyCode::KEY_ESCAPE))
            {
                transiteTo(new Menu(m_interactive_components, m_state_machine));
                return;
            }
            if (ev.isPressed(KeyCode::KEY_UP))
            {
                m_selected = (m_selected == 0) ? (int)m_bindings.size() - 1 : m_selected - 1;
            }
            else if (ev.isPressed(KeyCode::KEY_DOWN))
            {
                m_selected = (m_selected >= (int)m_bindings.size() - 1) ? 0 : m_selected + 1;
            }
            else if (ev.isPressed(KeyCode::KEY_RETURN))
            {
                m_listening = true;
            }
        }
    }

protected:
    void onInitialize() override {}

private:
    struct Binding {
        std::string name;
        std::vector<KeyCode> *keys;
        int index;
    };

    std::vector<Binding> m_bindings;
    int m_selected;
    bool m_listening;

    std::string keyName(KeyCode key)
    {
        switch (key)
        {
        case KEY_UP:     return "UP";
        case KEY_DOWN:   return "DOWN";
        case KEY_LEFT:   return "LEFT";
        case KEY_RIGHT:  return "RIGHT";
        case KEY_W:      return "W";
        case KEY_A:      return "A";
        case KEY_S:      return "S";
        case KEY_D:      return "D";
        case KEY_RCTRL:  return "RCTRL";
        case KEY_LCTRL:  return "LCTRL";
        case KEY_RALT:   return "RALT";
        case KEY_LALT:   return "LALT";
        case KEY_SPACE:  return "SPACE";
        case KEY_RETURN: return "ENTER";
        default:         return "KEY(" + std::to_string((int)key) + ")";
        }
    }

    KeyCode getKeyFromEvent(const KeyboardEvent &ev)
    {
        // Check all known keycodes
        static const KeyCode allKeys[] = {
            KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
            KEY_W, KEY_A, KEY_S, KEY_D,
            KEY_RCTRL, KEY_LCTRL, KEY_RALT, KEY_LALT,
            KEY_SPACE, KEY_RETURN, KEY_ESCAPE,
            KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
            KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
            KEY_Q, KEY_E, KEY_R, KEY_T, KEY_Y,
            KEY_U, KEY_I, KEY_O, KEY_P, KEY_F,
            KEY_G, KEY_H, KEY_J, KEY_K, KEY_L,
            KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B,
            KEY_N, KEY_M,
            KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
            KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
        };
        for (KeyCode k : allKeys)
        {
            if (ev.isPressed(k)) return k;
        }
        return KEY_UNKNOWN;
    }
};
#endif // CONTROLS_STATE_H