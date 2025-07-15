#include "mysqlModel.h"

MySQLModel::MySQLModel()
{
	m_dbName = "DB_Chat";
	m_hostaddress="localhost";
	m_username="root";
	m_password="1234";
}

MySQLModel::~MySQLModel()
{
}

// bool MySQLModel::m_CUD(std::string SQL)
// {
// 	try
// 	{
// 		mysqlpp::Connection con(m_dbName.c_str(), m_hostaddress.c_str(), m_username.c_str(), m_password.c_str());
// 		auto res = con.query(SQL).execute();
// 		if (res)
// 		{
// 			std::cout << "Query executed successfully: " << res.info() << '\n';
// 			return true;
// 		}
// 		else
// 		{
// 			std::cerr << "Query execution failed: " << con.error() << '\n';
// 			return false;
// 		}

// 	}
// 	catch(const mysqlpp::Exception& e)
// 	{
// 		std::cerr << e.what() << '\n';
// 		return false;
// 	}
	
	

// }

// bool MySQLModel::m_CUD(std::string SQL, int &insertedId)
// {
// 	try
// 	{
// 		mysqlpp::Connection con(m_dbName.c_str(), m_hostaddress.c_str(), m_username.c_str(), m_password.c_str());
// 		auto res = con.query(SQL).execute();
// 		if (res)
// 		{
// 			std::cout << "Query executed successfully: " << res.info() << '\n';
// 			insertedId = res.insert_id(); // Get the last inserted ID
// 			return true;
// 		}
// 		else
// 		{
// 			std::cerr << "Query execution failed: " << con.error() << '\n';
// 			return false;
// 		}

// 	}
// 	catch(const mysqlpp::Exception& e)
// 	{
// 		std::cerr << e.what() << '\n';
// 		return false;
// 	}
//     return false;
// }


// mysqlpp::StoreQueryResult MySQLModel::m_R(std::string SQL)
// {
	
	
// }
