#include "clientStateTranslation.h"
#include <iostream>
#include "sayHi.h"

Context::Context( std::function<void(const std::string& message)>&& write,std::atomic_bool& loginSuccess, int& pipefd) :
	m_write(std::move(write)), 
	m_loginSuccess(loginSuccess),
	m_pipefd(pipefd)
{
	// FD_ZERO(&m_inputSet);
	// FD_SET(STDIN_FILENO, &m_inputSet); // Initialize the input set to listen for standard input
	// FD_SET(pipefd, &m_inputSet); // Add the pipe file descriptor to the input set


}	

Context::~Context()
{
}

void Context::m_setCurrentState(std::unique_ptr<iState> state)
{
	if (dynamic_cast<MainMenuState *>(state.get()) ||
		dynamic_cast<RegisterState *>(state.get()) ||
		dynamic_cast<LoginState *>(state.get()))
	{
	}
	else if (!m_loginSuccess.load())
	{
		std::cout << "You are offline, please login again." << std::endl;

		state = std::make_unique<MainMenuState>(); // force to rotate to MainMenuState
	}
	
	if (m_currentState)
	{
		m_currentState->m_onExit(this);
	}
	
	m_currentState = std::move(state);
	
	if (m_currentState)
	{
		m_currentState->m_onEnter(this);
	}
}

void MainMenuState::m_onEnter(Context* context)
{
	std::cout << "Hello! welcome to the main menu!" << std::endl;
	std::cout << "---------------------------------" << std::endl;
	std::cout << "-1. Exit program" << std::endl;
	std::cout << " 0. Registor" << std::endl;
	std::cout << " 1. Login" << std::endl;
	
	std::cout << "Please select an option: "<<std::flush;

	int choice;
	if(!context->m_inputToVar(choice)){
		return;
	}
	switch (choice) {
	case -1:
		break;
	case 0:
		context->m_setCurrentState(std::make_unique<RegisterState>());
		break;
	case 1:	
		context->m_setCurrentState(std::make_unique<LoginState>());
		break;
	default:
		std::cout << "Invalid choice, please try again." << std::endl;
		m_onEnter(context); // Re-display the menu
		break;
	}
	
		 
}

void MainMenuState::m_onExit(Context* context)
{
}

void RegisterState::m_onEnter(Context* context)
{
	std::cout << "Registering a new user..." << std::endl;
	std::cout << "Please enter your ID:"<<std::flush;
	int friendId;
	if (!context->m_inputToVar(friendId))
	{
		return;
	}

	std::cout << std::endl << "Please enter your username: "<<std::flush;
	std::string username;
	if (!context->m_inputToVar(username))
	{
		return;
	}

	std::cout << std::endl << "Please enter your password: "<<std::flush;
	std::string password;
	if (!context->m_inputToVar(password))
	{
		return;
	}

	nlohmann::json requestJson = {
		{"MsgType", ENUM_MSGTYPE::REGISTOR},
		{"userId", friendId},
		{"username", username},
		{"password", password}
	};
	
	context->m_write(requestJson.dump());
	
	// After registration, return to the main menu
	context->m_setCurrentState(std::make_unique<MainMenuState>());
}

void RegisterState::m_onExit(Context* context)
{
}

void LoginState::m_onEnter(Context* context)
{
	std::cout << "Logging in..." << std::endl;
	std::cout << "Please enter your ID:"<<std::flush;
	int friendId;
	if (!context->m_inputToVar(friendId))
	{
		return;
	}
	std::cout << std::endl << "Please enter your password: "<<std::flush;
	std::string password;
	if (!context->m_inputToVar(password))
	{
		return;
	}

	nlohmann::json requestJson = {
		{"MsgType", ENUM_MSGTYPE::LOGIN},
		{"userId", friendId},
		{"password", password}
	};
	context->m_write(requestJson.dump());

	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_conditionVariable.wait_for(lock, std::chrono::seconds(1), [&context]() {
		return context->m_loginSuccess.load();
		})
		) 
	{//login success
		std::cout << "Login successful!" << std::endl;
		context->m_setCurrentState(std::make_unique<SecondMenuState>());
	}
	else //login failed
	{
		std::cout << "Login failed, please try again." << std::endl;
		context->m_setCurrentState(std::make_unique<MainMenuState>());
		
	}

}

