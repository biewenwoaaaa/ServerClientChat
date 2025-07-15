#pragma once
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Timestamp.h"
#include "nlohmann/json.hpp"
#include "magic_enum.hpp"

using MuduoTcpConnectionPtr = muduo::net::TcpConnectionPtr;
using MuduoEventLoop        = muduo::net::EventLoop;
using MuduoInetAddress      = muduo::net::InetAddress;
using MuduoNetBuffer        = muduo::net::Buffer;
using MuduoTimeStamp        = muduo::Timestamp;

enum class ENUM_MSGTYPE
{
	REGISTOR,
	REGISTOR_ACK,

	LOGIN,
	LOGIN_ACK,

	ADDFRIEND,
	ADDFRIEND_ACK,

	DELFRIEND,
	DELFRIEND_ACK,

	GETFRIENDLIST,
	GETFRIENDLIST_ACK,

	JOIN_GROUP,
	JOIN_GROUP_ACK,

	LEAVE_GROUP,
	LEAVE_GROUP_ACK,

	CREATE_GROUP,
	CREATE_GROUP_ACK,

	PEER_CHAT,
	PEER_CHAT_ACK,

	GROUP_CHAT,
	GROUP_CHAT_ACK
};



class iCallBackFunc {
public:
	virtual void m_CallregistUser   (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;
	virtual void m_CallloginUser    (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;
	virtual void m_CalladdFriend    (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;
	virtual void m_CalldeleteFriend (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;
	virtual void m_CallgetFriendList(const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;
	virtual void m_CalljoinGroup    (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;
	virtual void m_CallleaveGroup   (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;
	virtual void m_CallcreateGroup  (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;
	virtual void m_CallPeerChat     (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;
	virtual void m_CallgroupChat    (const MuduoTcpConnectionPtr& conn, std::string buf, MuduoTimeStamp time) = 0;

};

void from_json(const nlohmann::json& j, ENUM_MSGTYPE& e);

void to_json(nlohmann::json& j, const ENUM_MSGTYPE& e);

