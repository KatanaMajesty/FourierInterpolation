#pragma once

#include <chrono>

namespace fi
{

    template<typename Dur>
    class Clock
    {
    public:
        using clock = std::chrono::high_resolution_clock;
        using duration = Dur;
        using timepoint = std::chrono::time_point<clock>;

    public:
        Clock() = default;
        ~Clock() = default;

        static duration::rep getTime()
        {
            duration d = std::chrono::duration_cast<duration>(clock::now() - s_instance->m_begin);
            return d.count();
        } 

        static void initialize()
        {
            if (s_instance)
            {
                delete s_instance;
                s_instance = nullptr;
            }

            s_instance = new Clock();
            s_instance->m_begin = clock::now();
        }

        static void deinitialize()
        {
            delete s_instance;
            s_instance = nullptr;
        }

    private:
        timepoint m_begin;
        static inline Clock* s_instance = nullptr;
    };

}; // fi namespace