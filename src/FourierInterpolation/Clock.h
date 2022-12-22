#pragma once

#include <chrono>

namespace fi
{

    template<typename Dur>
    class Clock
    {
    public:
        using clock = std::chrono::high_resolution_clock;
        using duration = clock::duration;
        using time_point = clock::time_point;

    public:
        Clock()
            : m_paused(false)
            , m_dur{}
            , m_begin(clock::now()) {}
        ~Clock() = default;

        static float getTime()
        {
            if (!s_instance->m_paused)
            {
                s_instance->m_dur = clock::now() - s_instance->m_begin;
            }
            // here we cast to floating-point seconds
            return std::chrono::duration_cast<std::chrono::duration<float>>(s_instance->m_dur).count();
        }

        static void reset()
        {
            if (s_instance)
            {
                s_instance->m_begin = clock::now();
            }
        }

        static void toggle()
        {
            // TODO: Fix clock toggle
            if (s_instance)
            {
                s_instance->m_paused = !s_instance->m_paused;
                if (!s_instance->m_paused)
                {
                    time_point pauseTimepoint = clock::now() - s_instance->m_dur;
                    duration pauseDuration = clock::now() - pauseTimepoint;
                    s_instance->m_begin += pauseDuration;
                }
            }
        }

        static void initialize()
        {
            if (s_instance)
            {
                delete s_instance;
                s_instance = nullptr;
            }

            s_instance = new Clock();
        }

        static void deinitialize()
        {
            delete s_instance;
            s_instance = nullptr;
        }

    private:
        bool m_paused;
        duration m_dur;
        time_point m_begin;
        static inline Clock* s_instance = nullptr;
    };

}; // fi namespace