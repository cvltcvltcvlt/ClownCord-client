#include "renderGui.hpp"
#include "getMicrophoneInput.hpp"
#include <iostream>
#include <thread>
#include "connection.hpp"

bool joinedVC = false;
std::thread recordingThread;
std::thread connectionThread;

void Gui::SetupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void startRecording() {
    setupAndStartRecording("output.wav");
}

void Gui::RenderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
    ImGui::Begin("ClownCord");

    ImGui::Text("Channels:");

    if (ImGui::Button("Join VC")) {
        if (!joinedVC) {
            joinedVC = true;
            // Ensure the previous connection is cleaned up before starting a new one
            if (connectionSuccessfull) {
                closeConnection();  // Close any previous connection if still active
            }

            connectionThread = std::thread(startConnection);
            connectionThread.detach();  // Detach to allow the thread to run independently

            if (connectionSuccessfull) {
                // Wait for the connection to complete before starting the recording
                recordingThread = std::thread(startRecording);
                recordingThread.detach();  // Detach recording thread to run independently
                std::cout << "Joined VC and started recording!" << std::endl;
            }
            else {
                std::cout << "Connection error!" << std::endl;
            }
        }
    }

    if (ImGui::Button("Leave VC")) {
        if (joinedVC) {
            joinedVC = false;
            std::cout << "Left VC and stopped recording!" << std::endl;

            // Ensure that we close the connection and stop recording properly
            if (recordingThread.joinable()) {
                // Handle recording thread cleanup if necessary
                recordingThread.join();  // Wait for the recording thread to finish
            }

            if (connectionSuccessfull) {
                closeConnection();
            }
            else {
                std::cout << "Not connected to any voice chat!" << std::endl;
            }
        }
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


