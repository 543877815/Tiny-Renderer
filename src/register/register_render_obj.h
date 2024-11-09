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
using CreateRenderObjFuncPtr = std::function<std::shared_ptr<Renderable::RenderObjectBase>(Parser::RenderObjConfig&)>;

class RenderObjectFactory {
public:
	static std::unordered_map<std::string, CreateRenderObjFuncPtr>& GetRegisterRenderObj() { return register_obj; };

private:
	static std::shared_ptr<Renderable::RenderObjectBase> CreateAxis(Parser::RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateBox(Parser::RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateEllipsoid(Parser::RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateSphere(Parser::RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateRectangle2D(Parser::RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateMap(Parser::RenderObjConfig&);

	static std::unordered_map<std::string, CreateRenderObjFuncPtr> register_obj;
};


REGISTER_END