#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
using namespace eosio;
using std::string;

// Code adapted from:
// https://github.com/EOSIO/eos/blob/master/libraries/chain/asset.cpp
// https://github.com/EOSIO/eos/blob/master/libraries/chain/include/eosio/chain/symbol.hpp

// Convert std::string to eosio::symbol
symbol symbol_from_string(const string& s, uint8_t precision) {
    eosio_assert(!s.empty(), "symbol is empty string");
    const char* str = s.c_str();
    uint32_t len = 0;
    while (str[len])  len++;
    uint64_t result = 0;
    for (uint32_t i = 0; i<len; i++) {
        result |= (uint64_t(str[i]) << (8*(1+i)));
    }
    result |= uint64_t(precision);
    return symbol(result);
}

// Convert std::string to eosio::asset
asset asset_from_string(const string& s) {
    size_t space_pos = s.find(' ');
    eosio_assert((space_pos != string::npos), "Missing space");
    string symbol_str = s.substr(space_pos + 1);
    string amount_str = s.substr(0, space_pos);
    eosio_assert(amount_str[0] != '-', "Negative asset");
    size_t dot_pos = amount_str.find('.');
    if (dot_pos != string::npos) {
        eosio_assert((dot_pos != amount_str.size() - 1), "Missing fraction");
    }

    uint8_t precision;
    if (dot_pos == string::npos) {
        precision = 0;
    } else {
        precision = amount_str.size() - dot_pos - 1;
    }
    symbol sym = symbol_from_string(symbol_str, precision);

    uint64_t int_part, fract_part;
    if (dot_pos == string::npos) {
        int_part = strtoull(amount_str.c_str(), nullptr, 10);
        fract_part = 0;
    } else {
        int_part = strtoull(amount_str.substr(0, dot_pos).c_str(), nullptr, 10);
        fract_part = strtoull(amount_str.substr(dot_pos+1).c_str(), nullptr, 10);
    }

    for (int i = 0; i<sym.precision(); i++) {
        int_part *= 10;
    }
    uint64_t amount = int_part + fract_part;
    return asset(amount, sym);
}
