#pragma once
#include <memory>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>

class Context;
class iState 
{
public:
	virtual ~iState() = default;
	//virtual void m_handle(Context* context) = 0;
	virtual void m_onEnter(Context* context) = 0;
	virtual void m_onExit(Context* context) = 0;
};

class Context
{
public:
	Context(std::function<void(const std::string& message)>&& write,
		std::atomic_bool& loginSuccess,int& pipefd);
	~Context();

	/// <summary>
	/// Sets the current state to the provided state object.
	/// </summary>
	/// <param name="state">A unique pointer to the new state object to set as the current state.</param>
	void m_setCurrentState(std::unique_ptr<iState> state);
	
	/// <summary>
	/// send a message to the server
	/// </summary>
	std::function<void(const std::string)> m_write;

	//synchronize with the main thread
	std::atomic_bool& m_loginSuccess;

	///return true: normal input, return false: abnormal input
	template<typename T>
	bool m_inputToVar(T& value){
		fd_set inputSet;
		FD_ZERO(&inputSet);
		FD_SET(STDIN_FILENO, &inputSet);
		FD_SET(m_pipefd, &inputSet);

		int maxFd = std::max(STDIN_FILENO, m_pipefd);
		int result = select(maxFd + 1, &inputSet, nullptr, nullptr, nullptr);
		if (result < 0) {
			std::cout<<"select error!"<<std::endl;
			return false;
		}
		if(FD_ISSET(STDIN_FILENO, &inputSet)) {
			std::string inputString;  //std::getline only accept std::string ,then I will transform 
			if(!std::getline(std::cin, inputString)) {
				std::cerr << "Failed to read input." << std::endl;
				return false;
			}

			if constexpr (std::is_same_v<T, std::string>)
			{
				value = inputString;
			}
			else
			{
				std::istringstream iss(inputString);
				iss >> value; // when it is int,float ....basic type, ok!
			}

			return true;
		} else if (FD_ISSET(m_pipefd, &inputSet)) {
			char buffer;
			read(m_pipefd, &buffer, 1);
			std::cerr<<"shut down!"<<std::endl;
			return false;
		} else{
			return false;
		}

		
	}
private:
	
	std::unique_ptr<iState> m_currentState;

	//fd_set m_inputSet;//listen to the input from the user and pipe interrupt
	int m_pipefd; // File descriptor for the pipe to read messages from
};

class MainMenuState : public iState
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
	//void m_handle(Context* context) override;
};
class SecondMenuState :public iState
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
	//void m_handle(Context* context) override;
};

class RegisterState:public iState
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;

};

class LoginState:public iState
{	
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
	//void m_handle(Context* context) override;
private:
	std::mutex m_mutex; // Mutex for synchronizing access to m_loginSuccess
	std::condition_variable m_conditionVariable; // Condition variable for waiting on login success
};

class AddFriendState :public iState
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
};

class DeleteFriendState :public iState 
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
};

class GetFriendListState : public iState
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
};

class JoinGroupState :public iState 
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;

};

class LeaveGroupState :public iState
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
};

class CreateGroupState : public iState 
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
};

class PeerChatState:public iState
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
};

class GroupChatState :public iState
{
public:
	void m_onEnter(Context* context) override;
	void m_onExit(Context* context) override;
};