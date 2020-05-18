#include <eosio/eosio.hpp>

using namespace eosio;

class [[eosio::contract("Shop")]] Shop : public eosio::contract
{

public:
    Shop(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds) {}

    //sid、user、description、shop_name、tag、address、goodsid、visible
    [[eosio::action]] void shopinsert(name user, std::string shop_name, std::string description,
                                      std::string picture_url, std::string tag, std::string address)
    {
        require_auth(user);
        shop_index getid(get_self(), get_first_receiver().value);
        shop_index si(get_self(), user.value);
        si.emplace(user, [&](auto &row) {
            row.sid = getid.available_primary_key() ? getid.available_primary_key() : 1;
            row.user = user;
            row.description = description;
            row.picture_url = picture_url;
            row.shop_name = shop_name;
            row.tag = tag;
            row.address = address;
            row.goodsid = {};
            // 新增店铺默认不可见，需要管理员审核
            row.is_checked = false;
        });
        getid.emplace(user, [&](auto &row) {
            row.sid = getid.available_primary_key() ? getid.available_primary_key() : 1;
        });
    }

    [[eosio::action]] void shopupdate(uint64_t sid, name user, std::string shop_name, std::string description,
                                      std::string picture_url, std::string tag, std::string address)
    {
        require_auth(user);
        shop_index si(get_self(), user.value);
        auto &iterator = si.get(sid, "sid not found, please check if the user own the shop!");
        si.modify(iterator, user, [&](auto &row) {
            row.description = description;
            row.picture_url = picture_url;
            row.shop_name = shop_name;
            row.tag = tag;
            row.address = address;
            // 修改完信息也需要审核
            row.is_checked = false;
        });
    }

    [[eosio::action]] void shoperase(uint64_t sid, name user)
    {
        require_auth(user);
        shop_index si(get_self(), user.value);
        goods_index gi(get_self(), user.value);
        auto &iterator = si.get(sid, "sid not found, please check if the user own the shop!");
        for (auto i : iterator.goodsid)
        {
            auto tmp = gi.find(i);
            if (tmp != gi.end())
            {
                gi.erase(tmp);
            }
        }
        si.erase(iterator);
    }

    [[eosio::action]] void shopcheck(uint64_t sid, name user, bool is_checked)
    {
        require_auth("admin"_n);
        shop_index si(get_self(), user.value);
        auto &iterator = si.get(sid, "sid not found, please check if the user own the shop!");
        if (is_checked)
        {
            si.modify(iterator, user, [&](auto &row) {
                row.is_checked = true;
            });
        }
        else
        {
            si.erase(iterator);
        }
    }

    [[eosio::action]] void goodsinsert(name user, uint64_t shopid, std::string goods_name, std::string description,
                                        std::string picture_url)
    {
        require_auth(user);
        goods_index gi(get_self(), user.value);
        shop_index si(get_self(), user.value);
        auto &iterator = si.get(shopid, "shopid not found, please check if the user own the shop!");
        check(user == iterator.user, "You can't add goods to other user's shop!");
        uint64_t tmp;
        gi.emplace(user, [&](auto &row) {
            tmp = row.gid = gi.available_primary_key() ? gi.available_primary_key() : 1;
            row.user = user;
            row.shopid = shopid;
            row.description = description;
            row.picture_url = picture_url;
            row.goods_name = goods_name;
        });
        si.modify(iterator, user, [&](auto &row) {
            row.goodsid.push_back(tmp);
        });
    }

    [[eosio::action]] void goodsupdate(uint64_t gid, name user, uint64_t shopid, std::string goods_name, std::string description,
                                        std::string picture_url)
    {
        require_auth(user);
        goods_index gi(get_self(), user.value);
        auto &iterator = gi.get(gid, "gid not found");
        check(user == iterator.user && shopid == iterator.shopid, "You can't modify other shop's data!");
        gi.modify(iterator, user, [&](auto &row) {
            row.description = description;
            row.picture_url = picture_url;
            row.goods_name = goods_name;
        });
    }

    [[eosio::action]] void goodserase(uint64_t gid, name user, uint64_t shopid)
    {
        require_auth(user);
        shop_index si(get_self(), user.value);
        goods_index gi(get_self(), user.value);
        auto &iterator1 = si.get(shopid, "shopid not found, please check if the user own the shop!");
        auto &iterator2 = gi.get(gid, "data not found");
        check(user == iterator2.user, "You can't delete other user's data!");
        gi.erase(iterator2);
        si.modify(iterator1, user, [&](auto &row) {
            row.goodsid.erase(std::find(row.goodsid.begin(), row.goodsid.end(), gid));
        });
    }
    [[eosio::action]] void suinsert(name user)
    {
        require_auth("admin"_n);
        shopuser_index sui(get_self(), ("admin"_n).value);
        sui.emplace(user, [&](auto &row) {
            row.user = user;
        });
    }
    [[eosio::action]] void suerase(name user)
    {
        require_auth("admin"_n);
        shopuser_index sui(get_self(), ("admin"_n).value);
        auto &iterator = sui.get(user.value, "user doesn't exist!");
        sui.erase(iterator);
    }

private:
    struct [[eosio::table]] shop
    {
        uint64_t sid;
        name user;
        bool is_checked;
        std::string description;
        std::string picture_url;
        std::string shop_name;
        std::string tag;
        std::string address;
        std::vector<uint64_t> goodsid;
        uint64_t primary_key() const { return sid; }
    };
    struct [[eosio::table]] goods
    {
        uint64_t gid;
        name user;
        uint64_t shopid;
        std::string description;
        std::string picture_url;
        std::string goods_name;
        uint64_t primary_key() const { return gid; }
    };

    struct [[eosio::table]] shopuser
    {
        name user;
        uint64_t primary_key() const { return user.value; }
    };
    typedef eosio::multi_index<"shop.info"_n, shop> shop_index;
    typedef eosio::multi_index<"goods.info"_n, goods> goods_index;
    typedef eosio::multi_index<"shopuser"_n, shopuser> shopuser_index;
};
