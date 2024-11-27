#pragma once
#include "./common.h"
REGISTER_BEGIN

class UniformSetter
{
public:
	using SetterFuncPtr = std::function<void(std::unordered_map<std::string, std::any>&)>;
	UniformSetter() {
		m_uniformSetter["projection"] = SetProjectionUniform;
		m_uniformSetter["view"] = SetViewUniform;
		m_uniformSetter["model"] = SetModelUniform;
		m_uniformSetter["camPos"] = SetCamPosUniform;
		m_uniformSetter["viewport"] = SetViewportUniform;
		m_uniformSetter["focal"] = SetFocalUniform;
		m_uniformSetter["tanFov"] = SetTanFovUniform;
		m_uniformSetter["projParams"] = SetProjParamsUniform;
		m_uniformSetter["3dgs_projection"] = Set3DGSProjectionUniform;
	}
	std::unordered_map<std::string, SetterFuncPtr>& GetSetter() { return m_uniformSetter; }
	bool Contain(const std::string& uniform) { return m_uniformSetter.contains(uniform); }
	SetterFuncPtr GetFunc(const std::string& uniform) { return m_uniformSetter[uniform]; }
private:
	static void SetProjectionUniform(std::unordered_map<std::string, std::any>& uniforms);
	static void SetViewUniform(std::unordered_map<std::string, std::any>& uniforms);
	static void SetModelUniform(std::unordered_map<std::string, std::any>& uniforms);
	static void SetCamPosUniform(std::unordered_map<std::string, std::any>& uniforms);
	static void SetViewportUniform(std::unordered_map<std::string, std::any>& uniforms);
	static void SetFocalUniform(std::unordered_map<std::string, std::any>& uniforms);
	static void SetTanFovUniform(std::unordered_map<std::string, std::any>& uniforms);
	static void SetProjParamsUniform(std::unordered_map<std::string, std::any>& uniforms);
	static void Set3DGSProjectionUniform(std::unordered_map <std::string, std::any>& uniforms);

	std::unordered_map<std::string, SetterFuncPtr> m_uniformSetter = {};
};

REGISTER_END