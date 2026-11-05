#pragma once
#include <chrono>

namespace AC_Engine
{
class EngineClock
{
    

public:
    EngineClock() = default;
    void update()
    {
        auto now = std::chrono::high_resolution_clock::now();
        m_deltaTime = std::chrono::duration<float>(now - m_lastTime).count();
        m_lastTime = now;
    };
    float getDeltaTime() const
    {
        return std::chrono::duration<float>(m_deltaTime).count();
    };

private:
    float m_deltaTime;
    std::chrono::high_resolution_clock::time_point m_lastTime;

};
}