Tutorial/example smart contract that acts as an escrow account for trading EOS accounts.

## To do

* newoffer(): remove/change non-standard account permissions (owner/active are already taken care of)
  * Idea: set owner/active to contract@eosio.code, remove the rest
  * Idea: set all permissions (including non-standard) to contract@eosio.code
* deloffer(): reset account permissions to original
  * Idea: set all (including potential non-standard) permissions owner/active to payout@active
* Implement takeoffer()
  * price: use asset type instead of uint64_t
  * use eosio.token::transfer() inline or via apply()
* Write tutorial
  * Sam the Seller sells account to Bob the Buyer
  * samselleracc, sampayoutacc, bobsbuyeracc