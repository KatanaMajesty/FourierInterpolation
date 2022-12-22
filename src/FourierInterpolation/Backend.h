#pragma once

#include <array>
#include <ranges>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#define FUNCTION_DX 50.0f

namespace fi
{

    template<size_t Size>
    struct Context
    {
        bool initialized;
        GLFWwindow* window;
        ImGuiIO* io;
        std::array<ImVec2, Size> graph;

        void begin()
        {
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        }

        void end()
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwPollEvents();
            glfwSwapBuffers(window);
        }

        bool shouldClose() const { return glfwWindowShouldClose(window); }

        template<typename Functor>
        void setFunc(Functor&& functor)
        {
            // We get the offset between points in such way that the graph's length would be equal to FUCNTION_DX
            float dx = FUNCTION_DX / graph.size();
            for (size_t i : std::views::iota(size_t(0), Size))
            {
                float x = dx * i;
                graph[i] = ImVec2(x, functor(x));
            }
        }
    };

    template<size_t Size>
    Context<Size> initializeContext()
    {
        glfwInit();
        Context<Size> context;
        context.window = glfwCreateWindow(1920, 1080, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
        if (!context.window)
        {
            return { false, nullptr, nullptr };
        }
        if (glewInit() != GL_TRUE)
        {
            return { false, nullptr, nullptr };
        }

        glfwMakeContextCurrent(context.window);
        glfwSwapInterval(1); // Enable vsync

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(context.window, true);
        ImGui_ImplOpenGL3_Init("#version 150");
        ImPlot::CreateContext();
        
        context.io = &ImGui::GetIO();
        context.io->ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
        context.io->BackendFlags |= ImGuiDockNodeFlags_PassthruCentralNode;
        context.initialized = true;
        return context;
    }

}; // fi namespace