#pragma once

#include <atomic>

template <typename PlayerType>
class Looper
{
public:
    explicit Looper(PlayerType& targetPlayer) : player(targetPlayer) {}

    void setEnabled(bool shouldLoop)
    {
        enabled.store(shouldLoop);
    }

    bool isEnabled() const
    {
        return enabled.load();
    }

    void update()
    {
        if (! enabled.load())
            return;

        if (player.getLengthInSeconds() > 0.0
            && player.getPositionRelative() >= 0.999)
            player.setPosition(0.0);
    }

private:
    PlayerType& player;
    std::atomic<bool> enabled { false };
};