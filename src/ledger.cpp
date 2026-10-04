#include "ledger.h"
#include <stdexcept>
#include <numeric>

void Ledger::credit(const std::string& customer_id, const Money& amount) {
    if (amount.cents <= 0) {
        throw std::invalid_argument("Negative credit is rejected");
    }
    balances_[customer_id] += amount.cents;
}

void Ledger::debit(const std::string& customer_id, const Money& amount) {
    if (balances_[customer_id] - amount.cents < 0) {
        throw std::runtime_error("Insufficient balance for customer: " + customer_id);
    }
    balances_[customer_id] -= amount.cents;
}

std::string Ledger::charge(const std::string& customer_id,
                            const std::string& txn_id,
                            const Money& amount) {
    if (transactions_.count(txn_id)) {
        throw std::runtime_error("Duplicate transaction id: " + txn_id);
    }

    Transaction txn(txn_id, customer_id, amount);

    try {
        debit(customer_id, amount);
    } catch(std::runtime_error& e) {
        throw std::runtime_error("Insufficient balance for customer & transaction: " + customer_id + " " + txn_id);
    }

    txn.status = TransactionStatus::COMPLETED;
    transactions_.emplace(txn_id, txn);
    customer_txns_[customer_id].push_back(txn_id);
    return txn_id;
}

void Ledger::refund(const std::string& txn_id) {
    auto it = transactions_.find(txn_id);
    if (it == transactions_.end()) {
        throw std::runtime_error("Transaction not found: " + txn_id);
    }

    Transaction& txn = it->second;
    if (txn.status != TransactionStatus::COMPLETED) {
        throw std::runtime_error("Can only refund COMPLETED transactions");
    }

    credit(txn.customer_id, txn.amount);
    txn.status = TransactionStatus::REFUNDED;
}

Money Ledger::balance(const std::string& customer_id,
                       const std::string& currency) const {
    auto it = balances_.find(customer_id);
    if (it == balances_.end()) {
        return Money(0, currency);
    }
    return Money(it->second, currency);
}

std::vector<Transaction> Ledger::get_transactions(
    const std::string& customer_id) const {
    std::vector<Transaction> result;
    auto it = customer_txns_.find(customer_id);
    if (it == customer_txns_.end()) return result;

    for (const auto& txn_id : it->second) {
        result.push_back(transactions_.at(txn_id));
    }
    return result;
}

Money Ledger::total_revenue(const std::string& currency) const {
    long long total = 0;
    for (const auto& [id, txn] : transactions_) {
        if (txn.status == TransactionStatus::COMPLETED) {
            total += txn.amount.cents;
        }
    }
    return Money(total, currency);
}
