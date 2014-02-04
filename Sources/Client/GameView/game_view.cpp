
#include "precomp.h"
#include "game_view.h"
#include "Skybox/skybox.h"
#include "Model/model.h"
#include "Debris/debris.h"
//#include "Asteroid/asteroid.h"
#include "Planet/planet.h"
#include "RenderFramework/position.h"
#include "RenderFramework/light_model.h"
#include "RenderFramework/orientation.h"
#include "RenderFramework/camera.h"

GameView::GameView(CL_GUIManager *gui)
: CL_GUIComponent(gui, get_toplevel_description())
{
	func_render().set(this, &GameView::on_render);
	func_input_pressed().set(this, &GameView::on_input_pressed);
	func_input_released().set(this, &GameView::on_input_released);
	set_constant_repaint(true);
	set_focus();

	CL_GraphicContext gc = get_gc();
	crosshair = CL_Image(gc, "Resources/HUD/crosshair.png");
	shields = CL_Image(gc, "Resources/HUD/shields.png");
	skybox.reset(new Skybox(gc));
	debris.reset(new Debris(gc));
	planet.reset(new Planet(gc));
	ship.reset(new Model(gc, "Resources/Model/Ship/ship.3ds", Model::read_from_file));
	asteroid3ds1.reset(new Model(gc, "Resources/Model/Asteroid/asteroid1.3ds", Model::read_from_file));
	asteroid3ds2.reset(new Model(gc, "Resources/Model/Asteroid/asteroid2.3ds", Model::read_from_file));
	asteroid3ds3.reset(new Model(gc, "Resources/Model/Asteroid/asteroid3.3ds", Model::read_from_file));
	asteroid3ds4.reset(new Model(gc, "Resources/Model/Asteroid/asteroid4.3ds", Model::read_from_file));

//	slots.connect(client.sig_event_received(), this, &GameView::on_netevent);
	//client.connect(L"68.231.206.82", L"5566");
//	client.connect(L"127.0.0.1", L"5000");
//	client.send_event(CL_NetGameEvent("Client-Login", "unused", "unused"));

	timer.func_expired().set(this, &GameView::on_timer);
	timer.start(50, true);

	asteroid3ds1->set_scale(0.1f);
	asteroid3ds2->set_scale(0.4f);
	asteroid3ds3->set_scale(0.3f);
	asteroid3ds4->set_scale(0.2f);
	ship->set_scale(0.1f);

	CL_Vec3f eye(0.0f, 0.0f, -10.0f);
	CL_Vec3f center(0.0f, 0.0f, 0.0f);
	CL_Vec3f up(0.0f, 1.0f, 0.0f);
	our_position = Position(eye.x, eye.y, eye.z);
	our_orientation = Orientation::look_at(eye, center, up);
	our_orientation.rotate(180.0f, CL_Vec3f(0.0f, 0.0f, 1.0f));
	our_orientation.rotate(30.0f, CL_Vec3f(1.0f, 0.0f, 0.0f));
	thrust = 0.0f;

	for (int i = 0; i < 50; i++)
	{
		asteroid_positions.push_back(Position(
			(rand()/(float)RAND_MAX)*1000-500,
			(rand()/(float)RAND_MAX)*1000-500,
			(rand()/(float)RAND_MAX)*1000-500));
	}

	//font = CL_Font_Freetype(gc, L"Duxus Clock Regular", 24, CL_File(L"Resources/HUD/duxclock.ttf", CL_File::open_existing, CL_File::access_read, CL_File::share_read));
	font = CL_Font(gc, L"Duxus Clock", 24);

	/*CL_GUITopLevelDescription desc;
	desc.set_position(CL_Rect(50,50,400,400), false);
	CL_Window *window = new CL_Window(this, desc);
	window->set_title(L"");
    window->set_draggable(true);
    CL_Label *l = new CL_Label(window);
    l->set_text("Drag me around");
    l->set_geometry(CL_Rect(10,10,50,20));*/
}

GameView::~GameView()
{
}

CL_GUITopLevelDescription GameView::get_toplevel_description()
{
	CL_GUITopLevelDescription desc;
	desc.set_size(CL_Size(640, 480), true);
	desc.set_depth_size(16);
	return desc;
}

