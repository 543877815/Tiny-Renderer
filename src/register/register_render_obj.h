#pragma once
#include "../render_objs/common.h"
#include "../render_objs/aabb_obj.h"
#include "../render_objs/axis_obj.h"
#include "../render_objs/box_obj.h"
#include "../render_objs/ellipsoid_obj.h"
#include "../render_objs/sphere_obj.h"
#include "../render_objs/rectangle2d_obj.h"
#include "../render_objs/map_obj.h"
#include "../parser/config_parser.h"
#include "./common.h"
#include <functional>
#include <memory>
REGISTER_BEGIN
using ConfigReturnType = std::shared_ptr<Renderable::RenderObjectBase>;
using ConfigParamType = std::shared_ptr<Parser::RenderObjConfigBase>;
using CreateRenderObjFuncPtr = std::function<ConfigReturnType(ConfigParamType)>;

class RenderObjectFactory {
public:
	static std::unordered_map<std::string, CreateRenderObjFuncPtr>& GetRegisterRenderObj() { return register_obj; };

private:
	static std::shared_ptr<Renderable::RenderObjectBase> CreateAxis(std::shared_ptr<Parser::RenderObjConfigBase>);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateBox(std::shared_ptr<Parser::RenderObjConfigBase>);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateEllipsoid(std::shared_ptr<Parser::RenderObjConfigBase>);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateSphere(std::shared_ptr<Parser::RenderObjConfigBase>);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateRectangle2D(std::shared_ptr<Parser::RenderObjConfigBase>);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateMap(std::shared_ptr<Parser::RenderObjConfigBase>);

	static std::unordered_map<std::string, CreateRenderObjFuncPtr> register_obj;
};


REGISTER_END