
#include "precomp.h"
#include "game_server.h"
#include "connection_data.h"

GameServer::GameServer()
: next_id(0)
{
	slots.connect(netgame_server.sig_client_connected(), this, &GameServer::on_client_connected);
	slots.connect(netgame_server.sig_client_disconnected(), this, &GameServer::on_client_disconnected);
	slots.connect(netgame_server.sig_event_received(), this, &GameServer::on_event_received);

	event_dispatcher.func_event(L"Client-Login").set(this, &GameServer::on_event_login);
	event_dispatcher.func_event(L"Client-PositionUpdate").set(this, &GameServer::on_event_position_update);

	CL_Console::write_line(L"Started listen on port 5000");
	netgame_server.start(L"5000");
}

GameServer::~GameServer()
{
	CL_Console::write_line(L"Shutting down server..");
	netgame_server.stop();
}

void GameServer::on_client_connected(CL_NetGameConnection *connection)
{
	CL_Console::write_line(L"Client connected");

	ConnectionData *data = ConnectionData::create_data(connection);
	data->id = CL_StringHelp::int_to_text(next_id++);
	connections.push_back(ConnectionData::get_data(connection));
}

void GameServer::on_client_disconnected(CL_NetGameConnection *connection)
{
	CL_Console::write_line(L"Client disconnected");

	ConnectionData *data = ConnectionData::get_data(connection);
	for (size_t i = 0; i < connections.size(); i++)
	{
		if (connections[i] != data && connections[i]->logged_in)
		{
			connections[i]->connection->send_event(CL_NetGameEvent(L"Server-PlayerLeft", data->id));
		}
	}

	connections.erase(std::find(connections.begin(), connections.end(), data));
	ConnectionData::destroy_data(connection);
}

void GameServer::on_event_received(CL_NetGameConnection *connection, const CL_NetGameEvent &e)
{
	if (!event_dispatcher.dispatch(e, ConnectionData::get_data(connection)))
		CL_Console::write_line(L"Unknown event: %1", e.to_string());
}

void GameServer::on_event_login(const CL_NetGameEvent &e, ConnectionData *data)
{
	CL_Console::write_line(L"Client logged in");

	data->logged_in = true;
	data->connection->send_event(CL_NetGameEvent(L"Server-LoginComplete"));
	for (size_t i = 0; i < connections.size(); i++)
	{
		if (connections[i] != data && connections[i]->logged_in)
		{
			data->connection->send_event(CL_NetGameEvent(L"Server-PlayerJoined", L"unused", L"unused", connections[i]->id));
			connections[i]->connection->send_event(CL_NetGameEvent(L"Server-PlayerJoined", L"unused", L"unused", data->id));
		}
	}
}

void GameServer::on_event_position_update(const CL_NetGameEvent &e, ConnectionData *data)
{
	CL_Vec3f pos(e.get_argument(0), e.get_argument(1), e.get_argument(2));
	CL_Vec4f dir(e.get_argument(3), e.get_argument(4), e.get_argument(5), e.get_argument(6));

	CL_NetGameEvent update(L"Server-SetPlayerPosition");
	update.add_argument(pos.x);
	update.add_argument(pos.y);
	update.add_argument(pos.z);
	update.add_argument(dir.x);
	update.add_argument(dir.y);
	update.add_argument(dir.z);
	update.add_argument(dir.w);
	update.add_argument(data->id);
	for (size_t i = 0; i < connections.size(); i++)
	{
		if (connections[i] != data && connections[i]->logged_in)
			connections[i]->connection->send_event(update);
	}
}
