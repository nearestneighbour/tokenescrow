#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
using namespace eosio;

class [[eosio::contract]] rmtab : public contract {
public:
    rmtab(name receiver, name code, datastream<const char*> ds):
        contract(receiver, code, ds), offer(receiver, code.value) {}

    void transfer(name from, name to, asset quantity, std::string memo);

private:
    struct tx { // Transfer parameters
        name from;
        name to;
        asset quantity;
        std::string memo;
    };

    struct [[eosio::table("offer")]] offer_info {
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
    multi_index<"offer"_n, offer_info> offer;
};
