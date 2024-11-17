#include "register_render_obj.h"

REGISTER_BEGIN
#define DECLEAR_REGISTER_OBJECT(OBJNAME)																									\
std::shared_ptr<Renderable::RenderObjectBase> RenderObjectFactory::Create##OBJNAME##(std::shared_ptr<Parser::RenderObjConfigBase> config)	\
{																																			\
	return std::make_shared<Renderable::##OBJNAME##Obj>(config);																			\
}

DECLEAR_REGISTER_OBJECT(Axis);
DECLEAR_REGISTER_OBJECT(Sphere);
DECLEAR_REGISTER_OBJECT(Box);
DECLEAR_REGISTER_OBJECT(Rectangle2D);
DECLEAR_REGISTER_OBJECT(Ellipsoid);
DECLEAR_REGISTER_OBJECT(Map);

std::unordered_map<std::string, CreateRenderObjFuncPtr> RenderObjectFactory::register_obj = {
		{"axis", CreateAxis},
		{"sphere", CreateSphere},
		{"box", CreateBox},
		{"rectangle2d", CreateRectangle2D},
		{"ellipsoid", CreateEllipsoid},
		{"map", CreateMap}
};

REGISTER_END