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
#include <functional>
#include <memory>
REGISTER_BEGIN
class RenderObjectFactory {
public:
	static std::shared_ptr<Renderable::RenderObjectBase> CreateAxis();
	static std::shared_ptr<Renderable::RenderObjectBase> CreateSphere();
	static std::shared_ptr<Renderable::RenderObjectBase> CreateBox();
	static std::shared_ptr<Renderable::RenderObjectBase> CreateRectangle2D();
	static std::shared_ptr<Renderable::RenderObjectBase> CreateEllipsoid();
	static std::shared_ptr<Renderable::RenderObjectBase> CreateMap();
};

extern std::unordered_map<std::string, std::function<std::shared_ptr<Renderable::RenderObjectBase>()>> register_obj;
extern std::unordered_map<std::string, std::function<void()>> register_imguicallback;
std::unordered_map<std::string, std::function<std::shared_ptr<Renderable::RenderObjectBase>()>>& GetRegisterRenderObj();
REGISTER_END