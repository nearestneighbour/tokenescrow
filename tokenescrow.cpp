#include "tokenescrow.hpp"
#include "assetstr.hpp"
using namespace eosio;
using std::string;

// Return funds to offer-creator and remove offer from the RAM table
void tokenescrow::deloffer(uint64_t offerid) {
    // Not sure if this assertion every yields true, but just to be sure
    eosio_assert(get_code() == get_self(), "Wrong contract");

    // Check if offer exists in RAM table
    auto it = offers.find(offerid);
    eosio_assert(it != offers.end(), "Offer not found");

    // You can only delete your own offers, so require authentication from offer-creator
    require_auth(it->account);

    // Return funds to offer-creator
    string memo = "Refunding of deleted offer " + std::to_string(offerid);
    action(
        permission_level(get_self(), "active"_n),
        it->offer.contract,
        "transfer"_n,
        tx{get_self(), it->account, it->offer.quantity, memo.c_str()}
    ).send();

    // Remove offer from RAM table
    offers.erase(it);
}

// Create new offer and add to RAM table
void tokenescrow::newoffer(uint64_t oid, name acc, extended_asset off, extended_asset pr) {
    // You can only add an offer for yourself (already managed in transfer function)
    //require_auth(account);

    // Check if offer already exists
    auto it = offers.find(oid);
    eosio_assert(it == offers.end(), "Offer ID already taken");

    // TO DO: check if asset symbol matches with contract

    // Add offer to RAM table
    offers.emplace(get_self(), [&](auto& offr) { // who pays for RAM?
        offr.offerid = oid;
        offr.account = acc;
        offr.offer = off;
        offr.price = pr;
    });
}

// Accept an existing offer
void tokenescrow::takeoffer(name buyer, uint64_t offerid, asset quantity) {
    // Check if offer exists in RAM table
    auto it = offers.find(offerid);
    eosio_assert(it != offers.end(), "Offer not found");

    // Perform a number of assertions to check whether the submitted asset matches
    // with the asset specified in the offer. Compare asset symbol, contract and amount.
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
    // Forward submitted asset to offer-creator
    action(
        permission_level(get_self(), "active"_n),
        it->price.contract,
        "transfer"_n,
        tx{get_self(), it->account, it->price.quantity, memo.c_str()}
    ).send();

    // Send offer asset to offer-taker for payout
    action(
        permission_level(get_self(), "active"_n),
        it->offer.contract,
        "transfer"_n,
        tx{get_self(), buyer, it->offer.quantity, memo.c_str()}
    ).send();

    // Remove offer from RAM table
    offers.erase(it);
}

// This function is called whenever the contract account is included in a transaction.
// This can happen when the account acts as a sender or recipient of the transaction.
// When transferring funds to the account, the memo is used to specify the goal
// of the transaction - either to create, or accept an offer.
void tokenescrow::transfer(name from, name to, asset quantity, string memo) {
    // Only act on incoming transactions, ignore outgoing
    if (to != get_self()) {
        return;
    }

    // Verify transaction amount. Might be redundant, possibly useful safety check
    eosio_assert(quantity.amount>0, "Only positive transfer amount allowed");

    // Parse transaction memo to get action parameters - args are separated with comma
    // TO DO: check for errors in memo (more extensively than now)
    size_t index1 = memo.find(',');
    // Func is either 'newoffer' or 'takeoffer'
    string func = memo.substr(0,index1);
    if (func == "takeoffer") {
        // Only arg for takeoffer is offerid
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
        // Unreadable memo error
        eosio_assert(false, "Weird memo");
    }
}

extern "C" {
    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        switch (action) {
            EOSIO_DISPATCH_HELPER(tokenescrow, (deloffer)(transfer))
        }
    }
}
