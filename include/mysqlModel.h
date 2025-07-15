#pragma once
#include "mysql++.h"

class MySQLModel
{
public:
	MySQLModel();
	~MySQLModel();

	enum class ENUM_SQL_OPERATION
	{
		INSERT, UPDATE, DELETE
	};

	template<typename... T>
	bool m_CUD(int &insertedId, std::string SQL, T&&... args)
	{
		try
		{
			mysqlpp::Connection con(m_dbName.c_str(), m_hostaddress.c_str(), m_username.c_str(), m_password.c_str());
			mysqlpp::Query query = con.query(SQL);
			query.parse();
			query.execute(std::forward<T>(args)...);
			insertedId = query.insert_id(); // Get the last inserted ID
			std::cout << "Query executed successfully: " << query.info() << '\n';
			return true;
		}
		catch (const mysqlpp::Exception& e)
		{
			std::cerr << e.what() << '\n';
			return false;
		}
	}

	template<typename... T>
	bool m_CUD(std::string SQL, T&&... args)
	{
		try
		{
			mysqlpp::Connection con(m_dbName.c_str(), m_hostaddress.c_str(), m_username.c_str(), m_password.c_str());
			mysqlpp::Query query = con.query(SQL);
			query.parse();
			query.execute(std::forward<T>(args)...);
			std::cout << "Query executed successfully: " << query.info() << '\n';
			return true;
		}
		catch (const mysqlpp::Exception& e)
		{
			std::cerr << e.what() << '\n';
			return false;
		}
	}
	

	/// <summary>
	/// select operation on database
	/// </summary>
	/// <param name="SQL"></param>
	/// <returns></returns>
	template<typename... T>
	mysqlpp::StoreQueryResult m_R(std::string SQL, T&&... args)
	{
		try
		{
			mysqlpp::Connection con(m_dbName.c_str(), m_hostaddress.c_str(), m_username.c_str(), m_password.c_str());
			mysqlpp::Query query = con.query(SQL);
			query.parse();
			return query.store(std::forward<T>(args)...);
		}
		catch (const mysqlpp::Exception& e)
		{
			std::cerr << e.what() << '\n';
			return mysqlpp::StoreQueryResult();
		}
	}

private:
	std::string m_dbName;
	std::string m_hostaddress;
	std::string m_username;
	std::string m_password;
};

