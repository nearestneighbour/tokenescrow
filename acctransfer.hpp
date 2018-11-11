
#pragma once
#include <eosiolib/eosio.hpp>

using namespace eosio;

class [[eosio::contract]] acctransfer : public contract {
public:
    acctransfer(name receiver, name code, datastream<const char*> ds):
        contract(receiver, code, ds), _offers(receiver, code.value) {}

    [[eosio::action]] void newoffer(name account, name payout, uint64_t price);
    [[eosio::action]] void deloffer(name account);

private:
    struct pubkey {
        uint8_t type;
        std::array<unsigned char, 33> data;
    };
    struct perm_weight {
        permission_level permission;
        uint16_t weight;
    };
    struct key_weight {
        pubkey key;
        uint16_t weight;
    };
    struct wait_weight {
        uint32_t wait_sec;
        uint16_t weight;
    };
    struct authority {
        uint32_t threshold;
        std::vector<key_weight> keys;
        std::vector<perm_weight> accounts;
        std::vector<wait_weight> waits;
    };
    struct updateauth {
        name account;
        name permission;
        name parent;
        authority data;
    };
    struct [[eosio::table]] offer {
        name account;
        name payout;
        uint64_t price; // type?
        uint64_t primary_key() const {return account.value;}
        //EOSLIB_SERIALIZE(offer, (account)(payout)(price)) // ???
    };

    typedef multi_index<"offer"_n, offer> offertable;
    offertable _offers;
};
