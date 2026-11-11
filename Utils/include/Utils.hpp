#include "nlohmann/json.hpp"
#include "Command.hpp"
class Util
{
public:
    static Command deserialize(nlohmann::json &jsonCommand);
    static nlohmann::json serialize(Command command);
};