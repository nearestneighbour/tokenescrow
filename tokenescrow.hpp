#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
using namespace eosio;

class [[eosio::contract]] tokenescrow : public contract {
public:
    tokenescrow(name receiver, name code, datastream<const char*> ds):
        contract(receiver, code, ds), orders(receiver, receiver.value) {}

    // delorder is an ABI action that users can call
    [[eosio::action]] void delorder(uint64_t orderid);
    // transfer is implicitly called when the contract is mentioned in a token transaction
    void transfer(name from, name to, asset quantity, std::string memo);

private:
    // Parameters for transfer function
    struct tx { // Transfer parameters
        name from;
        name to;
        asset quantity;
        std::string memo;
    };

    // Define RAM table to store open orders
    struct [[eosio::table("orders")]] order_info {
        uint64_t orderid;
        name seller;
        extended_asset offer; // includes contract name on top of symbol+amount
        extended_asset price;
        uint64_t primary_key() const {return orderid;}
        //uint64_t sec_key() const {return account.value;} // 2ndary key (not used)

        // I don't know what this does and when it's needed
        //EOSLIB_SERIALIZE(order_info, (orderid)(account)(order)(price)) // ???
    };

    // For future reference if I want to add secondary indexing (currently no reason to)
    /*multi_index<"order"_n, order_info,
        indexed_by<"orderid"_n, const_mem_fun<order, uint64_t, &order::sec_key>>
    > order_table;*/

    // Create RAM table for storing open orders
    multi_index<"orders"_n, order_info> orders;

    // neworder creates a new order with the information provided
    void neworder(uint64_t oid, name acc, extended_asset offer, extended_asset pr);
    // fillorder accepts an open order
    void fillorder(name buyer, uint64_t orderid, asset quantity);
};
