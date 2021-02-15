#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/ignore.hpp>
#include <eosio/singleton.hpp>
#include <eosio/time.hpp>

using namespace eosio;
using namespace std;

namespace mycontract
{

    CONTRACT makecontract : public contract
    {

        enum ContractState
        {
            ContractState_pending = 0,
            ContractState_accepted,
            ContractState_aoorovaled,
        };

    public:
        TABLE contr
        {
            name contract_id;
            name party_a;
            name party_b;
            string content_hash;
            extended_asset contract_pay;
            uint8_t state;

            uint64_t primary_key() const { return contract_id.value; }
            uint64_t party_a_key() const { return party_a.value; }
            uint64_t party_b_key() const { return party_b.value; }
        };

        typedef eosio::multi_index<"contracts"_n, contr,
                                   eosio::indexed_by<"partya"_n, eosio::const_mem_fun<contr, uint64_t, &contr::party_a_key>>,
                                   eosio::indexed_by<"partyb"_n, eosio::const_mem_fun<contr, uint64_t, &contr::party_b_key>>>
            contract_table;

        makecontract(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds) {}

        ACTION createcontr(name id, name party_a, name party_b, string title, string summary, string content_hash, extended_asset contract_pay);
        ACTION cancelcontr(name id, name canceler);
        ACTION acceptcontr(name id, name accepter);
        ACTION approved(name id, name approver);
        ACTION pay(name id, name payer, extended_asset payment);
    };
} // namespace mycontract