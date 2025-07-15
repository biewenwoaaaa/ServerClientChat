#include "client.h"
#include "clientStateTranslation.h"

Client::Client(MuduoEventLoop* loop, const MuduoInetAddress& serverAddr, int& pipefd) :
	m_client(loop, serverAddr, "ChatClient"),
	m_loop(loop),
	m_loginSuccess(false)
{
	m_client.setConnectionCallback(std::bind(&Client::m_onConnection, this, std::placeholders::_1));
	m_client.setMessageCallback   (std::bind(&Client::m_onMessage   , this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	m_mapFunction[ENUM_MSGTYPE::REGISTOR_ACK     ] = std::bind(&Client::m_CallregistUser,    this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::LOGIN_ACK        ] = std::bind(&Client::m_CallloginUser,     this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::ADDFRIEND_ACK    ] = std::bind(&Client::m_CalladdFriend,     this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::DELFRIEND_ACK    ] = std::bind(&Client::m_CalldeleteFriend,  this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::GETFRIENDLIST_ACK] = std::bind(&Client::m_CallgetFriendList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::JOIN_GROUP_ACK   ] = std::bind(&Client::m_CalljoinGroup,     this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::LEAVE_GROUP_ACK  ] = std::bind(&Client::m_CallleaveGroup,    this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::CREATE_GROUP_ACK ] = std::bind(&Client::m_CallcreateGroup,   this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::PEER_CHAT_ACK    ] = std::bind(&Client::m_CallPeerChat,      this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::GROUP_CHAT_ACK   ] = std::bind(&Client::m_CallgroupChat,     this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	m_readPipeFd = pipefd; // Store the pipe file descriptor for reading messages
}

Client::~Client()
{
	
	if (m_mainMenuThread && m_mainMenuThread->joinable())//wait for the main menu thread to finish
	{
		m_mainMenuThread->join();
	}

	LOG_INFO << "Client destructor called, cleaning up resources." ;
}

void Client::m_connect()
{
	m_client.connect();
}

void Client::m_disconnect()
{
	m_client.disconnect();
}

void Client::m_write(const std::string& message)
{
	if(m_connection)
	{
		m_connection->send(message);
	}
	else
	{
		LOG_ERROR << "Connection is not established, cannot send message: " << message;
	}
}

void Client::m_onConnection(const MuduoTcpConnectionPtr& conn)
{
	if(conn->connected())
	{
		LOG_INFO << "Connected to server: " << conn->peerAddress().toIpPort();
		m_connection = conn;

		/// Start the main menu thread after a successful connection
		if (m_mainMenuThread == nullptr)
		{
			m_mainMenuThread = std::make_unique<std::thread>([&]()
															 { m_mainMenu(); });
		}
	}
	else
	{
		LOG_INFO << "Disconnected from server: " << conn->peerAddress().toIpPort();
		m_connection.reset();

		m_loginSuccess.store(false); // Reset login success state on disconnection

	}
}

void Client::m_onMessage(const MuduoTcpConnectionPtr& conn, MuduoNetBuffer* buf, MuduoTimeStamp receiveTime)
{
	if(!m_connection || !m_connection->connected())
	{
		LOG_ERROR << "Connection is not established, cannot process message.";
		return;
	}
	
	std::string message = buf->retrieveAllAsString();

	nlohmann::json jsonMessage = nlohmann::json::parse(message);

	auto msgType = jsonMessage["MsgType"].get<ENUM_MSGTYPE>();  //deserialize the message type
	
	m_mapFunction[msgType](conn, message, receiveTime);
}

void Client::m_mainMenu()
{
	//while (!m_mainMenuThreadStop) {

	//}//when exit loop, it means the main menu thread is stopped
	//

	//enter state mechanism of the main menu
	
	Context ctx(std::bind(&Client::m_write,this,std::placeholders::_1),m_loginSuccess,m_readPipeFd);
	ctx.m_setCurrentState(std::make_unique<MainMenuState>()); //set the initial state of the state machine

	m_loop->quit();//stop the event loop

}

void Client::m_CallregistUser(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	
	nlohmann::json object = nlohmann::json::parse(buf);

	LOG_INFO << object["message"].get<std::string>();
	
}

void Client::m_CallloginUser(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	if(object["message"].is_string())
	{
		LOG_INFO << object["message"].get<std::string>();
	}
	else if(object["message"].is_array())
	{
		for (const auto& msg : object["message"]) {
			LOG_INFO << msg.get<std::string>();
		}	
	}
	
	m_loginSuccess = object["result"].get<bool>();
}

void Client::m_CalladdFriend(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	LOG_INFO << object["message"].get<std::string>();
}

void Client::m_CalldeleteFriend(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	LOG_INFO << object["message"].get<std::string>();
}

void Client::m_CallgetFriendList(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	LOG_INFO << "Friend List: " << object["friends"].dump();

}

void Client::m_CalljoinGroup(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	LOG_INFO << object["message"].get<std::string>();
}

void Client::m_CallleaveGroup(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	LOG_INFO << object["message"].get<std::string>();
}

void Client::m_CallcreateGroup(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	LOG_INFO << object["message"].get<std::string>();
}

void Client::m_CallPeerChat(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	LOG_INFO << object["message"].get<std::string>();

	// Here you can handle the peer chat message, e.g., display it in the UI or log it
}

void Client::m_CallgroupChat(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	if(object.contains("result") && !object["result"].get<bool>())
	{
		LOG_ERROR << object["message"].get<std::string>();
		return;
	}
	LOG_INFO << object["groupId"].get<int>() << "::"<< object["senderId"].get<int>()<<":" << object["message"].get<std::string>();

	// Here you can handle the group chat message, e.g., display it in the UI or log it
}
