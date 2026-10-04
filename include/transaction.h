#pragma once
#include "money.h"
#include <string>
#include <chrono>
#include <stdexcept>

enum class TransactionStatus {
    PENDING,
    COMPLETED,
    REFUNDED,
    FAILED
};

struct Transaction {
    std::string id;
    std::string customer_id;
    Money amount;
    TransactionStatus status;
    std::chrono::system_clock::time_point created_at;

    Transaction(std::string id, std::string customer_id, Money amount)
        : id(std::move(id)),
          customer_id(std::move(customer_id)),
          amount(std::move(amount)),
          status(TransactionStatus::PENDING),
          created_at(std::chrono::system_clock::now()) {}

    std::string status_string() const {
        switch (status) {
            case TransactionStatus::PENDING:   return "PENDING";
            case TransactionStatus::COMPLETED: return "COMPLETED";
            case TransactionStatus::REFUNDED:  return "REFUNDED";
            case TransactionStatus::FAILED:    return "FAILED";
        }
        return "UNKNOWN";
    }
};
