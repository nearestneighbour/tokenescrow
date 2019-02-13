Simple smart contract that acts as an escrow account for trustless trading of EOS-based tokens. I made it for educational purposes, not with real world usage in mind. No security testing has been done, so use caution when deploying this contract on a real EOS network.

## Example usage
For this example, assume the contract has been uploaded to an account named escrowcontra. Sell 1 EOS for 5 IQ tokens with the following command:
```
cleos transfer client1 escrowcontra "1 EOS" "neworder,5.000 IQ,everipediaiq,1234"
```
This will create an order with ID 1234 in the RAM table where client1 offers to sell 1 EOS for 5 IQ tokens issued by the everipediaiq account.

Accept this order using the following command:
```
cleos push action everipediaiq transfer '{"from":"client2","to":"escrowcontra","quantity":"5 IQ","memo":"fillorder,1234"}'
```
This will send the necessary funds (5 IQ) from client2 to the escrow account, indicating the order ID (1234) in the memo. The escrow account will recognize the incoming transaction, pay out the offered funds (1 EOS) to the buyer (client2), and forward the price funds (5 IQ) to the seller (client1). The order will then be removed from the RAM table.

If client1 wishes to remove their order before it is filled, they will use the following command:
```
cleos push action escrowcontra delorder '{"orderid":1234}'
```
This will return the offered funds (1 EOS) to the seller (client1) and remove the order from the RAM table.

To view existing orders, use the following command:
```
cleos get table escrowcontra escrowcontra orders
```
(in the future it might be possible to see all orders associated with a particular account by using the scope parameter - see to do list at the bottom)

## Uploading the contract
Use the following cleos command to upload the contract to an account named 'escrowcontra':
```
cleos set contract escrowcontra contracts/tokenescrow
```
This assumes the contract files (WASM and ABI) are in the directory contracts/tokenescrow. Next, add the eosio.code permission to your contract account. This is necessary to allow the contract to spend tokens from the account it is uploaded to.
```
cleos set account permission escrowcontra active \
'{ \
    "threshold": 1, \
    "keys": [{"key": "PUBKEY", "weight" :1}], \
    "accounts": [{ \
        "permission": {"actor":"escrowcontra", "permission":"eosio.code"}, \
        "weight": 1 \
    }] \
}' \
owner
```
Where PUBKEY is replaced by the public key for the active permission of the contract account.

## To do
* `neworder()` currently only works if the asset precision is correctly specified in the command, e.g. "1.0000 EOS" works but "1 EOS" doesn't. This can be fixed by retrieving the asset precision from the currency contract, however this takes time and bandwidth. Maybe it's possible via another, more efficient way?
* Add Ricardian clauses to contract
* Resources - how to get the user to pay for RAM?
* Security testing. So far nothing has been done on this segment. What about timing - when simultaneously sending transactions to remove and to accept an order, is there a risk of double payout?
* Add brief README section on compiling the contract code using eosio-cpp. (`eosio-cpp -abigen -o tokenescrow.wasm tokenescrow.cpp`)
* RAM table scope - currently all entries are stored in the same scope (that of the contract account), should each order be stored under the scope of the seller? But how to find orders by order ID without knowing the scope?
* Allow partial filling of orders?
