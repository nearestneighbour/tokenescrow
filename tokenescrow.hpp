#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
using namespace eosio;

class [[eosio::contract]] tokenescrow : public contract {
public:
    tokenescrow(name receiver, name code, datastream<const char*> ds):
        contract(receiver, code, ds), offers(receiver, receiver.value) {}

    // deloffer is an ABI action that users can call
    [[eosio::action]] void deloffer(uint64_t offerid);
    // transfer is implicitly called when the contract account is included in a transaction
    void transfer(name from, name to, asset quantity, std::string memo);

private:
    // Parameters for transfer function
    struct tx { // Transfer parameters
        name from;
        name to;
        asset quantity;
        std::string memo;
    };

    // Define RAM table to store open offers
    struct [[eosio::table("offers")]] offer_info {
        uint64_t offerid;
        name account;
        extended_asset offer; // includes contract name on top of symbol+amount
        extended_asset price;
        uint64_t primary_key() const {return offerid;}
        //uint64_t sec_key() const {return account.value;} // 2ndary key (not used)

        // I don't know what this does and when it's needed
        //EOSLIB_SERIALIZE(offer_info, (offerid)(account)(offer)(price)) // ???
    };

    // For future reference if I want to add a 2ndary index (currently no reason to)
    /*multi_index<"offer"_n, offer_info,
        indexed_by<"offerid"_n, const_mem_fun<offer, uint64_t, &offer::sec_key>>
    > offer_table;*/
    
    // Create RAM table for storing open offers
    multi_index<"offers"_n, offer_info> offers;

    // newoffer creates a new offer with the information provided
    void newoffer(uint64_t oid, name acc, extended_asset off, extended_asset pr);
    // takeoffer accepts an open offer
    void takeoffer(name buyer, uint64_t offerid, asset quantity);
};
