
#include "precomp.h"
#include "connection_data.h"

ConnectionData::ConnectionData()
: connection(0), logged_in(false)
{
}

ConnectionData *ConnectionData::get_data(CL_NetGameConnection *connection)
{
	return reinterpret_cast<ConnectionData*>(connection->get_data(L"data"));
}

ConnectionData *ConnectionData::create_data(CL_NetGameConnection *connection)
{
	std::auto_ptr<ConnectionData> data(new ConnectionData());
	data->connection = connection;
	connection->set_data(L"data", data.get());
	return data.release();
}

void ConnectionData::destroy_data(CL_NetGameConnection *connection)
{
	delete get_data(connection);
	connection->set_data(L"data", 0);
}
