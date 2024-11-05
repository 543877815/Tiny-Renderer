#pragma once
#include "../render_objs/common.h"
#include "../render_objs/aabb_obj.h"
#include "../render_objs/axis_obj.h"
#include "../render_objs/box_obj.h"
#include "../render_objs/ellipsoid_obj.h"
#include "../render_objs/sphere_obj.h"
#include "../render_objs/rectangle2d_obj.h"
#include "../render_objs/map_obj.h"
#include "./common.h"
#include "./register_config.h"
#include <functional>
#include <memory>
REGISTER_BEGIN
using CreateRenderObjFuncPtr = std::function<std::shared_ptr<Renderable::RenderObjectBase>(RenderObjConfig&)>;

class RenderObjectFactory {
public:
	static std::unordered_map<std::string, CreateRenderObjFuncPtr>& GetRegisterRenderObj() { return register_obj; };

private:
	static std::shared_ptr<Renderable::RenderObjectBase> CreateAxis(RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateBox(RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateEllipsoid(RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateSphere(RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateRectangle2D(RenderObjConfig&);
	static std::shared_ptr<Renderable::RenderObjectBase> CreateMap(RenderObjConfig&);

	static std::unordered_map<std::string, CreateRenderObjFuncPtr> register_obj;
};


REGISTER_END