#include "rmtab.hpp"
using namespace eosio;
using std::string;

void rmtab::transfer(name from, name to, asset quantity, string memo) {
    // Check whether function was not directly called via push action, but indirect from another funds transfer
    // todo: test whether this is actually necessary
    eosio_assert(get_code() != get_self(), "Use any other token contract to transfer funds");
    // Only act on incoming transactions
    if (to != get_self()) {
        return;
    }
    eosio_assert(quantity.amount>0, "Only positive transfer amount allowed");
    // Parse TX memo to get action parameters - args are separated with comma
    // todo: exception catching
    uint64_t offerid = strtoull(memo.c_str(), nullptr, 10);
    // Check if offer exists
    string msg;
    for (auto it : offer) {
        msg += " " + std::to_string(offerid);
    }
    eosio_assert(false, msg.c_str());
    //offer.erase(it);
}

extern "C" {
    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        switch (action) {
            EOSIO_DISPATCH_HELPER(rmtab, (transfer))
        }
    }
}
