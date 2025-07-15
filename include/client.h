#pragma once
#include "sayHi.h"
#include "muduo/net/TcpClient.h"
#include <functional>
#include <map>
#include <thread>
#include <memory>

using MuduoTcpClient = muduo::net::TcpClient;

class Client :public iCallBackFunc {
public:
	Client(MuduoEventLoop* loop, const MuduoInetAddress& serverAddr,int& pipefd);
	~Client();

	void m_connect();
	void m_disconnect();
	void m_write(const std::string& message);

	/// <summary>
	/// Handles a response received from TcpServer.
	/// </summary>
	/// <param name="conn">A shared pointer to the TCP connection from which the request was received.</param>
	/// <param name="buf">the incoming data.</param>
	/// <param name="time">The timestamp indicating when the data was received.</param>
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

private:
	void m_onConnection(const MuduoTcpConnectionPtr& conn);
	void m_onMessage(const MuduoTcpConnectionPtr& conn,
		MuduoNetBuffer* buf,
		MuduoTimeStamp receiveTime);
	
	void m_mainMenu();
private:
	MuduoEventLoop* m_loop;
	MuduoTcpClient m_client;
	MuduoTcpConnectionPtr m_connection;
	std::map<ENUM_MSGTYPE, std::function<void(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time)>> m_mapFunction;
	

	/// <summary>
	///Thread for handling the main menu interaction
	/// </summary>
	std::unique_ptr<std::thread> m_mainMenuThread;
	std::atomic_bool m_loginSuccess;

	int m_readPipeFd; // File descriptor for reading messages from the pipe
};