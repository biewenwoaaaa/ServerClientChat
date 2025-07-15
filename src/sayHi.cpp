#include "../include/sayHi.h"


/// <summary>
/// define json serialization and deserialization for ENUM_MSGTYPE
/// </summary>
void from_json(const nlohmann::json& j, ENUM_MSGTYPE& e)
{
	std::string str = j.get<std::string>();
	e = magic_enum::enum_cast<ENUM_MSGTYPE>(str).value_or(ENUM_MSGTYPE::REGISTOR);
}
void to_json(nlohmann::json& j, const ENUM_MSGTYPE& e)
{
	j = magic_enum::enum_name(e);
}