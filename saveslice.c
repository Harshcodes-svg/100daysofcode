# SaveSlice 🍰

> **Spend. Save. Repeat.**
> A command-line expense tracker written in C that quietly slices **10% of every expense** into a savings wallet.

Built by **Team `.env`** for our class mini hackathon (Programming in C).

---

## The Problem

Small daily spends add up, nobody tracks them, and "I'll save whatever is left" usually means saving nothing.

## The Solution

SaveSlice takes the decision away. Every time you log an expense, **10% of it is moved into a separate savings wallet automatically**. That money is *not* added back into next month's spending budget, so it really is saved.

---

## Features

- **Automatic savings wallet**: 10% of each expense is set aside in a wallet that is separate from your spending budget.
- **Time-locked savings**: each month's savings unlock **5 months** after that month, so the wallet really stays saved.
- **Emergency withdrawal**: need the money early? Pick a reason (Medical, Family, Education, Other), confirm, and withdraw from locked savings.
- **Monthly budget and top-ups**: set a budget once per month and add extra money to the current month any time.
- **Automatic new month**: a new month starts by itself on the 1st, even if the program was left running overnight.
- **Daily budget**: shows what you can spend per day for the rest of the month (today included).
- **Overspend warning**: asks for confirmation before an expense goes over what is left.
- **Monthly summary**: budget, money added, expenses, savings, remaining balance and a category chart drawn with `#` bars.
- **Past month view**: look up the summary and expenses of any earlier month.
- **Backdated entries**: choose the day of an expense (press Enter for today).
- **Safe input**: letters, negatives, empty and oversized input are rejected with a clear message.

---

## How It Works

1. **Launch**: reads today's date, loads this month's record or asks for a budget.
2. **Log**: validates the amount and adds 10% on top as savings.
3. **Save**: appends the record to a CSV file immediately, so nothing is lost if the program closes.
4. **Report**: summary and wallet are totalled fresh from the files each time.
5. **Unlock**: the wallet checks each saving's age, and only savings that are 5 months old can be withdrawn normally.

```text
saved       = (amount + 5) / 10     (integer paise, rounded to nearest paisa)
budget used = amount + saved
wallet     += saved
```

### Savings lock and withdrawals

- Each month's savings unlock **5 months** after the month they were saved (savings from June unlock in November).
- **Normal withdrawal:** only up to the unlocked amount, with a confirmation.
- **Emergency withdrawal:** can use locked savings, but you must choose a reason and confirm that it is a real emergency.
- Withdrawals use the oldest savings first.
- Withdrawn money leaves the wallet for good. It does **not** return to your budget.

---

## Getting Started

### Requirements

- A C compiler (`gcc`)
- Linux, macOS, or Windows with WSL

### Compile

```bash
gcc -Wall -Wextra tracker.c -o tracker -lm
```

The `-lm` flag links the math library (needed for `llround`).

### Run

```bash
./tracker
```

### Main Menu

```text
1. Add Expense
2. View Expenses
3. Monthly Summary
4. Savings Wallet
5. Add Money to This Month
6. View Past Month
7. Exit
```

### Sample Session

```text
Expense amount (Rs.): 200
Expense recorded.
Actual spent:                 Rs. 200.00
10% moved to wallet:          Rs. 20.00
Total deducted from budget:   Rs. 220.00
```

```text
Wallet balance:               Rs. 50.00
Unlocked (withdrawable):      Rs. 30.00
Still locked:                 Rs. 20.00
Savings unlock 5 months after the month they were saved.
```

---

## Data Files

SaveSlice stores everything in plain CSV files created next to the program. All money is stored as **whole paise** (integers), so there is no floating point rounding drift.

| File | Columns |
| --- | --- |
| `monthly_budgets.csv` | `year,month,budget` |
| `expenses_v3.csv` | `year,month,day,name,amount,category,saved` |
| `topups.csv` | `year,month,day,amount` |
| `withdrawals.csv` | `type,year,month,day,amount,reason` (type is `NORMAL` or `EMERGENCY`) |

**Categories:** Food, Home, Work, Fun, Misc.

To start fresh, delete these four files.

---

## Built With

- **C** (standard library only: `stdio`, `stdlib`, `string`, `time`, `math`)
- **gcc** on Ubuntu (WSL)
- CSV files for storage, with `fgets` + `sscanf` for input validation

---

## Known Limitations

- Expenses cannot be edited or deleted from inside the program (edit the CSV by hand if needed).
- Withdrawals cannot be undone.
- Emergencies are taken on trust: the program does not check proof.
- Expense names cannot contain commas (they would break the CSV format).
- Amounts are in Rupees (Rs.) and the date comes from the system clock.

## What's Next

- Savings goals with progress bars
- Safe editing of past expenses
- Charts and monthly reports

---

## Contributors

**Team `.env`**

- Arnav Mathur
- Divyendra Singh Rajawat
- Harshvardhan Shandilya

---

*Thanks for reading.*

```c
return 0; /* thank you! */
```