void GameView::on_render(CL_GraphicContext &gc, const CL_Rect &update_rect)
{
	float dx = 0.0f, dy = 0.0f, dz = 0.0f;
	float rx = 0.0f, ry = 0.0f, rz = 0.0f;
	handle_input(dz, dx, dy, ry, rx, rz);

	our_orientation.rotate(rx, CL_Vec3f(1.0f, 0.0f, 0.0f));
	our_orientation.rotate(ry, CL_Vec3f(0.0f, 1.0f, 0.0f));
	our_orientation.rotate(rz, CL_Vec3f(0.0f, 0.0f, 1.0f));
	our_position.translate(our_orientation, dx, dy, dz);

	Camera camera(55.0f, gc.get_width()/(float)gc.get_height());
	camera.set_position(our_position);
	camera.set_orientation(our_orientation);

	gc.clear(CL_Colorf::gray);
	skybox->render(gc, camera);
	camera.setup_gc(gc, 1.0f, 1000.0f);

	CL_BufferControl buffer_control;
	buffer_control.enable_depth_test(true);
	buffer_control.enable_depth_write(true);
	gc.set_buffer_control(buffer_control);
	gc.clear_depth(1.0);

	LightModel lightmodel;
	LightSource light0;
	light0.constant_attenuation = 0.0002f;
	light0.set_position(camera.get_modelview(), CL_Vec4f(2000.0f, 2015.0f, 2025.0f, 1.0f));
	lightmodel.scene_ambient = CL_Vec4f(0.2f, 0.2f, 0.2f, 1.0f);
	lightmodel.light_sources.push_back(light0);

	planet->render(gc, lightmodel);

	for (size_t i = 0; i < asteroid_positions.size(); i++)
	{
		switch (i%4)
		{
		case 0: asteroid3ds1->render(gc, lightmodel, asteroid_positions[i], o_asteroid[i%7]); break;
		case 1: asteroid3ds2->render(gc, lightmodel, asteroid_positions[i], o_asteroid[i%7]); break;
		case 2: asteroid3ds3->render(gc, lightmodel, asteroid_positions[i], o_asteroid[i%7]); break;
		case 3: asteroid3ds4->render(gc, lightmodel, asteroid_positions[i], o_asteroid[i%7]); break;
		}
	}
	o_asteroid[0].rotate(2.0f, CL_Vec3f(0.6f, 0.3f, -0.2f).normalize());
	o_asteroid[1].rotate(2.5f, CL_Vec3f(-0.6f, 0.3f, -0.3f).normalize());
	o_asteroid[2].rotate(0.4f, CL_Vec3f(0.3f, 0.7f, -0.6f).normalize());
	o_asteroid[3].rotate(4.0f, CL_Vec3f(0.4f, 0.3f, -0.2f).normalize());
	o_asteroid[4].rotate(7.0f, CL_Vec3f(0.6f, -0.3f, -0.5f).normalize());
	o_asteroid[5].rotate(1.0f, CL_Vec3f(0.2f, 0.5f, -0.7f).normalize());
	o_asteroid[6].rotate(2.0f, CL_Vec3f(0.6f, 0.6f, -0.2f).normalize());

	debris->render(gc, lightmodel, CL_Vec3f(our_position.get_x(), our_position.get_y(), our_position.get_z()));

	for (size_t i = 0; i < ships.size(); i++)
	{
		Orientation o = Orientation::quaternion(ships[i].dir.x, ships[i].dir.y, ships[i].dir.z, ships[i].dir.w);
		o.multiply(Orientation::axis_angle(180.0f, CL_Vec3f(0.0f, 0.0f, 1.0f)));
		ship->render(gc, lightmodel, Position(ships[i].pos.x, ships[i].pos.y, ships[i].pos.z), o);
	}

	gc.reset_buffer_control();

	gc.set_projection(CL_Mat4f::identity());
	gc.set_modelview(CL_Mat4f::identity());
	gc.set_map_mode(cl_map_2d_upper_left);

	crosshair.draw(gc, gc.get_width()/2-crosshair.get_width()/2, gc.get_height()/2-crosshair.get_height()/2);
	shields.draw(gc, CL_Rectf(gc.get_width()-shields.get_width()*2-50, gc.get_height()-shields.get_height()*2-50, CL_Sizef(shields.get_width()*2, shields.get_height()*2)));

	font.draw_text(gc, 10, 25, cl_format(L"X = %1", our_position.get_x()), CL_Colorf::lightgreen);
	font.draw_text(gc, 10, 25+21, cl_format(L"Y = %1", our_position.get_y()), CL_Colorf::lightgreen);
	font.draw_text(gc, 10, 25+2*21, cl_format(L"Z = %1", our_position.get_z()), CL_Colorf::lightgreen);
}

