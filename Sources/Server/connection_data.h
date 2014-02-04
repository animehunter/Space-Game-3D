
#pragma once

class ConnectionData
{
public:
	ConnectionData();

	CL_NetGameConnection *connection;
	CL_String id;
	bool logged_in;

	static ConnectionData *get_data(CL_NetGameConnection *connection);
	static ConnectionData *create_data(CL_NetGameConnection *connection);
	static void destroy_data(CL_NetGameConnection *connection);
};
