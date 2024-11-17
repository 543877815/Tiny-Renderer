#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>

// common
#include "manager/glfw_mgr.h"
#include "manager/imgui_mgr.h"
#include "manager/callback.h"
#include "manager/render_obj_mgr.h"
#include "parser/config.h"
#include "parser/config_parser.h"

// custom
#include "draw/camera.h"
#include "draw/render_main.h"

extern const int SCR_WIDTH;
extern const int SCR_HEIGHT;

int main() {
	auto render_main = RenderMain::GetInstance();
	std::vector<std::string> configs{ "./config/map.json" };
	render_main->SetupRenderObjs(configs);

	while (!glfwWindowShouldClose(render_main->GetWindow())) {
		render_main->PrepareDraw();
		render_main->Draw();
		render_main->FinishDraw();
	}

	return 0;
}