void GameView::handle_input(float &dz, float &dx, float &dy, float &ry, float &rx, float &rz)
{
	if (more_thrust.down)
		thrust += 0.02f;
	else if (less_thrust.down)
		thrust -= 0.02f;

	dz += thrust;

	if (slide_left.down)
		dx += 0.2f;
	if (slide_right.down)
		dx -= 0.2f;
	if (slide_up.down)
		dy += 0.2f;
	if (slide_down.down)
		dy -= 0.2f;

	if (turn_left.down)
		ry -= 1.2f;
	if (turn_right.down)
		ry += 1.2f;
	if (turn_up.down)
		rx -= 0.8f;
	if (turn_down.down)
		rx += 0.8f;
	if (tilt_right.down)
		rz += 0.8f;
	if (tilt_left.down)
		rz -= 0.8f;
}

void GameView::on_netevent(const CL_NetGameEvent &e)
{
	try
	{
		if (e.get_name() == "Server-LoginComplete")
		{
		}
		else if (e.get_name() == "Server-SetPlayerPosition")
		{
			for (size_t i = 0; i < ships.size(); i++)
			{
				if (ships[i].id == e.get_argument(7).to_string())
				{
					float x = e.get_argument(0);
					float y = e.get_argument(1);
					float z = e.get_argument(2);
					float dirx = e.get_argument(3);
					float diry = e.get_argument(4);
					float dirz = e.get_argument(5);
					float dirw = e.get_argument(6);
					ships[i].pos = CL_Vec3f(x,y,z);
					ships[i].dir = CL_Vec4f(dirx,diry,dirz,dirw);
				}
			}
		}
		else if (e.get_name() == "Server-PlayerJoined")
		{
			Ship ship;
			ship.id = e.get_argument(2);
			ships.push_back(ship);
		}
		else if (e.get_name() == "Server-PlayerLeft")
		{
			for (size_t i = 0; i < ships.size(); i++)
			{
				if (ships[i].id == e.get_argument(0).to_string())
				{
					ships.erase(ships.begin()+i);
					break;
				}
			}
		}
	}
	catch (CL_Exception &)
	{
	}
}

void GameView::on_timer()
{
	float x = our_position.get_x();
	float y = our_position.get_y();
	float z = our_position.get_z();
	float dirx = our_orientation.get_x();
	float diry = our_orientation.get_y();
	float dirz = our_orientation.get_z();
	float dirw = our_orientation.get_w();
	CL_NetGameEvent e(L"Client-PositionUpdate");
	e.add_argument(x);
	e.add_argument(y);
	e.add_argument(z);
	e.add_argument(dirx);
	e.add_argument(diry);
	e.add_argument(dirz);
	e.add_argument(dirw);
//	client.send_event(e);
}

bool GameView::on_input_pressed(const CL_InputEvent &e)
{
	switch (e.id)
	{
	case CL_KEY_W: more_thrust.down = true; break;
	case CL_KEY_S: less_thrust.down = true; break;
	case CL_KEY_LEFT: if (e.alt) slide_left.down = true; else turn_left.down = true; break;
	case CL_KEY_RIGHT: if (e.alt) slide_right.down = true; else turn_right.down = true; break;
	case CL_KEY_UP: if (e.alt) slide_down.down = true; else turn_down.down = true; break;
	case CL_KEY_DOWN: if (e.alt) slide_up.down = true; else turn_up.down = true; break;
	case CL_KEY_A: tilt_left.down = true; break;
	case CL_KEY_D: tilt_right.down = true; break;
	}
	return true;
}

bool GameView::on_input_released(const CL_InputEvent &e)
{
	switch (e.id)
	{
	case CL_KEY_W: more_thrust.down = false; break;
	case CL_KEY_S: less_thrust.down = false; break;
	case CL_KEY_LEFT: slide_left.down = false; turn_left.down = false; break;
	case CL_KEY_RIGHT: slide_right.down = false; turn_right.down = false; break;
	case CL_KEY_UP: slide_down.down = false; turn_down.down = false; break;
	case CL_KEY_DOWN: slide_up.down = false; turn_up.down = false; break;
	case CL_KEY_A: tilt_left.down = false; break;
	case CL_KEY_D: tilt_right.down = false; break;
	}
	return true;
}
