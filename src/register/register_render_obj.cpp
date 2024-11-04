#include "register_render_obj.h"

REGISTER_BEGIN
#define DECLEAR_REGISTER_OBJECT(OBJNAME) \
	std::shared_ptr<Renderable::RenderObjectBase> RenderObjectFactory::Create##OBJNAME##(){ \
	return std::make_shared<Renderable::##OBJNAME##Obj>(); \
}

DECLEAR_REGISTER_OBJECT(Axis);
DECLEAR_REGISTER_OBJECT(Sphere);
DECLEAR_REGISTER_OBJECT(Box);
DECLEAR_REGISTER_OBJECT(Rectangle2D);
DECLEAR_REGISTER_OBJECT(Ellipsoid);
DECLEAR_REGISTER_OBJECT(Map);

std::unordered_map<std::string, std::function<std::shared_ptr<Renderable::RenderObjectBase>()>> register_obj{
	{"axis", RenderObjectFactory::CreateAxis},
	{"sphere", RenderObjectFactory::CreateSphere},
	{"box", RenderObjectFactory::CreateBox},
	{"rectangle2d", RenderObjectFactory::CreateRectangle2D},
	{"ellipsoid", RenderObjectFactory::CreateEllipsoid},
	{"map", RenderObjectFactory::CreateMap}
};

std::unordered_map<std::string, std::function<std::shared_ptr<Renderable::RenderObjectBase>()>>& GetRegisterRenderObj()
{
	return register_obj;
}
REGISTER_END