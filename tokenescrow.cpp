#include "tokenescrow.hpp"
#include "assetstr.hpp"
using namespace eosio;
using std::string;

void tokenescrow::deloffer(uint64_t offerid) {
    eosio_assert(get_code() == get_self(), "Wrong contract");
    // Check if offer exists
    auto it = offers.find(offerid);
    eosio_assert(it != offers.end(), "Offer not found");
    // You can only delete your own offers
    require_auth(it->account);
    // Return funds to account
    string memo = "Refunding of deleted offer " + std::to_string(offerid);
    action(
        permission_level(get_self(), "active"_n),
        it->offer.contract,
        "transfer"_n,
        tx{get_self(), it->account, it->offer.quantity, memo.c_str()}
    ).send();
    // Remove offer from table
    offers.erase(it);
}

void tokenescrow::newoffer(uint64_t oid, name acc, extended_asset off, extended_asset pr) {
    // You can only add an offer for yourself (already asserted in transfer())
    //require_auth(account);
    // Check if offer exists yet
    auto it = offers.find(oid);
    eosio_assert(it == offers.end(), "Offer ID already taken");
    // todo: check if symbol matches with contract
    // Add offer to table
    offers.emplace(get_self(), [&](auto& offr) { // who pays for RAM?
        offr.offerid = oid;
        offr.account = acc;
        offr.offer = off;
        offr.price = pr;
    });
}

void tokenescrow::takeoffer(name buyer, uint64_t offerid, asset quantity) {
    // Check if offer exists
    auto it = offers.find(offerid);
    eosio_assert(it != offers.end(), "Offer not found");
    // Check price assets
    string msg = "Wrong symbol, try " + it->price.quantity.to_string();
    msg += " instead of " + quantity.to_string();
    eosio_assert(
        (quantity.symbol == it->price.quantity.symbol && quantity.amount == it->price.quantity.amount),
        msg.c_str()
    );
    msg = "Wrong contract, try " + it->price.contract.to_string();
    msg += " instead of " + get_code().to_string();
    eosio_assert(get_code() == it->price.contract, msg.c_str());
    msg = "Wrong amount, try " + it->price.quantity.to_string();
    msg += " instead of " + quantity.to_string();
    eosio_assert(quantity == it->price.quantity, msg.c_str());

    string memo = "Payout offer ID " + std::to_string(offerid);
    // Send price assets to offer-creator
    action(
        permission_level(get_self(), "active"_n),
        it->price.contract,
        "transfer"_n,
        tx{get_self(), it->account, it->price.quantity, memo.c_str()}
    ).send();
    // Send offer assets to offer-taker
    action(
        permission_level(get_self(), "active"_n),
        it->offer.contract,
        "transfer"_n,
        tx{get_self(), buyer, it->offer.quantity, memo.c_str()}
    ).send();
    // Remove offer from table
    offers.erase(it);
}

void tokenescrow::transfer(name from, name to, asset quantity, string memo) {
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
    size_t index1 = memo.find(',');
    // Func is 'newoffer' or 'takeoffer'
    string func = memo.substr(0,index1);
    if (func == "takeoffer") {
        // Only parameter for takeoffer is offerid
        uint64_t offerid = strtoull(memo.substr(index1+1).c_str(), nullptr, 10);
        takeoffer(from, offerid, quantity);
    } else if (func == "newoffer") {
        // Parameters for newoffer are: price ("1.05 EOS"), price contract, offerid
        size_t index2 = memo.find(',', index1+1);
        size_t index3 = memo.find(',', index2+1);
        asset price = asset_from_string(memo.substr(index1+1, index2-index1-1));
        name pricecode = name(memo.substr(index2+1, index3-index2-1));
        uint64_t offerid = strtoull(memo.substr(index3+1).c_str(), nullptr, 10);
        newoffer(
            offerid,
            from,
            extended_asset(quantity, get_code()),
            extended_asset(price, pricecode)
        );
    } else {
        eosio_assert(false, "Retarded memo");
    }
}

extern "C" {
    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        switch (action) {
            EOSIO_DISPATCH_HELPER(tokenescrow, (deloffer)(transfer))
        }
    }
}
