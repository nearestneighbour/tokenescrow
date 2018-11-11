
#include "acctransfer.hpp"

void acctransfer::newoffer(name account, name payout, uint64_t price) {
    permission_level acc_owner(account, "owner"_n);
    require_auth(acc_owner);

    auto it = _offers.find(account.value);
    eosio_assert(it == _offers.end(), "Account already for sale, use deloffer to remove");

    authority myauth = {
        .threshold = 1,
        .keys = {},
        .accounts = {perm_weight{{get_self(),"eosio.code"_n},1}},
        .waits = {}
    };
    action(acc_owner,"eosio"_n,"updateauth"_n,updateauth{account, "active"_n, "owner"_n, myauth}).send();
    action(acc_owner,"eosio"_n,"updateauth"_n,updateauth{account, "owner"_n, "owner"_n, myauth}).send();
    //to do: remove/change all other permissions

    _offers.emplace(get_self(), [&](auto& off) { // who pays for RAM?
        off.account = account;
        off.payout = payout;
        off.price = price;
    });
}

void acctransfer::deloffer(name account) {
    auto it = _offers.find(account.value);
    eosio_assert(it != _offers.end(), "Account not for sale");
    require_auth(it->payout);

    

    _offers.erase(it);
}

EOSIO_DISPATCH(acctransfer, (newoffer)(deloffer))
