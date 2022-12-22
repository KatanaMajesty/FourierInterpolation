#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <cmath>
#include <numbers>
#include <complex>
#include <algorithm>

#include "Backend.h"
#include "Clock.h"

using clock_type = fi::Clock<std::chrono::duration<float>>;

namespace fi
{

    template<size_t Size>
    void PlotLine(const std::array<ImVec2, Size>& graph, float freq, const char* title) 
    {
        if (ImPlot::BeginPlot(title)) 
        {
            ImVec2 maxVal(50.0f, 3.0f);
            ImVec2 minVal(0.0f, -3.0f);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond);
            ImPlot::SetupAxisLimits(ImAxis_X1, minVal.x, maxVal.x);
            ImPlot::SetupAxisLimits(ImAxis_Y1, minVal.y, maxVal.y);
            ImPlot::SetupAxes("x","fx");
            ImPlot::PlotLine("f(x)", &graph[0].x, &graph[0].y, Size, ImPlotLineFlags_None, 0, sizeof(ImVec2));

            float period = 1.0f / freq;
            static std::array<float, 5> frequencyGraph;
            for (int32_t i = 0; i < frequencyGraph.size(); i++)
            {
                frequencyGraph.at(i) = period * (float) i;
            }
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
            ImPlot::PlotInfLines("Frequency", frequencyGraph.data(), frequencyGraph.size());
            ImPlot::EndPlot();
        }
    }

    // utility structure for realtime plot
    struct RollingBuffer 
    {
        float span;
        std::vector<ImVec2> data;

        RollingBuffer(float span) 
            : span(span)
        {
            data.reserve(2000);
        }

        void addPoint(float x, float y) 
        {
            float xmod = fmodf(x, span);
            if (!data.empty() && xmod < data.back().x)
            {
                data.clear();
            }
            data.push_back(ImVec2(xmod, y));
        }
    };

}; // fi namespace

template<size_t Size>
void FreqEulersInterpretation(const std::array<ImVec2, Size>& graph, float freq)
{
    std::array<float, Size> eulers_x;
    std::array<float, Size> eulers_fx; 
    std::complex<float> weight;
    for (size_t i = 0; i < graph.size(); i++)
    {
        auto cplx = graph[i].y * std::exp(-2 * std::numbers::pi_v<float> * std::complex(0.0f, 1.0f) * freq * graph[i].x);
        weight += cplx;
        eulers_x.at(i) = cplx.real(); 
        eulers_fx.at(i) = cplx.imag();
    }
    weight /= graph.size();
    float wx = weight.real();
    float wy = weight.imag();
    ImVec2 windowSize = ImGui::GetWindowSize();
    if (ImPlot::BeginPlot("Eulers", ImVec2((windowSize.x - 25.0f) / 3.0f, windowSize.y / 2.0f)))
    {
        ImPlot::SetNextMarkerStyle(ImPlotMarker_None);
        ImPlot::SetupAxes("x","fx");
        ImPlot::SetupAxesLimits(-2.0, 2.0, -2.0, 2.0, ImGuiCond_Always);
        ImPlot::PlotLine("f(x)", eulers_x.data(), eulers_fx.data(), Size, ImPlotLineFlags_None);
        ImPlot::PlotScatter("Weight", &wx, &wy, 1);

        ImPlot::EndPlot();
    }

    static float history = 1.0f;
    static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
    static fi::RollingBuffer tReal(history);
    static fi::RollingBuffer tImag(history);
    tReal.addPoint(freq, wx);
    tImag.addPoint(freq, wy);
    ImGui::SameLine();
    if (ImPlot::BeginPlot("##Rolling", ImVec2(2.0f * (windowSize.x - 25.0f) / 3.0f, windowSize.y / 2.0f)))
    {
        ImPlot::SetNextMarkerStyle(ImPlotMarker_None);
        ImPlot::SetupAxes(NULL, NULL, flags, flags);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, history, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5, 0.5);
        ImPlot::PlotLine("Real Part", &tReal.data[0].x, &tReal.data[0].y, tReal.data.size(), 0, 0, sizeof(ImVec2));
        ImPlot::PlotLine("Imaginary Part", &tImag.data[0].x, &tImag.data[0].y, tImag.data.size(), 0, 0, sizeof(ImVec2));
        ImPlot::EndPlot();
    }
}


int32_t main()
{ 
    auto context = fi::initializeContext<500>();
    if (!context.initialized)
    {
        std::cout << "Failed to initialize context! Aborting...\n";
        return -1;
    }
    context.setFunc([](float x) -> float {
        return cos(x);
    });  

    clock_type::initialize();
    while (!context.shouldClose())
    {
        context.begin();
        {
            ImGui::Begin("Fourier Interpolation");
            {
                float currentTime = clock_type::getTime() * 0.1f;
                float frequency = currentTime / (2 * std::numbers::pi_v<float>);
                ImGui::Text("Current time: %f", currentTime);
                ImGui::Text("Current frequency: %f", frequency);
                fi::PlotLine(context.graph, frequency, "Periodic");
                FreqEulersInterpretation(context.graph, frequency);
                if (ImGui::Button("Reset"))
                {
                    clock_type::reset();
                }
                if (ImGui::Button("Toggle Clock"))
                {
                    clock_type::toggle();
                }
            }
            ImGui::End();
            ImPlot::ShowDemoWindow();
        }
        context.end();
    }
    clock_type::deinitialize();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}