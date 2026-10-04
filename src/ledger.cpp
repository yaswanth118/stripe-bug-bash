#include "ledger.h"
#include <stdexcept>
#include <numeric>

// BUG #1: credit() adds to balance correctly, but does NOT validate that
// amount.cents > 0. Negative credits are silently accepted, effectively
// draining a customer's balance without going through debit().

void Ledger::credit(const std::string& customer_id, const Money& amount) {
    balances_[customer_id] += amount.cents;
}

// BUG #2: debit() checks balance AFTER subtracting, not before.
// A customer with $5 can be debited $10 — the check fires too late,
// and the balance is left corrupted (negative) before the exception is thrown.

void Ledger::debit(const std::string& customer_id, const Money& amount) {
    balances_[customer_id] -= amount.cents;
    if (balances_[customer_id] < 0) {
        throw std::runtime_error("Insufficient balance for customer: " + customer_id);
    }
}

// BUG #3: charge() marks the transaction COMPLETED immediately without
// checking if the customer has enough balance. It should call debit() first
// (which validates balance), then mark as COMPLETED.
// As written, a customer can be charged more than their balance and the txn
// is marked COMPLETED even if the underlying debit would have failed.

std::string Ledger::charge(const std::string& customer_id,
                            const std::string& txn_id,
                            const Money& amount) {
    if (transactions_.count(txn_id)) {
        throw std::runtime_error("Duplicate transaction id: " + txn_id);
    }

    Transaction txn(txn_id, customer_id, amount);
    txn.status = TransactionStatus::COMPLETED;  // marked before debit!
    transactions_.emplace(txn_id, txn);
    customer_txns_[customer_id].push_back(txn_id);

    debit(customer_id, amount);  // debit happens after — balance corrupted on throw
    return txn_id;
}

// BUG #4: refund() credits the customer back but forgets to update the
// transaction status to REFUNDED. The transaction stays as COMPLETED,
// meaning the same transaction can be refunded multiple times.

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
    // Missing: txn.status = TransactionStatus::REFUNDED;
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

// BUG #5: total_revenue() counts REFUNDED transactions as revenue.
// Refunded transactions should be excluded — only COMPLETED ones count.

Money Ledger::total_revenue(const std::string& currency) const {
    long long total = 0;
    for (const auto& [id, txn] : transactions_) {
        // Should check: txn.status == TransactionStatus::COMPLETED
        // But instead sums ALL transactions including REFUNDED ones.
        if (txn.status != TransactionStatus::FAILED) {
            total += txn.amount.cents;
        }
    }
    return Money(total, currency);
}
