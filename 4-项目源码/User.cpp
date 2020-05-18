#include <eosio/eosio.hpp>

using namespace eosio;

class [[eosio::contract("User")]] User : public eosio::contract
{

public:
    User(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds) {}

    [[eosio::action]] void userinsert(name user, std::string user_name, bool is_merchant, std::string avatar_url, std::string phone_number,
                                    std::string address, std::string utils)
    {
        require_auth(user);
        user_index ui(get_self(), get_first_receiver().value);
        ui.emplace(user, [&](auto &row) {
            row.user = user;
            row.user_name = user_name;
            row.is_merchant = is_merchant;
            row.avatar_url = avatar_url;
            row.phone_number = phone_number;
            row.address = address;
            row.utils = utils;
        });
    }
    [[eosio::action]] void userupdate(name user, std::string user_name, bool is_merchant, std::string avatar_url, std::string phone_number,
                                    std::string address, std::string utils)
    {
        require_auth(user);
        user_index ui(get_self(), get_first_receiver().value);
        auto &iterator = ui.get(user.value, "user not found, please regist first!");
        ui.modify(iterator, user, [&](auto &row) {
            row.user_name = user_name;
            row.is_merchant = is_merchant;
            row.avatar_url = avatar_url;
            row.phone_number = phone_number;
            row.address = address;
            row.utils = utils;
        });
    }

    [[eosio::action]] void usererase(name user)
    {
         require_auth(user);
        user_index ui(get_self(), get_first_receiver().value);
        auto &iterator = ui.get(user.value, "user doesn't exist!");
        ui.erase(iterator);
    }

private:
    struct [[eosio::table]] user
    {
        name user;
        std::string user_name;
        bool is_merchant;
        std::string avatar_url;
        std::string phone_number;
        std::string address;
        std::string utils;
        uint64_t primary_key() const { return user.value; }
    };
    typedef eosio::multi_index<"user.info"_n, user> user_index;
};
