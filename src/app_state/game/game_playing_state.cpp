#include "game.h"
#include "../../appconfig.h"
#include "../menu.h"

Game::PlayingState::PlayingState(Game *ps) : ContextState(ps, ps->m_game_state_machine) {}

void Game::PlayingState::draw(Renderer &renderer)
{
    m_context->drawScene(renderer);
}

void Game::PlayingState::update(const UpdateState &updateState)
{
    if (m_context->m_network_manager && m_context->m_network_manager->isConnected())
    {
        m_context->m_local_input.frame = m_context->m_frame;
        m_context->m_network_manager->sendInput(m_context->m_local_input);
        m_context->m_network_manager->receiveInput(m_context->m_remote_input);

        if (m_context->m_network_manager->isHost())
        {
            if (m_context->m_players.size() > 0)
                m_context->m_players[0]->applyNetworkInput(m_context->m_local_input);
            if (m_context->m_players.size() > 1)
                m_context->m_players[1]->applyNetworkInput(m_context->m_remote_input);
        }
        else
        {
            if (m_context->m_players.size() > 0)
                m_context->m_players[0]->applyNetworkInput(m_context->m_remote_input);
            if (m_context->m_players.size() > 1)
                m_context->m_players[1]->applyNetworkInput(m_context->m_local_input);
        }

        // Use fixed timestep when playing online so both machines step identically
        m_context->updateScene(16); // fixed 16ms (~60fps) instead of variable delta_time
    }
    else
    {
        m_context->updateScene(updateState.delta_time);
    }

    m_context->m_frame++;

    if (m_context->m_enemies.empty() && m_context->m_enemies_to_kill_count <= 0)
    {
        transiteTo(new Game::LevelEndingState(m_context, false, false));
    }
    if (m_context->m_players.empty())
    {
        transiteTo(new Game::GameOverState(m_context));
    }
}

void Game::PlayingState::eventProcess(const Event &event)
{
    if (event.type() == Event::KEYBOARD)
    {
        const KeyboardEvent &event_key = static_cast<const KeyboardEvent &>(event);
        if (event_key.isPressed(KEY_N))
        {
            transiteTo(new Game::LevelEndingState(m_context, true, false));
        }
        else if (event_key.isPressed(KEY_B))
        {
            m_context->m_current_level -= 2;
            transiteTo(new Game::LevelEndingState(m_context, true, false));
        }
        else if (event_key.isPressed(KEY_T))
        {
            m_context->m_show_enemies_targets = !m_context->m_show_enemies_targets;
        }
        else if (event_key.isPressed(KEY_RETURN))
        {
            transiteTo(new Game::PauseState(m_context));
        }
        else if (event_key.isPressed(KEY_ESCAPE))
        {
            m_context->transiteTo(new Menu(m_context->m_interactive_components, m_context->m_state_machine));
        }

        for (auto player : m_context->m_players)
        {
            if (m_context->m_network_manager && m_context->m_network_manager->isConnected())
            {
                // Use correct keys based on role - host=P1 keys, client=P2 keys
                auto &keys = m_context->m_network_manager->isHost()
                    ? AppConfig::player_1_keys
                    : AppConfig::player_2_keys;

                if (event_key.isPressed(keys[0]))  m_context->m_local_input.up    = true;
                if (event_key.isPressed(keys[1]))  m_context->m_local_input.down  = true;
                if (event_key.isPressed(keys[2]))  m_context->m_local_input.left  = true;
                if (event_key.isPressed(keys[3]))  m_context->m_local_input.right = true;
                if (event_key.isPressed(keys[4]))  m_context->m_local_input.fire  = true;

                if (event_key.isReleased(keys[0])) m_context->m_local_input.up    = false;
                if (event_key.isReleased(keys[1])) m_context->m_local_input.down  = false;
                if (event_key.isReleased(keys[2])) m_context->m_local_input.left  = false;
                if (event_key.isReleased(keys[3])) m_context->m_local_input.right = false;
                if (event_key.isReleased(keys[4])) m_context->m_local_input.fire  = false;
            }
            else
            {
                // Local: use normal keyboard handling which respects key bindings
                player->handleKeyboardEvent(event_key);
            }
        }
    }
}