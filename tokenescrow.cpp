#include "tokenescrow.hpp"
#include "assetstr.hpp"
using namespace eosio;
using std::string;

// Create new order and add to RAM table
void tokenescrow::neworder(uint64_t oid, name acc, extended_asset offer, extended_asset pr) {
    // You can only add an order for yourself (already managed in transfer function)
    //require_auth(account);

    // Check if order ID already exists
    auto it = orders.find(oid);
    eosio_assert(it == orders.end(), "Order ID already taken");

    // TO DO: check if asset symbol matches with contract

    // Add order to RAM table
    //orders.emplace(get_self(), [&](auto& ord) { // contract pays for RAM
    orders.emplace(acc, [&](auto& ord) { // seller pays for RAM
        ord.orderid = oid;
        ord.seller = acc;
        ord.offer = offer;
        ord.price = pr;
    });
}

// Fill an existing order
void tokenescrow::fillorder(name buyer, uint64_t orderid, asset quantity) {
    // Check if order exists in RAM table
    auto it = orders.find(orderid);
    eosio_assert(it != orders.end(), "Order not found");

    // Perform a number of checks to check whether the submitted asset matches
    // with the asset specified in the order. Compare asset symbol, contract and amount.
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

    string memo = "Payout from order " + std::to_string(orderid);
    // Forward price asset to seller
    action(
        permission_level(get_self(), "active"_n),
        it->price.contract,
        "transfer"_n,
        tx{get_self(), it->seller, it->price.quantity, memo.c_str()}
    ).send();

    // Send offer asset to buyer for payout
    action(
        permission_level(get_self(), "active"_n),
        it->offer.contract,
        "transfer"_n,
        tx{get_self(), buyer, it->offer.quantity, memo.c_str()}
    ).send();

    // Remove order from RAM table
    orders.erase(it);
}

// Return funds to seller and remove order from the RAM table
void tokenescrow::delorder(uint64_t orderid) {
    // Not sure if this assertion ever yields true, but just to be sure
    eosio_assert(get_code() == get_self(), "Wrong contract");

    // Check if order exists in RAM table
    auto it = orders.find(orderid);
    eosio_assert(it != orders.end(), "Order not found");

    // You can only delete your own orders, so require authentication from seller
    require_auth(it->seller);

    // Return funds to seller
    string memo = "Refunding of deleted order " + std::to_string(orderid);
    action(
        permission_level(get_self(), "active"_n),
        it->offer.contract,
        "transfer"_n,
        tx{get_self(), it->seller, it->offer.quantity, memo.c_str()}
    ).send();

    // Remove order from RAM table
    orders.erase(it);
}

// This function is called whenever the contract account is included in a transaction.
// This can happen when the account acts as a sender or recipient of the transaction.
// When transferring funds to the account, the memo is used to specify the goal
// of the transaction - either to create, or fill an order.
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
    // Func is either 'neworder' or 'fillorder'
    string func = memo.substr(0,index1);
    if (func == "fillorder") {
        // Only arg for fillorder is orderid
        uint64_t orderid = strtoull(memo.substr(index1+1).c_str(), nullptr, 10);
        fillorder(from, orderid, quantity);
    } else if (func == "neworder") {
        // Parameters for neworder are: price ("1.0000 EOS"), price contract, orderid
        size_t index2 = memo.find(',', index1+1);
        size_t index3 = memo.find(',', index2+1);
        asset price = asset_from_string(memo.substr(index1+1, index2-index1-1));
        name pricecode = name(memo.substr(index2+1, index3-index2-1));
        uint64_t orderid = strtoull(memo.substr(index3+1).c_str(), nullptr, 10);
        neworder(
            orderid,
            from,
            extended_asset(quantity, get_code()),
            extended_asset(price, pricecode)
        );
    } else {
        // Unreadable memo error
        eosio_assert(false, "Weird memo");
    }
}

// Define the contract apply handler. transfer and delorder are declared in the
// contract ABI and can be called directly; neworder and fillorder are can only be
// called indirectly by using transfer.
extern "C" {
    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        switch (action) {
            EOSIO_DISPATCH_HELPER(tokenescrow, (transfer)(delorder))
        }
    }
}
