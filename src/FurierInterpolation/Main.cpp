#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <cmath>
#include <numbers>
#include <complex>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include "Clock.h"
using clock_type = fi::Clock<std::chrono::duration<float>>;

namespace fi
{

    template<size_t Size>
    void PlotLine(const std::array<ImVec2, Size>& graph, float freq, const char* title) 
    {
        if (ImPlot::BeginPlot(title)) 
        {
            ImPlot::SetNextMarkerStyle(ImPlotMarker_None);
            ImPlot::SetupAxes("x","fx");
            ImPlot::PlotLine("f(x)", &graph[0].x, &graph[0].y, Size, ImPlotLineFlags_None, 0, sizeof(ImVec2));

            float period = 1.0f / freq;
            if (period < graph.size() * 0.1f)
            {
                static std::array<ImVec2, 5> frequencyGraph;
                for (int32_t i = 0; i < frequencyGraph.size(); i++)
                {
                    frequencyGraph.at(i).x = period * (float) i;
                    frequencyGraph.at(i).y = 0;
                }
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                ImPlot::PlotScatter("T(freq)", &frequencyGraph[0].x, &frequencyGraph[0].y, frequencyGraph.size(), 
                    ImPlotLineFlags_None, 0, sizeof(ImVec2));
            }

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
                data.shrink_to_fit();
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
    if (ImPlot::BeginPlot("Eulers"))
    {
        ImPlot::SetNextMarkerStyle(ImPlotMarker_None);
        ImPlot::SetupAxes("x","fx");
        ImPlot::PlotLine("f(x)", eulers_x.data(), eulers_fx.data(), Size, ImPlotLineFlags_None);
        ImPlot::PlotScatter("Weight", &wx, &wy, 1);

        ImPlot::EndPlot();
    }

    static fi::RollingBuffer rweight(1.0f);
    rweight.addPoint(freq, wx);
    if (ImPlot::BeginPlot("Rolling"))
    {
        ImPlot::PlotLine("freq -> weight", &rweight.data[0].x, &rweight.data[0].y, rweight.data.size(), 0, 0, sizeof(ImVec2));
        ImPlot::EndPlot();
    }
}

int32_t main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (!window)
    {
        return 1;
    }

    if (glewInit() != GL_TRUE)
    {
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.BackendFlags |= ImGuiDockNodeFlags_PassthruCentralNode;

    std::array<ImVec2, 500> graph;
    float dx = graph.size() * 0.1f;
    for (size_t i = 0; i < graph.size(); i++)
    {
        graph.at(i).x = (float) i * 0.1f;
        float x = graph[i].x;
        graph.at(i).y = cos(x);
    }
    
    clock_type::initialize();
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        
        ImGui::NewFrame();
        {
            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
            ImGui::Begin("Fourier Interpolation");
            {
                float currentTime = clock_type::getTime() * 0.1f;
                float frequency = currentTime / (2 * std::numbers::pi_v<float>);
                ImGui::Text("Current time: %f", currentTime);
                ImGui::Text("Current frequency: %f", frequency);
                fi::PlotLine(graph, frequency, "Periodic");
                FreqEulersInterpretation(graph, frequency);
            }
            ImGui::End();
        }
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    clock_type::deinitialize();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}