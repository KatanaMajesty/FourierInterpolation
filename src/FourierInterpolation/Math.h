#pragma once

#include <functional>
#include <cassert>

#include <imgui.h>

#define FT_ASSERT(...) assert(__VA_ARGS__)

namespace ft
{
    using Vec2 = ImVec2;
    using Complex = std::complex<float>;

    // Discrete Fourier Transform traits
    struct Traits
    {
        inline static constexpr float pi() { return std::numbers::pi_v<float>; }
        inline static constexpr float pi2() { return pi() * 2.0f; }
        inline static constexpr Complex cplx_i() { return Complex(0.0f, 1.0f); }
    };

    class DiscreteData
    {
    public:
        DiscreteData(size_t samples, float from = 0.0f, float to = 20.0f)
            : samples(samples)
            , from(from)
            , to(to)
        {
            // Resize!
            data.reserve(samples);
            data.resize(samples);
        }
        DiscreteData(const std::function<float(float)>& func, size_t samples, float from = 0.0f, float to = 20.0f)
            : samples(samples)
            , from(from)
            , to(to)
        {
            FT_ASSERT(to > from && "TO should be larger than FROM");
            float L = to - from;
            for (size_t i = 0; i < samples; i++)
            {
                float x = from + (static_cast<float>(i) / samples) * L;
                float fx = std::invoke(func, x);
                data.push_back(Vec2(x, fx));
            }
        }
        ~DiscreteData() = default;

        inline constexpr size_t size() const { return data.size(); }
        inline constexpr Vec2& operator[](size_t idx) { return data.at(idx); }
        inline constexpr const Vec2& operator[](size_t idx) const { return data.at(idx); }

        size_t samples;
        float from;
        float to;
        std::vector<Vec2> data;
    }; // DiscreteData class

    // Forward discrete Fourier Transform
    std::vector<Complex> DFT(const DiscreteData& data)
    {
        std::vector<Complex> res;
        res.reserve(data.samples);
        for (size_t i = 0; i < data.samples; ++i)
        {
            Complex yk; // 0.0f + i0.0f
            float N = data.samples;
            Complex ce = Traits::pi2() * Traits::cplx_i() / N;
            for (size_t j = 0; j < data.samples; ++j)
            {
                float x = data[j].x;
                float fx = data[j].y;
                Complex fourierSin = std::exp(-ce * static_cast<float>(j * i));
                yk += fx * fourierSin;
            }
            res.push_back(yk);
        }
        return res;
    }
    
    // Forward discrete Fourier Transform with Zero-Padding interpolation
    // 
    // Function's name stands for abbreviation of "Frequency-domain Zero-Padding", which Rick Lyons' paper called
    // "How to Interpolate in the Time-Domain by Zero-Padding in the Frequency Domain"
    // @see https://dspguru.com/dsp/howtos/how-to-interpolate-in-time-domain-by-zero-padding-in-frequency-domain/
    //
    std::vector<Complex> FDZP(const DiscreteData& data, size_t factor)
    {
        std::vector<Complex> res;
        size_t newSz = data.samples * factor;
        res.reserve(newSz);
        // calculate how many new zero-samples to insert
        size_t addSamples = newSz - data.samples;
        // this will create zero-valued complex numbers (0.0f + i0.0f), which
        // will then be inserted in the middle of result to achieve zero-padding interpolation
        std::vector<Complex> zeroPadder(addSamples);
        for (size_t i = 0; i < data.samples; ++i)
        {
            Complex yk; // 0.0f + i0.0f
            float N = data.samples;
            Complex ce = Traits::pi2() * Traits::cplx_i() / N;
            for (size_t j = 0; j < data.samples; ++j)
            {
                float x = data[j].x;
                float fx = data[j].y;
                Complex fourierSin = std::exp(-ce * static_cast<float>(j * i));
                yk += fx * fourierSin;
            }
            res.push_back(yk);
        }

        // The amplitudes of the new x’(n) time sequence will be reduced by a factor 
        // (This amplitude reduction can, of course, be avoided by multiplying either the X’(m) or the x’(n) amplitudes)
        for (Complex& c : res)
        {
            c *= static_cast<float>(factor);
        }
        auto mid = res.begin() + res.size() / 2 + 1;
        res.insert(mid, zeroPadder.begin(), zeroPadder.end());
        return res;
    }

    DiscreteData IDFT(const std::vector<Complex>& data, float from = 0.0f, float to = 20.0f)
    {
        FT_ASSERT(to > from && "TO should be larger than FROM");
        float L = to - from;
        size_t samples = data.size();
        DiscreteData res(samples, from, to);
        for (size_t i = 0; i < samples; ++i)
        {
            Complex c; // 0.0f + i0.0f;
            float N = samples;
            Complex ce = Traits::pi2() * Traits::cplx_i() / N;
            for (size_t j = 0; j < samples; ++j)
            {
                Complex yk = data[j];
                Complex fourierSin = std::exp(ce * static_cast<float>(i * j));
                c += yk * fourierSin;
            }
            c /= N; // normalize the result (Because we don't normalize Fourier's coefficients)
            float x = from + (static_cast<float>(i) / samples) * L;
            res.data[i] = Vec2(x, c.real());
        }
        return res;
    }

}; // ft namespace