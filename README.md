# Stripe Bug Bash — C++ Interview Practice

A realistic C++ debugging challenge modelled after Stripe's **Bug Bash** interview round.

## The Scenario

You've been handed a payment processing library that a colleague wrote. It implements a `Ledger` that tracks customer balances and transactions. There are **failing tests** — your job is to find and fix all the bugs.

## Project Structure

```
stripe-bug-bash/
├── include/
│   ├── money.h          # Money type (integer cents)
│   ├── transaction.h    # Transaction struct
│   └── ledger.h         # Ledger class interface
├── src/
│   └── ledger.cpp       # ← The buggy implementation
└── tests/
    └── ledger_test.cpp  # Test suite — run these to find failures
```

## Getting Started

```bash
# Clone the repo
git clone https://github.com/yaswanth118/stripe-bug-bash.git
cd stripe-bug-bash

# Configure and build
cmake -S . -B build
cmake --build build

# Run the tests — some will fail!
cd build && ctest --output-on-failure
```

## Rules (simulate the real interview)

- You have **45 minutes**
- You may use your IDE debugger, grep, and any standard tools
- Focus on `src/ledger.cpp` — the headers are correct
- Think out loud / comment your reasoning as you go
- Fix bugs without breaking passing tests

## Hints (read only if stuck)

<details>
<summary>Hint 1 — How many bugs?</summary>
There are <strong>5 bugs</strong> in <code>src/ledger.cpp</code>.
</details>

<details>
<summary>Hint 2 — Categories</summary>
Look for: validation gaps, ordering issues, missing state updates, and incorrect filter logic.
</details>

Good luck!
