#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <cmath>
#include <numbers>
#include <complex>
#include <algorithm>

#include "Backend.h"
#include "Math.h"

void PlotLine(const std::vector<ft::Vec2>& data, const ft::Vec2& boundsX, const ft::Vec2& boundsY, ImPlotMarker marker = ImPlotMarker_None)
{
    size_t N = data.size();
    ImPlot::SetNextMarkerStyle(marker);
    ImPlot::SetupAxisLimits(ImAxis_X1, boundsX.x, boundsX.y, ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, boundsY.x, boundsY.y, ImGuiCond_Always);
    ImPlot::SetupAxes("x","f(x)");
    ImPlot::PlotLine("f(x)", &data[0].x, &data[0].y, N, ImPlotLineFlags_None, 0, sizeof(ft::Vec2));
}

void PlotLine(const ft::DiscreteData& plot, const ft::Vec2& boundsY, ImPlotMarker marker = ImPlotMarker_None)
{
    PlotLine(plot.data, ft::Vec2(plot.from, plot.to), boundsY, marker);
}

float CalculateError(const ft::DiscreteData& lhs, const ft::DiscreteData& rhs, size_t factor)
{
    float error = 0;  
    size_t N = lhs.size();
    for (size_t i = 0; i < N; ++i)
    {
        error += std::abs(lhs[i].y - rhs[factor * i].y);
    }
    error /= N;
    return error;
}

int32_t main()
{ 
    // First we initialize the rendering context
    auto context = ft::initializeContext();
    if (!context.initialized)
    {
        std::cout << "Failed to initialize context! Aborting...\n";
        return -1;
    }

    // Determine the number of samples
    size_t samples = 40;
    auto mathFunc = [](float x) -> float 
    { 
        return sin(x) + 0.5f * sin(x + 3.0f * x); 
    };
    ft::DiscreteData mathData(mathFunc, samples);

    std::vector<ft::Complex> coeffs = ft::DFT(mathData);
    ft::DiscreteData result = ft::IDFT(coeffs);

    size_t factor = 4;
    std::vector<ft::Complex> interpolated = ft::FDZP(mathData, factor);
    ft::DiscreteData interp = ft::IDFT(interpolated);
    
    // Calculate an error
    float error = CalculateError(result, interp, factor);

    bool renderMarkers = true;
    while (!context.shouldClose())
    {
        context.begin();
        {
            ImGui::Begin("Fourier Interpolation");
            {
                if (ImGui::SliderInt("Amount of Samples", (int32_t*) &samples, 0, 80) ||
                    ImGui::SliderInt("Scaling Factor", (int32_t*) &factor, 1, 16))
                {
                    mathData = ft::DiscreteData(mathFunc, samples);
                    coeffs = ft::DFT(mathData);
                    result = ft::IDFT(coeffs);

                    interpolated = ft::FDZP(mathData, factor);
                    interp = ft::IDFT(interpolated);
                    error = CalculateError(result, interp, factor);
                };
                ImGui::SameLine();
                ImGui::Checkbox("Render Markers", &renderMarkers);
                if (ImPlot::BeginPlot("Periodic function"))
                {
                    PlotLine(mathData, ft::Vec2(-2.0f, 2.0f), renderMarkers ? ImPlotMarker_Circle : ImPlotMarker_None);
                    ImPlot::EndPlot();
                }
                if (ImPlot::BeginPlot("IDFT result"))
                {
                    ImPlot::PushColormap(ImPlotColormap_Pastel);
                    PlotLine(result, ft::Vec2(-2.0f, 2.0f), renderMarkers ? ImPlotMarker_Circle : ImPlotMarker_None);
                    ImPlot::PopColormap();
                    ImPlot::EndPlot();
                }
                if (ImPlot::BeginPlot("Interpolation result"))
                {
                    ImPlot::PushColormap(ImPlotColormap_Twilight);
                    ImPlot::NextColormapColor();
                    PlotLine(interp, ft::Vec2(-2.0f, 2.0f), renderMarkers ? ImPlotMarker_Circle : ImPlotMarker_None);
                    ImPlot::PopColormap();
                    ImPlot::EndPlot();
                }
                ImGui::Text("Error: %.9f", error);
            }
            ImGui::End();
        }
        context.end();
    }

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}