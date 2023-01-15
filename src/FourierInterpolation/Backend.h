#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

namespace ft
{

    struct Context
    {
        bool initialized;
        GLFWwindow* window;
        ImGuiIO* io;

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
    };

    Context initializeContext()
    {
        glfwInit();
        Context context;
        context.window = glfwCreateWindow(1920, 1080, "Fourier Interpolation with Zero-Padding Frequency domain", NULL, NULL);
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

}; // ft namespace