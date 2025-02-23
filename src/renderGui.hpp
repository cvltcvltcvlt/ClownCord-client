#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <atomic>

extern std::atomic<bool>joinedVC;

class Gui
{
public:
	void SetupImGui(GLFWwindow* window);
	void RenderUI();
	void LoginPage();
};

