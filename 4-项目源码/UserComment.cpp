#include <eosio/eosio.hpp>

using namespace eosio;

class [[eosio::contract("UserComment")]] UserComment : public eosio::contract
{

public:
    UserComment(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds) {}

    [[eosio::action]] void login(name user)
    {
        require_auth(user);
    }

    [[eosio::action]] void insert(name user, std::string content, uint64_t replyid, uint64_t shopid)
    {
        require_auth(user);
        comment_index cmi(get_self(), shopid);
        if (replyid != 0) {
            auto& iterator = cmi.get(replyid, "You can't reply to NULL!");
            check(iterator.visible, "You can't reply to NULL!");
        }
        cmi.emplace(user, [&](auto &row) {
            row.cid = cmi.available_primary_key() ? cmi.available_primary_key() : 1;
            row.user = user;
            row.content = content;
            row.visible = true;
            row.replyid = replyid;
            row.shopid = shopid;
        });
    }

    [[eosio::action]] void update(uint64_t cid, name user, std::string content, uint64_t shopid)
    {
        require_auth(user);
        comment_index cmi(get_self(), shopid);
        auto& iterator = cmi.get(cid, "data not found");
        check(iterator.visible, "data not found");
        check(user == iterator.user, "You can't modify other user's data!");
        cmi.emplace(user, [&](auto &row) {
            row.cid = cmi.available_primary_key() ? cmi.available_primary_key() : 1;
            row.user = user;
            row.content = content;
            row.visible = true;
            row.replyid = iterator.replyid;
            row.shopid = iterator.shopid;
        });
        cmi.modify(iterator, user, [&](auto &row) {
            row.visible = false;
        });
    }

    [[eosio::action]] void erase(uint64_t cid, name user, uint64_t shopid)
    {
        check(has_auth(user) || has_auth("admin"_n), "Missing requried authority");
        comment_index cmi(get_self(), shopid);
        auto& iterator = cmi.get(cid, "data not found");
        check(iterator.visible, "data not found");
        check(user == iterator.user, "You can't delete other user's data!");
        cmi.modify(iterator, user, [&](auto &row) {
            row.visible = false;
        });
    }

private:
    struct [[eosio::table]] comment
    {
        uint64_t cid;
        name user;
        bool visible;
        std::string content;
        uint64_t replyid;
        uint64_t shopid;
        uint64_t primary_key() const { return cid; }
    };
    typedef eosio::multi_index<"comments"_n, comment> comment_index;
};
