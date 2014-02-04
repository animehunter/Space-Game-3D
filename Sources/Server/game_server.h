
#pragma once

class ConnectionData;

class GameServer
{
public:
	GameServer();
	~GameServer();

private:
	void on_client_connected(CL_NetGameConnection *connection);
	void on_client_disconnected(CL_NetGameConnection *connection);
	void on_event_received(CL_NetGameConnection *connection, const CL_NetGameEvent &e);
	void on_event_login(const CL_NetGameEvent &e, ConnectionData *data);
	void on_event_position_update(const CL_NetGameEvent &e, ConnectionData *data);

	CL_NetGameServer netgame_server;
	CL_SlotContainer slots;
	std::vector<ConnectionData *> connections;
	CL_NetGameEventDispatcher_v1<ConnectionData *> event_dispatcher;
	int next_id;
};
