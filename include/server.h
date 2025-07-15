#pragma once
#include "mysqlModel.h"
#include "muduo/net/TcpServer.h"
#include "sayHi.h"
#include <functional>
#include <mutex>
#include "redisModel.h"

using MuduoTcpServer = muduo::net::TcpServer;

class Server:public iCallBackFunc
{
public:
	Server(MuduoEventLoop* loop, const MuduoInetAddress& listenAddr);
	~Server();

	void m_startServer();
	

private:

	/// <summary>
	/// transport connection established callback
	/// </summary>
	/// <param name="conn"></param>
	/// <param name="buf"></param>
	/// <param name="time"></param>
	void m_onMessage(const MuduoTcpConnectionPtr& conn, MuduoNetBuffer* buf, MuduoTimeStamp time);
	
	/// <summary>
	/// response to client message
	/// </summary>
	/// <param name="conn"></param>
	/// <param name="buf"></param>
	/// <param name="time"></param>
	virtual void m_CallregistUser   (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;
	virtual void m_CallloginUser    (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;
	virtual void m_CalladdFriend    (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;
	virtual void m_CalldeleteFriend (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;
	virtual void m_CallgetFriendList(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;
	virtual void m_CalljoinGroup    (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;
	virtual void m_CallleaveGroup   (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;
	virtual void m_CallcreateGroup  (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;
	virtual void m_CallPeerChat     (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;
	virtual void m_CallgroupChat    (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) override;

	bool m_registUser(int id, std::string name, std::string password, std::string state);
	bool m_loginUser(int id, std::string password);
	bool m_addFriend(int userid, int friendid);
	bool m_deleteFriend(int userid, int friendid);
	std::vector<std::string>	m_getFriendList(int userid);
	bool m_joinGroup(int userid, int groupid, std::string userRole);
	bool m_leaveGroup(int userid, int groupid);
	bool m_createGroup(int userid, int groupid, std::string groupName, std::string groupDescribtion);
	bool m_PeerChat(int userid, int friendid, const std::string& message);
	bool m_groupChat(int userid, int groupid, const std::string& message);

private:
	bool m_submitOfflineMessage(int fromId,int toid, const std::string& message);
	std::vector<std::string> m_retrieveOfflineMessages(int userid);
	std::vector<std::string> m_retrieveOfflineGroupMessages(int userid);

	MySQLModel m_MySQLModel;
	MuduoTcpServer m_server;
	std::map<ENUM_MSGTYPE, std::function<void(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)>> m_mapFunction; // strategy pattern for message handling

	
	std::mutex m_mutex; // Mutex for thread safety
	std::unordered_map<int, MuduoTcpConnectionPtr> m_userConnections; // Store user connections by user ID

	RedisModel m_RedisModel; // Redis model for caching and message storage
};

