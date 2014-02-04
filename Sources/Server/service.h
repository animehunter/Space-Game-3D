
#pragma once

class Service : public CL_Service
{
public:
	Service();
	void service_start(std::vector<CL_String> &args);
	void service_stop();
	void service_reload();

private:
	void thread_main();

	CL_Event stop_event;
	CL_Thread thread;
};
