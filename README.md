Simple smart contract that acts as an escrow account for trading EOS-based tokens.

## Example usage
Offering 1 EOS for 1 IQ (token issued by everipediaiq account):
```
cleos transfer client1 escrowcontra "1 EOS" "newoffer,1.000 IQ,everipediaiq,1234"
```
This will create an offer with ID 1234 (necessary for future reference) where client1 offers to exchange 1 EOS for 1 IQ issued by the everipediaiq account.

Accept this order using the following command:
```
cleos push action everipediaiq transfer '{"from":"client2","to":"escrowcontra","quantity":"1 IQ","memo":"takeoffer,1234"}'
```
This will send the necessary funds (1 IQ) to the escrow account, indicating the offer ID (1234) in the memo. The escrow account (escrowcontra) will recognize the incoming transaction, pay out the offer (1 EOS) to the offer-taker (client2), and forward the price (1 IQ) to the offer-creator (client1). Next, the offer will be removed from the RAM table.

If client1 wishes to remove their offer before it is accepted by client2, they will use the following command:
```
cleos push action escrowcontra deloffer '{"offerid":1234}'
```
This will return the offered funds (1 EOS) to the offer-creator (client1) and remove the offer from the RAM table.

To view any existing orders, use the following cleos command:
```
cleos get table escrowcontra escrowcontra offers
```

## Uploading the contract
Use the following Cleos commands to upload the contract to an account named 'escrowcontra':
```
cleos set contract excrowcontra contracts/tokenescrow
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
* newoffer() currently only works if the asset precision is correctly specified in the command, e.g. "1.0000 EOS" works but "1 EOS" doesn't. This can be fixed by retrieving the asset precision from the currency contract, however this takes time and bandwidth. Maybe it's possible via another, more efficient way?
* Resources - who pays for RAM, and how to get the user to pay for it? How much RAM/NET/CPU does it consume?
* Security testing. So far nothing has been done on this segment. What about timing - when simultaneously sending transactions to remove and to accept an offer, is there a risk of double payout?
* Add brief readme section on compiling the contract code using eosio-cpp. (eosio-cpp -abigen -o tokenescrow.wasm tokenescrow.cpp)
* RAM table scope - currently all entries are stored in the same scope (that of the contract account), should each offer be stored under the scope of the offer-creator? But how to find offers by Offer ID without knowing the scope?