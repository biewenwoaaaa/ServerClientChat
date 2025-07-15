#include "server.h"
#include "fmt/format.h"

Server::Server(MuduoEventLoop* loop, const MuduoInetAddress& listenAddr):
    m_server(loop, listenAddr, "ChatServer"),
	m_RedisModel("192.168.8.100", 6379, "1234", 0) // Initialize RedisModel with host, port, password, and database index
{
    m_server.setConnectionCallback(
        [&](const MuduoTcpConnectionPtr& conn) {
            if (conn->connected()) {
                LOG_INFO << "New connection from " << conn->peerAddress().toIpPort();
            }
            else {
				int userid = -1; // Extract the user ID from the connection
				{
					std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
					for (auto &it : m_userConnections)
					{
						if (it.second == conn)
						{
							m_userConnections.erase(it.first); // Remove the user connection from the map
							LOG_INFO << "User " << it.first << " disconnected";
							userid = it.first; // Get the user ID from the connection

							m_RedisModel.m_unsubscribe(fmt::format("user_{}", it.first)); // Unsubscribe from the user's Redis channel
							break;
						}
					}
				}
				if (userid != -1) {
					// Update the user's state in the database to 'offline'
					std::string SQL = "UPDATE registUsers SET state = 'offline' WHERE id = %0";
					m_MySQLModel.m_CUD(SQL,userid);
				}
				conn->shutdown(); // Shutdown the connection

				LOG_INFO << "Connection disconnected";
            }
        });
	m_server.setMessageCallback(std::bind(&Server::m_onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
   
	m_server.setThreadNum(1);

	m_mapFunction[ENUM_MSGTYPE::REGISTOR]      = std::bind(&Server::m_CallregistUser,    this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::LOGIN]         = std::bind(&Server::m_CallloginUser,     this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::ADDFRIEND]     = std::bind(&Server::m_CalladdFriend,     this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::DELFRIEND]     = std::bind(&Server::m_CalldeleteFriend,  this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::GETFRIENDLIST] = std::bind(&Server::m_CallgetFriendList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::JOIN_GROUP]    = std::bind(&Server::m_CalljoinGroup,     this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::LEAVE_GROUP]   = std::bind(&Server::m_CallleaveGroup,    this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::CREATE_GROUP]  = std::bind(&Server::m_CallcreateGroup,   this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::PEER_CHAT]     = std::bind(&Server::m_CallPeerChat,      this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_mapFunction[ENUM_MSGTYPE::GROUP_CHAT]    = std::bind(&Server::m_CallgroupChat,     this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	m_RedisModel.m_setupCallbacks();
	
}

void Server::m_startServer() {
    m_server.start();
}



Server::~Server()
{
}


void Server::m_onMessage(const MuduoTcpConnectionPtr& conn, MuduoNetBuffer* buf, muduo::Timestamp time)
{
	// Handle incoming messages from clients
	std::string message = buf->retrieveAllAsString();
	

	nlohmann::json object=nlohmann::json::parse(message);

	auto key= object["MsgType"].get<ENUM_MSGTYPE>();

	m_mapFunction[key](conn, message, time);

}

bool Server::m_registUser(int id, std::string name, std::string password, std::string state)
{
	if (id <= 0 || name.empty() || password.empty() || state.empty()) {
		return false;
	}
	
	std::string SQL = "INSERT INTO registUsers VALUES (%0, %1q, %2q, %3q)";
	if (m_MySQLModel.m_CUD(SQL,id,name,password,state)) {
		LOG_INFO << "User registered successfully: " << name;
		return true;
	}
	else {
		LOG_ERROR << "Failed to register user: " << name;
		return false;
	}
}

bool Server::m_loginUser(int id, std::string password)
{
	if (id <= 0 || password.empty()) {
		return false;
	}
	std::string SQL = "SELECT * FROM registUsers WHERE id = %0 AND password = %1q";
	mysqlpp::StoreQueryResult result = m_MySQLModel.m_R(SQL,id, password);
	if (result.num_rows() > 0) {
		auto& row=result[0];
		if(strcmp(row["state"].c_str(),"online")==0){
			LOG_INFO << "User: " << id << " already online";
			return false;
		}
		LOG_INFO << "User logged in successfully: " << id;
		
		std::string updateSQL = "UPDATE registUsers SET state = 'online' WHERE id = %0";
		m_MySQLModel.m_CUD(updateSQL,id); // Update user state to online
		return true;
	}
	else {
		LOG_ERROR << "Failed to log in user: " << id;
		return false;
	}
	
}

bool Server::m_addFriend(int userid, int friendid)
{
	if (userid <= 0 || friendid <= 0 || userid == friendid) {
		return false; // Invalid user IDs or trying to add oneself as a friend
	}
	std::string SQL1 = "SELECT id FROM registUsers WHERE id = %0";
	mysqlpp::StoreQueryResult result = m_MySQLModel.m_R(SQL1,friendid);
	if(result.num_rows() == 0) {
		LOG_ERROR << "Failed to add friend: User " << friendid << " does not exist.";
		return false; // Friend does not exist
	}

	std::string SQL = "INSERT INTO friends VALUES (%0, %1)";
	if (m_MySQLModel.m_CUD(SQL, userid, friendid)) {
		LOG_INFO << "Friend added successfully: User " << userid << " added User " << friendid;
		return true;
	}
	else {
		LOG_ERROR << "Failed to add friend: User " << userid << " tried to add User " << friendid;
		return false;
	}
	return false;
}

bool Server::m_deleteFriend(int userid, int friendid)
{
	if (userid <= 0 || friendid <= 0 || userid == friendid) {
		return false; // Invalid user IDs or trying to delete oneself as a friend
	}
	std::string SQL = "DELETE FROM friends WHERE (userid = %0 AND friendid = %1);";
	if (m_MySQLModel.m_CUD(SQL,userid, friendid)) {
		LOG_INFO << "Friend deleted successfully: User " << userid << " deleted User " << friendid;
		return true;
	}
	else {
		LOG_ERROR << "Failed to delete friend: User " << userid << " tried to delete User " << friendid;
		return false;
	}
	return false;
}

std::vector<std::string> Server::m_getFriendList(int userid)
{
	std::string SQL = "SELECT friendid FROM friends WHERE userid = %0 UNION SELECT userid FROM friends WHERE friendid = %1;";
	mysqlpp::StoreQueryResult result = m_MySQLModel.m_R(SQL,userid,userid);
	if (result.num_rows() > 0) {
		std::vector<std::string> friendList;
		for (const auto& row : result) {
			
			friendList.push_back(row[0].c_str()); 
			
		}
		return friendList;
	}

		
    return std::vector<std::string>();
}


bool Server::m_createGroup(int userid, int groupid, std::string groupName, std::string groupDescribtion)
{
	std::string SQL="INSERT INTO `groups` VALUES (%0, %1q, %2q)";
	if (m_MySQLModel.m_CUD(SQL, groupid, groupName, groupDescribtion)) {
		LOG_INFO << "Group created successfully: Group ID " << groupid << ", Name: " << groupName;
		return m_joinGroup(userid, groupid, "creator"); // Automatically add the creator as an creator of the group
	}
	else {
		LOG_ERROR << "Failed to create group: Group ID " << groupid;
	}
	return false;
}

bool Server::m_PeerChat(int userid, int friendid, const std::string &message)
{
	std::string SQL = "SELECT id,name,state FROM registUsers WHERE id = %0;";
	mysqlpp::StoreQueryResult result = m_MySQLModel.m_R(SQL,friendid);
	if (result.num_rows() == 0) {
		LOG_ERROR << "Failed to send peer chat message: User " << friendid << " does not exist.";
		return false; // Friend does not exist
	}
	else
	{
		// Check if the friend is online
		std::string friendName = result[0]["name"].c_str();
		std::string friendState = result[0]["state"].c_str();
		if (friendState == "offline") {
			m_submitOfflineMessage(userid,friendid, message); // Submit offline message
			return true;
		}
		else
		{
			// Friend is online, send the message
			std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
			nlohmann::json response;
			response["MsgType"] = ENUM_MSGTYPE::PEER_CHAT_ACK;
			response["result"] = true;
			response["message"] = std::to_string(userid) + ": " + message; // Include the sender ID and message content
			if (m_userConnections.find(friendid) != m_userConnections.end())
			{
				m_userConnections[friendid]->send(response.dump()); // Send the message to the friend if they are at same server
			}
			else
			{
				m_RedisModel.m_publish(fmt::format("user_{}", friendid), response.dump()); // Publish the message to the Redis channel for the friend
			}

			return true;

		}

	}
	return false;
	
}

bool Server::m_groupChat(int userid, int groupid, const std::string &message)
{
	auto queryGroupExists = "SELECT id FROM `groups` WHERE id = %0";
	mysqlpp::StoreQueryResult groupExistsResult = m_MySQLModel.m_R(queryGroupExists, groupid);
	if (groupExistsResult.num_rows() == 0) {
		LOG_ERROR << "Failed to send group chat message: Group " << groupid << " does not exist.";
		return false; // Group does not exist
	}
	auto queryUserInGroup = "SELECT * FROM usersInGroup WHERE groupid = %0 AND userid = %1";
	mysqlpp::StoreQueryResult userInGroupResult = m_MySQLModel.m_R(queryUserInGroup, groupid, userid);
	if (userInGroupResult.num_rows() == 0) {
		LOG_ERROR << "Failed to send group chat message: User " << userid << " is not a member of Group " << groupid;
		return false; // User is not a member of the group
	}

	std::string SQL1="INSERT INTO groupMessage (groupId,senderId,message) VALUES (%0,%1,%2q)";
	int msgId=-1;
	if (m_MySQLModel.m_CUD(msgId,SQL1,groupid,userid,message)) {
		std::string SQL2="SELECT id,state FROM registUsers WHERE id IN (SELECT userid FROM usersInGroup WHERE groupid = %0)";
		mysqlpp::StoreQueryResult groupResult = m_MySQLModel.m_R(SQL2,groupid);

		nlohmann::json groupMessage;
		groupMessage["MsgType"] = ENUM_MSGTYPE::GROUP_CHAT_ACK;
		groupMessage["groupId"] = groupid;
		groupMessage["senderId"] = userid; // Set the sender ID
		groupMessage["message"] = message; // Set the message content

		for (const auto& row : groupResult) {
			int memberId = row["id"]; // Assuming the first column is the user ID
			std::string memberState = row["state"].c_str(); // Assuming the second column is the user state
			if(memberState == "online")
			{
				std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
				if(m_userConnections.find(memberId) != m_userConnections.end())
				{
					m_userConnections[memberId]->send(groupMessage.dump()); // Send the message to the group member if they are at same server

				}else{
					m_RedisModel.m_publish(fmt::format("user_{}", memberId), groupMessage.dump()); // Publish the message to the Redis channel for the group member
				}
			}
			else
			{
				std::string SQL3 = "INSERT INTO offlineGroupMessage (messageId, receiverId) VALUES (%0, %1)";
				if (!m_MySQLModel.m_CUD(SQL3, msgId, memberId)) {
					LOG_ERROR << "Failed to submit offline group message for User " << memberId;
				}
				else {
					LOG_INFO << "Offline group message submitted for User " << memberId;
				}
			}

		}
		return true; // Successfully sent group chat message
	}
	else {
		LOG_ERROR << "Failed to send group chat message: User " << userid << " tried to send message to Group " << groupid;
	}

    return false;
}

bool Server::m_submitOfflineMessage(int fromId,int toid, const std::string& message)
{
	std::string SQL = "INSERT INTO offlineMessage (senderId,receiverId, msg) VALUES (%0, %1, %2q)";
	if (m_MySQLModel.m_CUD(SQL,fromId, toid, message)) {
		LOG_INFO << "Offline message submitted. ";
		return true;
	}
	else 
	{
		LOG_ERROR << "Failed to submit offline message to User " << toid;
	}
	return false;
}

std::vector<std::string> Server::m_retrieveOfflineMessages(int userid)
{
	std::string SQL = "SELECT senderId,msg FROM offlineMessage WHERE receiverId = %0";
	mysqlpp::StoreQueryResult result = m_MySQLModel.m_R(SQL,userid);
	std::vector<std::string> messages;

	if (result.num_rows() > 0)
	{
		for (const auto &row : result)
		{
			messages.push_back(std::string(row[0].c_str()) + ":" + row[1].c_str()); // Assuming the message is in the first column
		}
		// Clear the offline messages after retrieval
		std::string deleteSQL = "DELETE FROM offlineMessage WHERE receiverId = %0";
		m_MySQLModel.m_CUD(deleteSQL,userid);
	}

	return messages;
}

std::vector<std::string> Server::m_retrieveOfflineGroupMessages(int userid)
{
	std::string SQL = "SELECT groupId,senderId,message FROM groupMessage WHERE msgId IN (SELECT messageId FROM offlineGroupMessage WHERE receiverId = %0)";
	mysqlpp::StoreQueryResult result = m_MySQLModel.m_R(SQL,userid);
	std::vector<std::string> messages;

	if (result.num_rows() > 0) {
		for (const auto& row : result) {
			messages.push_back(fmt::format("Group:{} Sender:{}:{}", row[0].c_str(), row[1].c_str(), row[2].c_str()));
		}
		// Clear the offline group messages after retrieval
		std::string deleteSQL = "DELETE FROM offlineGroupMessage WHERE receiverId = %0";
		m_MySQLModel.m_CUD(deleteSQL,userid);
		return messages;
	}
	
    return messages;
}

bool Server::m_joinGroup(int userid, int groupid, std::string userRole)
{
	if (userid <= 0 || groupid <= 0 || userRole.empty()) {
		return false; // Invalid parameters
	}

	std::string SQL1 = "SELECT id FROM `groups` WHERE id = %0";
	mysqlpp::StoreQueryResult result = m_MySQLModel.m_R(SQL1,groupid);
	if (result.num_rows() == 0) {
		LOG_ERROR << "Failed to join group: Group " << groupid << " does not exist.";
		return false; // Group does not exist
	}

	std::string SQL = "INSERT INTO usersInGroup VALUES (%0, %1, %2q)";
	if (m_MySQLModel.m_CUD(SQL, groupid, userid, userRole)) {
		LOG_INFO << "User " << userid << " joined group " << groupid << " as " << userRole;
		return true;
	}
	else {
		LOG_ERROR << "Failed to join group: User " << userid << " tried to join Group " << groupid;
		return false;
	}
	return false;
}

bool Server::m_leaveGroup(int userid, int groupid)
{
	if (userid <= 0 || groupid <= 0) {
		return false; // Invalid parameters
	}
	std::string SQL = "DELETE FROM usersInGroup WHERE userid = %0 AND groupid = %1";
	if (m_MySQLModel.m_CUD(SQL,userid, groupid)) {
		LOG_INFO << "User " << userid << " left group " << groupid;
		return true;
	}

    return false;
}

void Server::m_CallregistUser(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	
	nlohmann::json object=nlohmann::json::parse(buf);

	auto id = object["userId"].get<int>();
	auto name = object["username"].get<std::string>();
	auto password = object["password"].get<std::string>();
	std::string state = "offline";// Default state is offline

	if (m_registUser(id, name, password, state)) {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::REGISTOR_ACK;
		response["result"] = true;
		response["message"] = "Registration successful";
		conn->send(response.dump());

	}
	else {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::REGISTOR_ACK;
		response["result"] = false;
		response["message"] = "Registration failed";
		conn->send(response.dump());
	}
	
}

void Server::m_CallloginUser(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	auto id = object["userId"].get<int>();
	auto password = object["password"].get<std::string>();

	if (m_loginUser(id, password)) {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::LOGIN_ACK;
		response["result"] = true;
		response["message"] = "Login successful";
		conn->send(response.dump());
		{
			std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
			m_userConnections[id] = conn; // Store the connection for this user
		}

		m_RedisModel.m_subscribe(fmt::format("user_{}", id), 
		[this, conn](const std::string &channel, const std::string &message) {
			// Handle messages from Redis for this user
			conn->send(message);
		});
		m_RedisModel.m_start();

		// Retrieve and clear offline messages for the user
		std::vector<std::string> offlineMessages = m_retrieveOfflineMessages(id);
		response["message"]=offlineMessages;
		conn->send(response.dump()); // Send the offline messages to the user

		auto offlineGroupMessages = m_retrieveOfflineGroupMessages(id);
		response["message"]=offlineGroupMessages;
		conn->send(response.dump()); // Send the offline group messages to the user
	}
	else {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::LOGIN_ACK;
		response["result"] = false;
		response["message"] = "Login failed";
		conn->send(response.dump());
	}

}

void Server::m_CalladdFriend(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	auto friendid = object["friendId"].get<int>();

	int userid = -1; // Extract the user ID from the connection

	{
		std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
		for (auto &it : m_userConnections)
		{
			if (it.second == conn)
			{
				userid = it.first; // Get the user ID from the connection
				break;
			}
		}
	}

	//in case the table didn't allow inserting, we need to ensure that the smaller ID is always first
	int smallerId = std::min(userid, friendid);
	int largerId = std::max(userid, friendid);

	if (m_addFriend(smallerId, largerId)){
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::ADDFRIEND_ACK;
		response["result"] = true;
		response["message"] = "Friend added successfully";
		conn->send(response.dump());
	}
	else {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::ADDFRIEND_ACK;
		response["result"] = false;
		response["message"] = "Failed to add friend";
		conn->send(response.dump());
	}
}

void Server::m_CalldeleteFriend(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	auto friendid = object["friendId"].get<int>();

	int userid = -1; // Extract the user ID from the connection
	// Find the user ID associated with the connection
	{
		std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
		for(auto& it: m_userConnections) {
			if (it.second == conn) {
				userid = it.first; // Get the user ID from the connection
				break;
			}
		}
	}
	
	//in case the table didn't allow inserting, we need to ensure that the smaller ID is always first
	int smallerId = std::min(userid, friendid);
	int largerId = std::max(userid, friendid);

	if (m_deleteFriend(smallerId, largerId)) {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::DELFRIEND_ACK;
		response["result"] = true;
		response["message"] = "Friend deleted successfully";
		conn->send(response.dump());
	}
	else {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::DELFRIEND_ACK;
		response["result"] = false;
		response["message"] = "Failed to delete friend";
		conn->send(response.dump());
	}
}

void Server::m_CallgetFriendList(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	int userid = -1; // Extract the user ID from the connection
	// Find the user ID associated with the connection
	{
		std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
		for (auto &it : m_userConnections)
		{
			if (it.second == conn)
			{
				userid = it.first; // Get the user ID from the connection
				break;
			}
		}
	}

	std::vector<std::string> friendList = m_getFriendList(userid);
	nlohmann::json response;
	response["MsgType"] = ENUM_MSGTYPE::GETFRIENDLIST_ACK;
	response["result"] = true;
	response["friends"] = friendList;
	conn->send(response.dump());
}

void Server::m_CalljoinGroup(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	auto groupId = object["groupId"].get<int>();
	auto userRole = "normal"; // Default user role is normal, can be changed based on requirements

	int userid = -1; // Extract the user ID from the connection
	// Find the user ID associated with the connection
	{
		std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
		for (auto &it : m_userConnections)
		{
			if (it.second == conn)
			{
				userid = it.first; // Get the user ID from the connection
				break;
			}
		}
	}
	
	if (m_joinGroup(userid, groupId, userRole)) {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::JOIN_GROUP_ACK;
		response["result"] = true;
		response["message"] = "Joined group successfully";
		conn->send(response.dump());
	}
	else {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::JOIN_GROUP_ACK;
		response["result"] = false;
		response["message"] = "Failed to join group";
		conn->send(response.dump());
	}
}

void Server::m_CallleaveGroup(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);
	auto groupId = object["groupId"].get<int>();
	int userid = -1; // Extract the user ID from the connection
	// Find the user ID associated with the connection
	{
		std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
		for (auto &it : m_userConnections)
		{
			if (it.second == conn)
			{
				userid = it.first; // Get the user ID from the connection
				break;
			}
		}
	}

	if (m_leaveGroup(userid, groupId)) {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::LEAVE_GROUP_ACK;
		response["result"] = true;
		response["message"] = "Left group successfully";
		conn->send(response.dump());
	}
	else {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::LEAVE_GROUP_ACK;
		response["result"] = false;
		response["message"] = "Failed to leave group";
		conn->send(response.dump());
	}
}

void Server::m_CallcreateGroup(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	auto groupId = object["groupId"].get<int>();
	auto groupName = object["groupName"].get<std::string>();
	auto groupDescribe = object["groupDescribe"].get<std::string>();

	int userid = -1; // Extract the user ID from the connection
	// Find the user ID associated with the connection
	{
		std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
		for (auto &it : m_userConnections)
		{
			if (it.second == conn)
			{
				userid = it.first; // Get the user ID from the connection
				break;
			}
		}
	}


	if (m_createGroup(userid, groupId, groupName, groupDescribe)) {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::CREATE_GROUP_ACK;
		response["result"] = true;
		response["message"] = "Group created successfully";
		conn->send(response.dump());
	}
	else {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::CREATE_GROUP_ACK;
		response["result"] = false;
		response["message"] = "Failed to create group";
		conn->send(response.dump());
	}
}

void Server::m_CallPeerChat(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);

	auto friendId = object["friendId"].get<int>();
	auto message = object["message"].get<std::string>();

	int userid = -1; // Extract the user ID from the connection
	// Find the user ID associated with the connection
	{
		std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
		for (auto &it : m_userConnections)
		{
			if (it.second == conn)
			{
				userid = it.first; // Get the user ID from the connection
				break;
			}
		}
	}
	
	if (m_PeerChat(userid, friendId, message)) {
	}
	else {
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::PEER_CHAT_ACK;
		response["result"] = false;
		response["message"] = "friend does not exist";
		conn->send(response.dump());
	}
}

void Server::m_CallgroupChat(const MuduoTcpConnectionPtr& conn, std::string buf, muduo::Timestamp time)
{
	nlohmann::json object = nlohmann::json::parse(buf);
	auto groupId = object["groupId"].get<int>();
	auto message = object["message"].get<std::string>();

	int userid = -1; // Extract the user ID from the connection

	{
		std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety when accessing m_userConnections
		for (auto &it : m_userConnections)
		{
			if (it.second == conn)
			{
				userid = it.first; // Get the user ID from the connection
				break;
			}
		}
	}

	if(!m_groupChat(userid, groupId, message)){
		nlohmann::json response;
		response["MsgType"] = ENUM_MSGTYPE::GROUP_CHAT_ACK;
		response["result"] = false;
		response["message"] = "message failed to send";
		conn->send(response.dump());
	}
	

}
