#include "register_uniform_setter.h"
#include "../draw/camera.h"
#include "../render_objs/gs_ply_obj.h"
REGISTER_BEGIN


void UniformSetter::SetPerspectiveProjectionUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms["perspective_projection"] = instance->GetProjMat();
}

void UniformSetter::SetOrthogonalProjectionUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms["orthogonal_projection"] = instance->GetProjMat();
}

void UniformSetter::SetProjectionUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms["projection"] = instance->GetProjMat();
}

void UniformSetter::SetViewUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms["view"] = instance->GetViewMat();
}

void UniformSetter::SetModelUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms["model"] = glm::mat4(1.0f);
}

void UniformSetter::SetCamPosUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms["camPos"] = instance->GetPosition();
}

void UniformSetter::SetViewportUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Camera::GetInstance();
	uniforms["viewport"] = glm::vec2(instance->GetScreenWidth(), instance->GetScreenHeight());
}

void UniformSetter::SetFocalUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Renderable::Base3DGSCamera::GetInstance();
	uniforms["focal"] = instance->GetFocal();
}

void UniformSetter::SetTanFovUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Renderable::Base3DGSCamera::GetInstance();
	uniforms["tanFov"] = instance->GetTanFov();
}

void UniformSetter::SetNearFarUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Renderable::Base3DGSCamera::GetInstance();
	uniforms["nearFar"] = instance->GetNearFar();
}

void UniformSetter::Set3DGSProjectionUniform(std::unordered_map<std::string, std::any>& uniforms)
{
	auto instance = Renderable::Base3DGSCamera::GetInstance();
	uniforms["3dgs_projection"] = instance->GetProjection();
}
REGISTER_END
