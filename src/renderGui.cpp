#include "renderGui.hpp"
#include "getMicrophoneInput.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <string>

std::atomic<bool> joinedVC = false;
std::thread connectThread;
std::thread leaveThread;

bool isLoggedIn = false; // Login state
std::string username;    // User input for username
std::string password;    // User input for password

void handleJoinVC() {
    if (!joinedVC) {
        joinedVC = true;
        if (connectThread.joinable()) {
            connectThread.join();
        }
        connectThread = std::thread(startVoiceChat);
    }
}

void handleLeaveVC() {
    if (joinedVC) {
        joinedVC = false;
        stopVoiceChat();
        if (connectThread.joinable()) {
            connectThread.join();
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

void MainPage() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
    ImGui::Begin("ClownCord");

    ImGui::Text("Channels:");

    if (ImGui::Button("Join VC")) {
        handleJoinVC();
    }

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

void Gui::LoginPage() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
    ImGui::Begin("Login to ClownCord");

    ImGui::Text("Enter your credentials to login.");

    static char usernameBuffer[64] = "";
    static char passwordBuffer[64] = "";

    // Username Input
    ImGui::InputText("Username", usernameBuffer, sizeof(usernameBuffer));

    // Password Input (with password masking)
    ImGui::InputText("Password", passwordBuffer, sizeof(passwordBuffer), ImGuiInputTextFlags_Password);

    // Handle Login Button
    if (ImGui::Button("Login")) {
        // Basic validation (in a real application, you would check these against a database or authentication service)
        if (std::string(usernameBuffer) == "user" && std::string(passwordBuffer) == "password") {
            isLoggedIn = true;
            username = usernameBuffer; // Store the logged-in username
            password = passwordBuffer; // Store the password (optional, for session handling)
        }
        else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Invalid username or password");
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::RenderUI() {
    if (!isLoggedIn) {
        LoginPage(); // Display the login page if not logged in
    }
    else {
        MainPage(); // Display the main page if logged in
    }
}
