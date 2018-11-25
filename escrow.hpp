#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
using namespace eosio;

class [[eosio::contract]] escrow : public contract {
public:
    escrow(name receiver, name code, datastream<const char*> ds):
        contract(receiver, code, ds), offers(receiver, receiver.value) {}

    [[eosio::action]] void deloffer(uint64_t offerid);
    void transfer(name from, name to, asset quantity, std::string memo);

private:
    struct tx { // Transfer parameters
        name from;
        name to;
        asset quantity;
        std::string memo;
    };

    struct [[eosio::table("offers")]] offer_info {
        uint64_t offerid;
        name account;
        extended_asset offer; // includes contract name on top of symbol+amount
        extended_asset price;
        uint64_t primary_key() const {return offerid;}
        //uint64_t sec_key() const {return account.value;}
        //EOSLIB_SERIALIZE(offer_info, (account)(offer)(price)(offerid)) // ???
    };

    /*multi_index<"offer"_n, offer_info,
        indexed_by<"offerid"_n, const_mem_fun<offer, uint64_t, &offer::sec_key>>
    > offer_table;*/
    multi_index<"offers"_n, offer_info> offers;

    void newoffer(uint64_t oid, name acc, extended_asset off, extended_asset pr);
    void takeoffer(name buyer, uint64_t offerid, asset quantity);
};
