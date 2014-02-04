
#include "precomp.h"
#include "program.h"
#include "GameView/game_view.h"

CL_ClanApplication clanapp(&Program::main);

int Program::main(const std::vector<CL_String> &args)
{
	try
	{
		CL_SetupCore setup_core;
		//CL_SetupNetwork setup_network;
		CL_SetupDisplay setup_display;
		CL_SetupGL setup_gl;

		Program p;
		p.exec();
		return 0;
	}
	catch (CL_Exception &e)
	{
		MessageBox(0, e.get_message_and_stack_trace().c_str(), "Unhandled Exception", MB_ICONERROR|MB_OK);
		return 1;
	}
}

Program::Program()
{
}

void Program::run_gui(CL_DisplayWindow &display_window)
{
//	CL_ResourceManager local_resources("Resources/resources.xml");
	CL_ResourceManager local_resources;
	CL_String resource_filename("Resources/HUD/GUITheme/resources.xml");
	CL_String theme_filename("Resources/HUD/GUITheme/theme.css");

	CL_ResourceManager resources(resource_filename);
	local_resources.add_resources(resources);
	CL_GUIThemeDefault theme;
	theme.set_resources(local_resources);
	CL_GUIWindowManagerTexture wm(display_window);
	gui.reset(new CL_GUIManager());
	gui->set_window_manager(wm);
	gui->set_theme(theme);
	gui->set_css_document(theme_filename);

	CL_Size size = display_window.get_gc().get_size();
	game_view.reset(new GameView(gui.get()));
	game_view->set_geometry(size);
	gui->exec();
	game_view.reset();
	gui.reset();
}

void Program::exec()
{
	AddFontResourceEx("Resources/HUD/duxclock.ttf", FR_PRIVATE, 0);

	CL_OpenGLWindowDescription windowdesc;
	windowdesc.set_title("Space Game");
	//windowdesc.set_depth_size(16);
	windowdesc.set_size(CL_Size(800, 600), true);
	windowdesc.set_allow_resize(true);
	//windowdesc.set_multisampling(4);
	window = CL_DisplayWindow(windowdesc);
	CL_Slot slot = window.sig_window_close().connect(this, &Program::on_close);
	CL_Slot slot2 = window.sig_resize().connect(this, &Program::on_resize);
	run_gui(window);
	window = CL_DisplayWindow();
}

void Program::on_close()
{
	gui->exit_with_code(0);
}

void Program::on_resize(int,int)
{
	CL_Size size = window.get_gc().get_size();
	game_view->set_geometry(size);
}
