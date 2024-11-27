#include "register_uniform_setter.h"
#include "../draw/camera.h"
#include "../render_objs/ply_obj.h"
REGISTER_BEGIN


void UniformSetter::SetProjectionUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms.emplace("projection", instance->GetProjectionMatrix());

}

void UniformSetter::SetViewUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms.emplace("view", instance->GetViewMatrix());
}

void UniformSetter::SetModelUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms.emplace("model", glm::mat4(1.0f));
}

void UniformSetter::SetCamPosUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms.emplace("camPos", instance->GetPosition());
}

void UniformSetter::SetViewportUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms.emplace("viewport", glm::vec2(instance->GetScreenWidth(), instance->GetScreenHeight()));
}

void UniformSetter::SetFocalUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Renderable::Base3DGSCamera::GetInstance();
	uniforms.emplace("focal", instance->GetTanFov());
}

void UniformSetter::SetTanFovUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Renderable::Base3DGSCamera::GetInstance();
	uniforms.emplace("tanFov", instance->GetTanFov());
}

void UniformSetter::SetProjParamsUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Renderable::Base3DGSCamera::GetInstance();
	uniforms.emplace("projParams", instance->GetProjParams());
}

void UniformSetter::Set3DGSProjectionUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Renderable::Base3DGSCamera::GetInstance();
	uniforms.emplace("3dgs_projection", instance->GetProjection());
}
REGISTER_END
