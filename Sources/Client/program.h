
#pragma once

class GameView;

class Program
{
public:
	static int main(const std::vector<CL_String> &args);

private:
	Program();
	void run_gui(CL_DisplayWindow &display_window);
	void exec();
	void on_close();
	void on_resize(int,int);

	CL_DisplayWindow window;
	std::auto_ptr<CL_GUIManager> gui;
	std::auto_ptr<GameView> game_view;
};
