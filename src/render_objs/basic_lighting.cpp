//#include "basic_lighting.h"
//
//RENDERABLE_BEGIN
//BasicLightingObj::BasicLightingObj(Parser::RenderObjConfigNaive& config)
//{
//	SetUpData();
//	SetUpShader();
//}
//
//
//void BasicLightingObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
//{
//}
//
//void BasicLightingObj::ImGuiCallback()
//{
//	static glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
//	//ImGui::SliderFloat3("lightPos", &lightPos,m)
//}
//
//void BasicLightingObj::SetUpData()
//{
//	std::vector<float> cubes{
//		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
//		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
//		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
//		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
//		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
//		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
//
//		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
//		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
//		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
//		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
//		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
//		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
//
//		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
//		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
//		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
//		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
//		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
//		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
//
//		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
//		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
//		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
//		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
//		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
//		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
//
//		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
//		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
//		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
//		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
//		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
//		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
//
//		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
//		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
//		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
//		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
//		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
//		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
//	};
//
//	std::vector<VertexInfo> vertex_info = std::vector<VertexInfo>{
//		{"aPos", 0, 3, GL_FLOAT, GL_FALSE, 1, 0},
//		{"aNormal", 1, 3, GL_FLOAT, GL_FALSE, 1, 3},
//	};
//
//	SetMesh(&cubes, &vertex_info);
//
//}
//
//void BasicLightingObj::SetUpShader()
//{
//	m_shader = std::make_unique<Shader>("./shader/learningopengl/3.1.light_cube_vs.glsl", "./shader/learningopengl/3.1.light_cube_fs.glsl");
//	m_lighting_shader = std::make_unique<Shader>("./shader/learningopengl/3.1.materials_vs.glsl", "./shader/learningopengl/3.1.materials_fs.glsl");
//
//}
//RENDERABLE_END