Tutorial/example smart contract that acts as an escrow account for trading EOS tokens.

## Example commands
Offering 1 EOS for 1 IQ (issued by everipediaiq account):
```
cleos transfer client1 escrowcontra "1 EOS" "newoffer,1.000 IQ,everipediaiq,1234"
```
This will create an offer with ID 1234 (necessary for later actions) where client1 offers to exchange 1 EOS for 1 IQ issued by the everipediaiq account.

Accept this order using the following command:
```
cleos push action everipediaiq transfer '{"from":"client2","to":"escrowcontra","quantity":"1 IQ","memo":"takeoffer,1234"}'
```
This will send the necessary funds (1 IQ) to the escrow account, indicating the offer ID (1234) in the memo. The escrow account (escrowcontra) will recognize the incoming transaction, pay out the offer of 1 EOS to the offer-taker (client2), and forward the price (1 IQ) to the offer-creator (client1). Next, the offer will be removed from the RAM table.

If client1 wishes to remove their offer before it is accepted by client2, they will use the following command:
```
cleos push action escrowcontra deloffer '{"offerid":1234}'
```
This will return the offered funds (1 EOS) to the offer-creator (client1) and remove the offer from the RAM table.

## To do

* takeoffer() pays out the offered funds to the offer-buyer, but doesn't forward the submitted funds to the offer-seller. (fixed, untested)

* deloffer() removes the offer from the RAM table but doesn't return the funds. (fixed, untested)

* newoffer() currently only works if the asset precision is correctly specified in the command, e.g. "1.0000 EOS" works but "1 EOS" doesn't. This can be fixed by querying the asset precision from the currency contract (e.g. eosio.token), or maybe via another, more efficient way?

* Extensive testing, different currencies, symbols, contracts, etc. Overflow errors, malicious contracts, etc.

* Resource testing - who pays for RAM? How much RAM/NET/CPU does it consume?
* * Is there a tool for analyzing this?