void LoginState::m_onExit(Context* context)
{
}

void SecondMenuState::m_onEnter(Context* context)
{
	std::cout << "Hello! welcome to the Second menu!" << std::endl;
	std::cout << "---------------------------------" << std::endl;
	std::cout << "-1. Exit program" << std::endl;
	std::cout << " 2. Add Friend" << std::endl;
	std::cout << " 3. Delete Friend" << std::endl;
	std::cout << " 4. Get Friend List" << std::endl;
	std::cout << " 5. Join Group" << std::endl;
	std::cout << " 6. Leave Group" << std::endl;
	std::cout << " 7. Create Group" << std::endl;
	std::cout << " 8. Peer Chat" << std::endl;
	std::cout << " 9. Group Chat" << std::endl;
	std::cout << "---------------------------------" << std::endl;
	std::cout << "Please select an option: "<<std::flush;

	int choice;
	if (!context->m_inputToVar(choice))
	{
		return;
	}
	switch (choice) {
	case -1:
		break;
	case 2:
		context->m_setCurrentState(std::make_unique<AddFriendState>());
		break;
	case 3:
		context->m_setCurrentState(std::make_unique<DeleteFriendState>());
		break;
	case 4:
		context->m_setCurrentState(std::make_unique<GetFriendListState>());
		break;
	case 5:
		context->m_setCurrentState(std::make_unique<JoinGroupState>());
		break;
	case 6:
		context->m_setCurrentState(std::make_unique<LeaveGroupState>());
		break;
	case 7:
		context->m_setCurrentState(std::make_unique<CreateGroupState>());
		break;
	case 8:
		context->m_setCurrentState(std::make_unique<PeerChatState>());
		break;
	case 9:
		context->m_setCurrentState(std::make_unique<GroupChatState>());
		break;
	default:
		std::cout << "Invalid choice, please try again." << std::endl;
		m_onEnter(context); // Re-display the menu
		break;
	}
}

void SecondMenuState::m_onExit(Context* context)
{
}

void AddFriendState::m_onEnter(Context* context)
{
	std::cout << "Add friend ..." << std::endl;
	std::cout << "Please enter your friend ID:"<<std::flush;
	int friendId;
	if (!context->m_inputToVar(friendId))
	{
		return;
	}

	nlohmann::json requestJson = {
		{"MsgType", ENUM_MSGTYPE::ADDFRIEND},
		{"friendId", friendId}
	};
	context->m_write(requestJson.dump());


	//back to secondMeneState
	context->m_setCurrentState(std::make_unique<SecondMenuState>());
}

void AddFriendState::m_onExit(Context* context)
{
}

void DeleteFriendState::m_onEnter(Context* context)
{
	std::cout << "Delete friend ..." << std::endl;
	std::cout << "Please enter your friend ID:"<<std::flush;
	int friendId;
	if (!context->m_inputToVar(friendId))
	{
		return;
	}

	nlohmann::json requestJson = {
		{"MsgType", ENUM_MSGTYPE::DELFRIEND},
		{"friendId", friendId}
	};
	context->m_write(requestJson.dump());

	//back to secondMeneState
	context->m_setCurrentState(std::make_unique<SecondMenuState>());
}

void DeleteFriendState::m_onExit(Context* context)
{
}

void GetFriendListState::m_onEnter(Context* context)
{
	nlohmann::json requestJson = {
		{"MsgType", ENUM_MSGTYPE::GETFRIENDLIST}
		
	};
	context->m_write(requestJson.dump());

	//back to secondMeneState
	context->m_setCurrentState(std::make_unique<SecondMenuState>());
}

void GetFriendListState::m_onExit(Context* context)
{
}

