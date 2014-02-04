
#pragma once

#include "RenderFramework/position.h"
#include "RenderFramework/orientation.h"

class Skybox;
class Debris;
class Planet;
class Model;

class GameView : public CL_GUIComponent
{
public:
	GameView(CL_GUIManager *gui);
	~GameView();

private:
	void handle_input(float &dz, float &dx, float &dy, float &ry, float &rx, float &rz);
	void on_render(CL_GraphicContext &gc, const CL_Rect &update_rect);
	void on_netevent(const CL_NetGameEvent &e);
	void on_timer();
	bool on_input_pressed(const CL_InputEvent &e);
	bool on_input_released(const CL_InputEvent &e);
	static CL_GUITopLevelDescription get_toplevel_description();

	struct Ship
	{
		CL_String id;
		CL_Vec3f pos;
		CL_Vec4f dir;
	};

	Position our_position;
	Orientation our_orientation;
	//CL_NetGameClient client;
	std::vector<Ship> ships;
	float thrust;

	CL_Image crosshair;
	CL_Image shields;
	std::auto_ptr<Skybox> skybox;
	std::auto_ptr<Debris> debris;
	std::auto_ptr<Planet> planet;
	std::auto_ptr<Model> ship;
	std::auto_ptr<Model> asteroid3ds1;
	std::auto_ptr<Model> asteroid3ds2;
	std::auto_ptr<Model> asteroid3ds3;
	std::auto_ptr<Model> asteroid3ds4;
	CL_Timer timer;
	CL_SlotContainer slots;
	std::vector<Position> asteroid_positions;
	Orientation o_asteroid[7];
	CL_Font font;
	std::vector<Position> positions;
	std::vector<Orientation> orientations;

	struct InputButton
	{
		InputButton() : down(false) { }
		bool down;
	};

	InputButton more_thrust;
	InputButton less_thrust;
	InputButton turn_left;
	InputButton turn_right;
	InputButton turn_up;
	InputButton turn_down;
	InputButton slide_left;
	InputButton slide_right;
	InputButton slide_up;
	InputButton slide_down;
	InputButton tilt_left;
	InputButton tilt_right;
};
