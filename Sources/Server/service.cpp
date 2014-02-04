
#include "precomp.h"
#include "service.h"
#include "game_server.h"

int main(int argc, char **argv)
{
	CL_SetupCore setup_core;
	CL_SetupNetwork setup_network;
	Service service;
	return service.main(argc, argv);
}

Service::Service()
: CL_Service(L"StarSkyServer")
{
}

void Service::service_start(std::vector<CL_String> &args)
{
	thread.start(this, &Service::thread_main);
}

void Service::service_stop()
{
	stop_event.set();
	thread.join();
}

void Service::service_reload()
{
}

void Service::thread_main()
{
	GameServer server;
	while (!stop_event.wait(0))
		CL_KeepAlive::process(500);
}