void JoinGroupState::m_onEnter(Context* context)
{
	std::cout << "Join Croup ..." << std::endl;
	std::cout << "Please enter group ID:"<<std::flush;
	int groupId;
	if (!context->m_inputToVar(groupId))
	{
		return;
	}

	nlohmann::json requestJson = {
		{"MsgType", ENUM_MSGTYPE::JOIN_GROUP},
		{"groupId", groupId}
	};
	context->m_write(requestJson.dump());

	//back to secondMeneState
	context->m_setCurrentState(std::make_unique<SecondMenuState>());
}

void JoinGroupState::m_onExit(Context* context)
{
}

void LeaveGroupState::m_onEnter(Context* context)
{
	std::cout << "Leave Croup ..." << std::endl;
	std::cout << "Please enter group ID:"<<std::flush;
	int groupId;
	if (!context->m_inputToVar(groupId))
	{
		return;
	}

	nlohmann::json requestJson = {
		{"MsgType", ENUM_MSGTYPE::LEAVE_GROUP},
		{"groupId", groupId}
	};
	context->m_write(requestJson.dump());

	//back to secondMeneState
	context->m_setCurrentState(std::make_unique<SecondMenuState>());
}

void LeaveGroupState::m_onExit(Context* context)
{
}

void CreateGroupState::m_onEnter(Context* context)
{
	std::cout << "Create Croup ..." << std::endl;
	std::cout << "Please enter group ID:"<<std::flush;
	int groupId;
	if (!context->m_inputToVar(groupId))
	{
		return;
	}
	std::cout << "Please enter group name: "<<std::flush;
	std::string groupName;
	if (!context->m_inputToVar(groupName))
	{
		return;
	}
	std::cout << "Please enter group describe:";
	std::string groupDescribe;
	if (!context->m_inputToVar(groupDescribe))
	{
		return;
	}

	nlohmann::json requestJson = {
		{"MsgType", ENUM_MSGTYPE::CREATE_GROUP},
		{"groupId", groupId},
		{"groupName", groupName},
		{"groupDescribe",groupDescribe}
	};
	context->m_write(requestJson.dump());

	//back to secondMeneState
	context->m_setCurrentState(std::make_unique<SecondMenuState>());

	
}

void CreateGroupState::m_onExit(Context* context)
{
}

void PeerChatState::m_onEnter(Context* context)
{
	std::cout << "Peer Chat ..." << std::endl;
	std::cout << "Please enter your friend ID:"<<std::flush;
	int friendId;
	if (!context->m_inputToVar(friendId))
	{
		return;
	}

	while (true)
	{
		std::cout << "Please enter your message (!!quit to return secondMenu): "<<std::flush;
		std::string message;
		if (!context->m_inputToVar(message))
		{
			return;
		}

		if(message == "!!quit")
		{
			break; // Exit the loop if user types !!quit
		}
		

		nlohmann::json requestJson = {
			{"MsgType", ENUM_MSGTYPE::PEER_CHAT},
			{"friendId", friendId},
			{"message", message}};
		context->m_write(requestJson.dump());
	}
	//back to secondMeneState
	context->m_setCurrentState(std::make_unique<SecondMenuState>());
}

void PeerChatState::m_onExit(Context* context)
{
}

void GroupChatState::m_onEnter(Context* context)
{
	std::cout << "Group Chat ..." << std::endl;
	std::cout << "Please enter group ID:"<<std::flush;
	int groupId;
	if (!context->m_inputToVar(groupId))
	{
		return;
	}

	while (true)
	{
		std::cout << "Please enter your message (!!quit to return secondMenu): "<<std::flush;
		std::string message;
		if (!context->m_inputToVar(message))
		{
			return;
		}

		if(message == "!!quit")
		{
			break; // Exit the loop if user types !!quit
		}

		nlohmann::json requestJson = {
			{"MsgType", ENUM_MSGTYPE::GROUP_CHAT},
			{"groupId", groupId},
			{"message", message}
		};
		context->m_write(requestJson.dump());
	}
	//back to secondMeneState
	context->m_setCurrentState(std::make_unique<SecondMenuState>());
}

void GroupChatState::m_onExit(Context* context)
{
}
