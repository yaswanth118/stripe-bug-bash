#include <gtest/gtest.h>
#include "ledger.h"
#include "money.h"

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------
static Money usd(long long cents) { return Money(cents, "USD"); }

// ---------------------------------------------------------------------------
// TC-01: Basic credit and balance
// ---------------------------------------------------------------------------
TEST(LedgerTest, CreditIncreasesBalance) {
    Ledger ledger;
    ledger.credit("cust_001", usd(10000)); // $100.00
    EXPECT_EQ(ledger.balance("cust_001", "USD").cents, 10000);
}

// ---------------------------------------------------------------------------
// TC-02: Credit with negative amount should be rejected
// A negative credit is economically equivalent to a hidden debit — must throw.
// ---------------------------------------------------------------------------
TEST(LedgerTest, NegativeCreditIsRejected) {
    Ledger ledger;
    ledger.credit("cust_002", usd(5000));
    EXPECT_THROW(ledger.credit("cust_002", usd(-1000)), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// TC-03: Debit reduces balance correctly when funds are sufficient
// ---------------------------------------------------------------------------
TEST(LedgerTest, DebitReducesBalance) {
    Ledger ledger;
    ledger.credit("cust_003", usd(10000));
    ledger.debit("cust_003", usd(3000));
    EXPECT_EQ(ledger.balance("cust_003", "USD").cents, 7000);
}

// ---------------------------------------------------------------------------
// TC-04: Debit should reject overdraft without corrupting balance
// After a failed debit the balance must remain unchanged.
// ---------------------------------------------------------------------------
TEST(LedgerTest, DebitInsufficientFundsLeavesBalanceIntact) {
    Ledger ledger;
    ledger.credit("cust_004", usd(5000)); // $50.00
    EXPECT_THROW(ledger.debit("cust_004", usd(10000)), std::runtime_error);
    // Balance must still be $50.00 — NOT negative
    EXPECT_EQ(ledger.balance("cust_004", "USD").cents, 5000);
}

// ---------------------------------------------------------------------------
// TC-05: charge() succeeds when customer has enough balance
// ---------------------------------------------------------------------------
TEST(LedgerTest, ChargeSucceedsWithSufficientBalance) {
    Ledger ledger;
    ledger.credit("cust_005", usd(20000)); // $200.00
    ledger.charge("cust_005", "txn_001", usd(5000)); // $50.00
    EXPECT_EQ(ledger.balance("cust_005", "USD").cents, 15000);
}

// ---------------------------------------------------------------------------
// TC-06: charge() must fail and leave balance unchanged when insufficient
// ---------------------------------------------------------------------------
TEST(LedgerTest, ChargeFailsWithInsufficientBalance) {
    Ledger ledger;
    ledger.credit("cust_006", usd(1000)); // $10.00
    EXPECT_THROW(ledger.charge("cust_006", "txn_002", usd(5000)), std::runtime_error);
    // Balance must remain $10.00
    EXPECT_EQ(ledger.balance("cust_006", "USD").cents, 1000);
    // No transactions should be recorded
    EXPECT_TRUE(ledger.get_transactions("cust_006").empty());
}

// ---------------------------------------------------------------------------
// TC-07: charge() must reject duplicate transaction IDs
// ---------------------------------------------------------------------------
TEST(LedgerTest, ChargeDuplicateTxnIdThrows) {
    Ledger ledger;
    ledger.credit("cust_007", usd(30000));
    ledger.charge("cust_007", "txn_dup", usd(5000));
    EXPECT_THROW(ledger.charge("cust_007", "txn_dup", usd(5000)), std::runtime_error);
}

// ---------------------------------------------------------------------------
// TC-08: refund() restores balance and marks transaction REFUNDED
// ---------------------------------------------------------------------------
TEST(LedgerTest, RefundRestoresBalance) {
    Ledger ledger;
    ledger.credit("cust_008", usd(20000));
    ledger.charge("cust_008", "txn_003", usd(8000));
    EXPECT_EQ(ledger.balance("cust_008", "USD").cents, 12000);

    ledger.refund("txn_003");
    EXPECT_EQ(ledger.balance("cust_008", "USD").cents, 20000);

    // Transaction status must be REFUNDED
    auto txns = ledger.get_transactions("cust_008");
    ASSERT_EQ(txns.size(), 1u);
    EXPECT_EQ(txns[0].status, TransactionStatus::REFUNDED);
}

// ---------------------------------------------------------------------------
// TC-09: refund() must only be allowed once per transaction
// A double-refund is a serious financial bug.
// ---------------------------------------------------------------------------
TEST(LedgerTest, DoubleRefundIsRejected) {
    Ledger ledger;
    ledger.credit("cust_009", usd(20000));
    ledger.charge("cust_009", "txn_004", usd(5000));
    ledger.refund("txn_004");
    EXPECT_THROW(ledger.refund("txn_004"), std::runtime_error);
}

// ---------------------------------------------------------------------------
// TC-10: total_revenue() must only count COMPLETED transactions
// Refunded transactions must NOT contribute to revenue.
// ---------------------------------------------------------------------------
TEST(LedgerTest, TotalRevenueExcludesRefunds) {
    Ledger ledger;
    ledger.credit("cust_010", usd(50000));
    ledger.charge("cust_010", "txn_005", usd(10000)); // $100 — keep
    ledger.charge("cust_010", "txn_006", usd(5000));  // $50  — refund
    ledger.refund("txn_006");

    Money rev = ledger.total_revenue("USD");
    // Only txn_005 ($100) should count — txn_006 is refunded
    EXPECT_EQ(rev.cents, 10000);
}

// ---------------------------------------------------------------------------
// TC-11: balance() returns zero for unknown customer
// ---------------------------------------------------------------------------
TEST(LedgerTest, BalanceForUnknownCustomerIsZero) {
    Ledger ledger;
    EXPECT_EQ(ledger.balance("nobody", "USD").cents, 0);
}

// ---------------------------------------------------------------------------
// TC-12: get_transactions() returns transactions in insertion order
// ---------------------------------------------------------------------------
TEST(LedgerTest, GetTransactionsInOrder) {
    Ledger ledger;
    ledger.credit("cust_012", usd(50000));
    ledger.charge("cust_012", "txn_a", usd(1000));
    ledger.charge("cust_012", "txn_b", usd(2000));
    ledger.charge("cust_012", "txn_c", usd(3000));

    auto txns = ledger.get_transactions("cust_012");
    ASSERT_EQ(txns.size(), 3u);
    EXPECT_EQ(txns[0].id, "txn_a");
    EXPECT_EQ(txns[1].id, "txn_b");
    EXPECT_EQ(txns[2].id, "txn_c");
}
