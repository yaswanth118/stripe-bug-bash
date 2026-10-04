#pragma once
#include <string>
#include <stdexcept>

// Money is represented as integer cents to avoid floating-point issues.
// e.g. $12.50 => 1250 cents, currency = "USD"
struct Money {
    long long cents;       // amount in smallest currency unit
    std::string currency;  // ISO 4217 currency code e.g. "USD"

    Money(long long cents, std::string currency)
        : cents(cents), currency(std::move(currency)) {}

    bool operator==(const Money& other) const {
        return cents == other.cents && currency == other.currency;
    }

    bool operator!=(const Money& other) const { return !(*this == other); }

    Money operator+(const Money& other) const {
        if (currency != other.currency)
            throw std::invalid_argument("Currency mismatch in addition");
        return Money(cents + other.cents, currency);
    }

    Money operator-(const Money& other) const {
        if (currency != other.currency)
            throw std::invalid_argument("Currency mismatch in subtraction");
        return Money(cents - other.cents, currency);
    }

    bool operator>=(const Money& other) const {
        if (currency != other.currency)
            throw std::invalid_argument("Currency mismatch in comparison");
        return cents >= other.cents;
    }

    bool operator>(const Money& other) const {
        if (currency != other.currency)
            throw std::invalid_argument("Currency mismatch in comparison");
        return cents > other.cents;
    }

    std::string to_string() const {
        return currency + " " + std::to_string(cents / 100) + "." +
               (cents % 100 < 10 ? "0" : "") + std::to_string(cents % 100);
    }
};
