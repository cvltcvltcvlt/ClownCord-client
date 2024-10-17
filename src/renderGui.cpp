#include "renderGui.hpp"
#include "getMicrophoneInput.hpp"
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<bool> joinedVC = false;
std::thread connectThread;
std::thread leaveThread;

void handleJoinVC() {
    if (!joinedVC) {
        joinedVC = true;  // Устанавливаем флаг
        if (connectThread.joinable()) {
            connectThread.join();  // Ожидание завершения предыдущего потока
        }
        connectThread = std::thread(startVoiceChat);  // Запускаем новый поток для голосового чата
    }
}

// Функция для выхода из голосового чата
void handleLeaveVC() {
    if (joinedVC) {
        joinedVC = false;  // Сбрасываем флаг
        stopVoiceChat();   // Останавливаем голосовой чат

        // Безопасное завершение потоков
        if (connectThread.joinable()) {
            connectThread.join();  // Дожидаемся завершения потока
        }
    }
}

void Gui::SetupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void Gui::RenderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
    ImGui::Begin("ClownCord");

    ImGui::Text("Channels:");

    // Handle Join VC button
    if (ImGui::Button("Join VC")) {
        handleJoinVC();
    }

    // Handle Leave VC button
    if (ImGui::Button("Leave VC")) {
        handleLeaveVC();
    }

    static float micVolume = 0.5f;
    ImGui::SliderFloat("Mic Volume", &micVolume, 0.0f, 1.0f);

    ImGui::Text("Voice Activity: ");
    ImGui::SameLine();
    ImGui::ProgressBar(0.2f, ImVec2(0.0f, 0.0f));

    ImGui::BeginChild("Child Window", ImVec2(0, 100), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::Text("This is a child window.");
    ImGui::EndChild();

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
