#include "makecontract.hpp"
#include <algorithm>
#include <eosio/action.hpp>
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>
#include <typeinfo>

namespace mycontract
{
    ACTION makecontract::createcontr(name id, name party_a, name party_b, string title, string summary, string content_hash, extended_asset contract_pay)
    {
        require_auth(party_a);
        contract_table contracts(get_self(), get_self().value);

        check(party_a != party_b, "ERR::CREATECONTR_PARTY_SHOULD_NOT_SAME::Party A and Party B shouldn't be same.");

        check(contracts.find(id.value) == contracts.end(),
              "ERR::CREATECONTR_DUPLIcanE_ID::A Contract with the id already exists. Try again with a different id.");

        check(title.length() > 3, "ERR::CREATECONTR_SHORT_TITLE::Title length is too short.");
        check(summary.length() > 3, "ERR::CREATECONTR_SHORT_SUMMARY::Summary length is too short.");
        check(contract_pay.quantity.symbol.is_valid(), "ERR::CREATECONTR_INVALID_SYMBOL::Invalid pay amount symbol.");
        check(contract_pay.quantity.amount > 0,
              "ERR::CREATECONTR_INVALID_CONTRACT_PAY::Invalid pay amount. Must be greater than 0.");

        action(permission_level{"acctsmanager"_n, "active"_n}, "acctsmanager"_n, "xtransfer"_n,
               make_tuple(party_a, get_self(), contract_pay, std::string("get the paymant from Party A.")))
            .send();

        contracts.emplace(party_a, [&](contr &p) {
            p.contract_id = id;
            p.party_a = party_a;
            p.party_b = party_b;
            p.content_hash = content_hash;
            p.contract_pay = contract_pay;
            p.state = ContractState_pending;
        });
    }

    ACTION makecontract::cancelcontr(name id, name canceler)
    {
        require_auth(canceler);

        contract_table contracts(get_self(), get_self().value);
        const contr &ct =
            contracts.get(id.value, "ERR::CANCELCONTR_CONTRACT_NOT_FOUND::Contract not found.");
        check(canceler == ct.party_a, "ERR::CANCELCONTR_CABCEL_FORBIDDEN::Only Party A can cancel the contract.");
        check(ct.state == ContractState_pending, "ERR::CANCELCONTR_CONTRACT_STATE_ERR::Only ContractState_pending status can cancel the contract.");
        // back
        action(permission_level{"acctsmanager"_n, "active"_n}, "acctsmanager"_n, "xtransfer"_n,
               make_tuple(get_self(), ct.party_a, ct.contract_pay, std::string("return the paymant to Party A.")))
            .send();
        contracts.erase(ct);
    }

    ACTION makecontract::acceptcontr(name id, name accepter)
    {
        require_auth(accepter);

        contract_table contracts(get_self(), get_self().value);
        const contr &ct =
            contracts.get(id.value, "ERR::ACCEPTCONTR_CONTRACT_NOT_FOUND::Contract not found.");
        check(accepter == ct.party_b, "ERR::ACCEPTCONTR_CABCEL_FORBIDDEN::Only Party B can accept the contract.");
        check(ct.state == ContractState_pending, "ERR::ACCEPTCONTR_CONTRACT_STATE_ERR::Contract can only  status canceled from ContractState_pending status.");

        contracts.modify(ct, ct.party_a, [&](contr &c) {
            c.state = ContractState_accepted;
        });
    }
    ACTION makecontract::approved(name id, name approver)
    {
        require_auth(approver);

        contract_table contracts(get_self(), get_self().value);
        const contr &ct =
            contracts.get(id.value, "ERR::APPROVED_CONTRACT_NOT_FOUND::Contract not found.");
        check(approver == ct.party_a, "ERR::APPROVED_APPROVE_FORBIDDEN::Only Party A can approve the contract.");
        check(ct.state == ContractState_accepted, "ERR::APPROVED_CONTRACT_STATE_ERR::Contract can only approved from ContractState_accepted status.");

        contracts.modify(ct, ct.party_a, [&](contr &c) {
            c.state = ContractState_aoorovaled;
        });
    }
    ACTION makecontract::pay(name id, name payer, extended_asset payment)
    {
        require_auth(payer);

        contract_table contracts(get_self(), get_self().value);
        const contr &ct =
            contracts.get(id.value, "ERR::PAY_CONTRACT_NOT_FOUND::Contract not found.");

        check(payer == ct.party_a, "ERR::PAY_PAY_FORBIDDEN::Only Party A can pay the contract.");
        check(ct.state == ContractState_aoorovaled, "ERR::PAY_CONTRACT_STATE_ERR::Contract can only payed from ContractState_aoorovaled status.");
        check(ct.contract_pay.quantity.symbol.is_valid(), "ERR::PAY_INVALID_SYMBOL::Invalid pay amount symbol.");
        check(ct.contract_pay.quantity.amount > 0,
              "ERR::PAY_INVALID_CONTRACT_PAY::Invalid pay amount. Must be greater than 0.");
        check(ct.contract_pay >= payment,
              "ERR::PAY_INSUFFICIENT_BALANCE. pay asset should less then contract_pay");
        action(permission_level{"acctsmanager"_n, "active"_n}, "acctsmanager"_n, "xtransfer"_n,
               make_tuple(get_self(), ct.party_b, payment, std::string("payment to Party Bd.")))
            .send();

        contracts.modify(ct, ct.party_a, [&](contr &c) {
            c.state = ContractState_aoorovaled;
        });
    }
} // namespace mycontract