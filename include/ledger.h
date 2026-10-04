#pragma once
#include "transaction.h"
#include "money.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>

// Ledger tracks balances per customer and records all transactions.
// Rules:
//   - A customer balance can never go below zero.
//   - Only COMPLETED transactions can be refunded.
//   - A transaction can only be refunded once.
//   - Partial refunds are NOT supported — refunds must match the full amount.
class Ledger {
public:
    // Credit funds into a customer account.
    void credit(const std::string& customer_id, const Money& amount);

    // Debit funds from a customer account.
    // Throws std::runtime_error if balance is insufficient.
    void debit(const std::string& customer_id, const Money& amount);

    // Record a completed transaction against a customer's balance.
    // Returns the transaction id.
    std::string charge(const std::string& customer_id,
                       const std::string& txn_id,
                       const Money& amount);

    // Refund a previously completed transaction.
    // Throws if txn not found, not completed, or already refunded.
    void refund(const std::string& txn_id);

    // Returns balance for a given customer. Returns 0-cent Money if not found.
    Money balance(const std::string& customer_id, const std::string& currency) const;

    // Returns all transactions for a customer (in insertion order).
    std::vector<Transaction> get_transactions(const std::string& customer_id) const;

    // Returns total revenue: sum of all COMPLETED transaction amounts.
    Money total_revenue(const std::string& currency) const;

private:
    // customer_id -> balance (cents)
    std::unordered_map<std::string, long long> balances_;

    // txn_id -> Transaction
    std::unordered_map<std::string, Transaction> transactions_;

    // customer_id -> list of txn_ids (in order)
    std::unordered_map<std::string, std::vector<std::string>> customer_txns_;
};
