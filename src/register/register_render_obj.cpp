#include "register_render_obj.h"

namespace renderable {
#define DECLEAR_REGISTER_OBJECT(OBJNAME) \
	std::shared_ptr<RenderObjectBase> RenderObjectFactory::Create##OBJNAME##(){ \
	return std::make_shared<##OBJNAME##Obj>(); \
}

	DECLEAR_REGISTER_OBJECT(Axis);
	DECLEAR_REGISTER_OBJECT(Sphere);
	DECLEAR_REGISTER_OBJECT(Box);
	DECLEAR_REGISTER_OBJECT(Rectangle2D);
	DECLEAR_REGISTER_OBJECT(Ellipsoid);
	DECLEAR_REGISTER_OBJECT(Map);

	std::unordered_map<std::string, std::function<std::shared_ptr<RenderObjectBase>()>> register_obj{
		{"axis", RenderObjectFactory::CreateAxis},
		{"sphere", RenderObjectFactory::CreateSphere},
		{"box", RenderObjectFactory::CreateBox},
		{"rectangle2d", RenderObjectFactory::CreateRectangle2D},
		{"ellipsoid", RenderObjectFactory::CreateEllipsoid},
		{"map", RenderObjectFactory::CreateMap}
	};

	std::unordered_map<std::string, std::function<std::shared_ptr<RenderObjectBase>()>>& GetRegisterRenderObj()
	{
		return register_obj;
	}





};