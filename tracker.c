/*
 * Expense Tracker
 * - Every expense also sets aside 10% into a separate Savings Wallet.
 * - Wallet money is NOT part of the spending budget.
 * - Savings unlock 5 months after the month they were saved.
 * - Emergency withdrawals can use locked savings.
 * - A new month starts automatically on the 1st (real calendar date).
 * - Money is stored as integer paise (no floating point drift).
 *
 * Compile: gcc -Wall -Wextra tracker.c -o tracker -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define BUDGET_FILE  "monthly_budgets.csv"   /* year,month,budget            */
#define EXPENSE_FILE "expenses_v3.csv"       /* y,m,d,name,actual,cat,saved  */
#define TOPUP_FILE   "topups.csv"            /* year,month,day,amount        */
#define WITHDRAW_FILE "withdrawals.csv"     /* type,year,month,day,amount,reason */

#define MAX_LINE     512
#define MAX_NAME     100
#define MAX_CAT      20
#define MAX_AMOUNT   1000000000.0
#define N_CATEGORIES 5
#define LOCK_MONTHS  5
#define N_REASONS    4

typedef long long Money;

typedef struct {
    int year, month;
    Money budget;               /* base budget set at start of the month */
} Month;

typedef struct {
    int year, month, day;
    char name[MAX_NAME];
    Money actual;
    char category[MAX_CAT];
    Money saved;
} Expense;

typedef struct {
    Money spent, saved;
    Money by_category[N_CATEGORIES];
} Totals;

static const char *categories[] = {"Food", "Home", "Work", "Fun", "Misc"};
static const char *reasons[] = {"Medical", "Family", "Education", "Other"};

enum {
    MENU_ADD = 1, MENU_VIEW, MENU_SUMMARY, MENU_WALLET,
    MENU_TOPUP, MENU_PAST, MENU_EXIT
};
static const char *menu_items[] = {
    "Add Expense", "View Expenses", "Monthly Summary", "Savings Wallet",
    "Add Money to This Month", "View Past Month", "Exit"
};
#define N_MENU ((int)(sizeof(menu_items) / sizeof(menu_items[0])))

/* ---------- input helpers ---------- */

static int read_line(char *s, size_t n) {
    if (!fgets(s, (int)n, stdin)) return 0;
    size_t k = strcspn(s, "\n");
    if (s[k] == '\n') s[k] = 0;
    else { int c; while ((c = getchar()) != '\n' && c != EOF) {} }
    k = strlen(s);
    if (k && s[k - 1] == '\r') s[k - 1] = 0;
    return 1;
}

/* allow_zero = 0 -> amount must be > 0; allow_zero = 1 -> 0 is fine */
static Money ask_money(const char *prompt, int allow_zero) {
    char line[MAX_LINE], extra;
    double d;
    while (1) {
        printf("%s", prompt);
        if (!read_line(line, sizeof(line))) exit(0);
        if (sscanf(line, " %lf %c", &d, &extra) == 1 && isfinite(d) &&
            d <= MAX_AMOUNT && (allow_zero ? d >= 0 : d > 0))
            return llround(d * 100);
        printf("Enter a valid amount%s, e.g. 250 or 250.50.\n",
               allow_zero ? " (0 or more)" : " greater than 0");
    }
}

static int ask_int(const char *prompt, int min, int max) {
    char line[MAX_LINE], extra;
    int n;
    while (1) {
        printf("%s", prompt);
        if (!read_line(line, sizeof(line))) exit(0);
        if (sscanf(line, " %d %c", &n, &extra) == 1 && n >= min && n <= max)
            return n;
        printf("Please enter a number between %d and %d.\n", min, max);
    }
}

/* Enter = today; otherwise a day between 1 and today */
static int ask_day(int today) {
    char line[MAX_LINE], extra;
    int d;
    while (1) {
        printf("Day of month [1-%d, Enter = today]: ", today);
        if (!read_line(line, sizeof(line))) exit(0);
        if (!line[0]) return today;
        if (sscanf(line, " %d %c", &d, &extra) == 1 && d >= 1 && d <= today)
            return d;
        printf("Enter a day between 1 and %d.\n", today);
    }
}

/* Reads a name without commas (commas would break the CSV files) */
static void ask_name(const char *prompt, char *name, const char *fallback) {
    do {
        printf("%s", prompt);
        if (!read_line(name, MAX_NAME)) exit(0);
        if (strchr(name, ',')) puts("Please avoid commas in names.");
    } while (strchr(name, ','));
    if (!name[0]) strcpy(name, fallback);
}

/* ---------- output helpers ---------- */

static void print_money(Money p) {
    const char *sign = p < 0 ? "-" : "";
    if (p < 0) p = -p;
    printf("Rs. %s%lld.%02lld", sign, p / 100, p % 100);
}

static void show(const char *label, Money amount) {
    printf("%-30s", label);
    print_money(amount);
    puts("");
}

/* ---------- date helpers ---------- */

static void today_date(int *y, int *mo, int *d) {
    time_t t = time(NULL);
    struct tm *l = localtime(&t);
    if (!l) { puts("Cannot determine date."); exit(1); }
    *y = l->tm_year + 1900;
    *mo = l->tm_mon + 1;
    *d = l->tm_mday;
}

static int days_in_month(int y, int m) {
    static const int d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) return 29;
    return d[m - 1];
}

/* ---------- file access ---------- */

static int parse_expense(const char *line, Expense *e) {
    return sscanf(line, "%d,%d,%d,%99[^,],%lld,%19[^,],%lld",
                  &e->year, &e->month, &e->day, e->name,
                  &e->actual, e->category, &e->saved) == 7;
}

static int get_month(int year, int month, Month *out) {
    FILE *f = fopen(BUDGET_FILE, "r");
    if (!f) return 0;
    char line[MAX_LINE];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        Month m;
        if (sscanf(line, "%d,%d,%lld", &m.year, &m.month, &m.budget) == 3 &&
            m.year == year && m.month == month) { *out = m; found = 1; }
    }
    fclose(f);
    return found;
}

static int append_month(const Month *m) {
    FILE *f = fopen(BUDGET_FILE, "a");
    if (!f) return 0;
    int ok = fprintf(f, "%d,%d,%lld\n", m->year, m->month, m->budget) > 0;
    if (fclose(f)) ok = 0;
    return ok;
}

static Totals month_totals(int year, int month) {
    Totals t = {0};
    FILE *f = fopen(EXPENSE_FILE, "r");
    if (!f) return t;
    char line[MAX_LINE];
    Expense e;
    while (fgets(line, sizeof(line), f)) {
        if (!parse_expense(line, &e) || e.year != year || e.month != month)
            continue;
        t.spent += e.actual;
        t.saved += e.saved;
        for (int i = 0; i < N_CATEGORIES; ++i)
            if (strcmp(e.category, categories[i]) == 0) {
                t.by_category[i] += e.actual;
                break;
            }
    }
    fclose(f);
    return t;
}

static Money wallet_total(void) {
    Money total = 0;
    FILE *f = fopen(EXPENSE_FILE, "r");
    if (!f) return 0;
    char line[MAX_LINE];
    Expense e;
    while (fgets(line, sizeof(line), f))
        if (parse_expense(line, &e)) total += e.saved;
    fclose(f);
    return total;
}

static Money month_topups(int year, int month) {
    Money total = 0;
    FILE *f = fopen(TOPUP_FILE, "r");
    if (!f) return 0;
    char line[MAX_LINE];
    int y, mo, d;
    Money amount;
    while (fgets(line, sizeof(line), f))
        if (sscanf(line, "%d,%d,%d,%lld", &y, &mo, &d, &amount) == 4 &&
            y == year && mo == month) total += amount;
    fclose(f);
    return total;
}

static Money available(const Month *m) {
    return m->budget + month_topups(m->year, m->month);
}

/* withdrawals.csv: type (NORMAL or EMERGENCY),year,month,day,amount,reason */
static Money wallet_withdrawn(void) {
    Money total = 0;
    FILE *f = fopen(WITHDRAW_FILE, "r");
    if (!f) return 0;
    char line[MAX_LINE];
    Money amount;
    while (fgets(line, sizeof(line), f))
        if (sscanf(line, "%*[^,],%*d,%*d,%*d,%lld", &amount) == 1) total += amount;
    fclose(f);
    return total;
}

static Money wallet_balance(void) {
    return wallet_total() - wallet_withdrawn();
}

/* Savings from months that are at least LOCK_MONTHS old */
static Money wallet_matured(int year, int month) {
    Money total = 0;
    FILE *f = fopen(EXPENSE_FILE, "r");
    if (!f) return 0;
    char line[MAX_LINE];
    Expense e;
    while (fgets(line, sizeof(line), f)) {
        if (!parse_expense(line, &e)) continue;
        int age = (year * 12 + month) - (e.year * 12 + e.month);
        if (age >= LOCK_MONTHS) total += e.saved;
    }
    fclose(f);
    return total;
}

/* Withdrawals use the oldest savings first, so they come out of matured money */
static Money wallet_unlocked(const Month *m) {
    Money open = wallet_matured(m->year, m->month) - wallet_withdrawn();
    Money balance = wallet_balance();
    if (open < 0) open = 0;
    return open > balance ? balance : open;
}

static int append_withdrawal(const char *type, Money amount, const char *reason) {
    int y, mo, d;
    today_date(&y, &mo, &d);
    FILE *f = fopen(WITHDRAW_FILE, "a");
    if (!f) return 0;
    int ok = fprintf(f, "%s,%d,%d,%d,%lld,%s\n", type, y, mo, d, amount, reason) > 0;
    if (fclose(f)) ok = 0;
    return ok;
}

/* ---------- month setup ---------- */

static int initialize_month(Month *m, int year, int month) {
    if (get_month(year, month, m)) return 1;
    printf("\nNew month: %04d-%02d.\n", year, month);
    Money budget = ask_money("Enter your monthly budget (Rs.): ", 1);
    *m = (Month){year, month, budget};
    return append_month(m);
}

/* ---------- features ---------- */

static void summary(const Month *m, int today) {
    Totals t = month_totals(m->year, m->month);
    Money topups = month_topups(m->year, m->month);
    Money avail = m->budget + topups;
    Money remaining = avail - t.spent - t.saved;

    printf("\n======= SUMMARY %04d-%02d =======\n", m->year, m->month);
    show("Monthly budget:", m->budget);
    show("Money added:", topups);
    show("Available this month:", avail);
    show("Actual expenses:", t.spent);
    show("Moved to savings wallet:", t.saved);
    show("Remaining to spend:", remaining);

    if (today > 0) {   /* only meaningful for the current month */
        int days_left = days_in_month(m->year, m->month) - today + 1;
        if (remaining > 0) {
            printf("Daily budget (%d days incl. today): ", days_left);
            print_money(remaining / days_left);
            puts("");
        } else {
            puts("Daily budget: budget used up.");
        }
    }

    puts("\nCategory totals (actual spending):");
    Money largest = 0;
    for (int i = 0; i < N_CATEGORIES; ++i)
        if (t.by_category[i] > largest) largest = t.by_category[i];
    for (int i = 0; i < N_CATEGORIES; ++i) {
        printf("%-5s | ", categories[i]);
        int bars = largest ? (int)(40.0 * t.by_category[i] / largest) : 0;
        for (int j = 0; j < bars; ++j) putchar('#');
        printf(" ");
        print_money(t.by_category[i]);
        puts("");
    }
}

static void add_expense(const Month *m, int today) {
    char name[MAX_NAME];
    puts("\n--- Add Expense ---");
    ask_name("Expense name (optional, press Enter to skip): ", name, "Unnamed");

    Money actual = ask_money("Expense amount (Rs.): ", 0);
    puts("Categories:");
    for (int i = 0; i < N_CATEGORIES; ++i) printf("%d. %s\n", i + 1, categories[i]);
    int c = ask_int("Choose category: ", 1, N_CATEGORIES) - 1;
    int day = ask_day(today);

    Money saved = (actual + 5) / 10;   /* 10%, rounded to nearest paisa */

    Totals t = month_totals(m->year, m->month);
    Money remaining = available(m) - t.spent - t.saved;
    if (actual + saved > remaining) {
        printf("WARNING: this needs ");
        print_money(actual + saved);
        printf(" but only ");
        print_money(remaining);
        puts(" is left.");
        if (ask_int("1. Add anyway  2. Cancel: ", 1, 2) == 2) {
            puts("Cancelled.");
            return;
        }
    }

    FILE *f = fopen(EXPENSE_FILE, "a");
    if (!f) { puts("ERROR: Could not open expense file."); return; }
    int ok = fprintf(f, "%d,%d,%d,%s,%lld,%s,%lld\n", m->year, m->month, day,
                     name, actual, categories[c], saved) > 0;
    if (fclose(f)) ok = 0;
    if (!ok) { puts("ERROR: Could not save expense."); return; }

    puts("\nExpense recorded.");
    show("Actual spent:", actual);
    show("10% moved to wallet:", saved);
    show("Total deducted from budget:", actual + saved);
}

/* Prints this month's expenses and returns how many there were */
static int view_expenses(const Month *m) {
    printf("\n--- Expenses for %04d-%02d ---\n", m->year, m->month);
    FILE *f = fopen(EXPENSE_FILE, "r");
    int count = 0;
    if (f) {
        char line[MAX_LINE];
        Expense e;
        while (fgets(line, sizeof(line), f)) {
            if (!parse_expense(line, &e) || e.year != m->year || e.month != m->month)
                continue;
            printf("%d. %02d/%02d - %s (%s): ", ++count, e.day, e.month, e.name, e.category);
            print_money(e.actual);
            printf(" | saved ");
            print_money(e.saved);
            puts("");
        }
        fclose(f);
    }
    if (!count) puts("No expenses recorded yet.");
    return count;
}

static void add_money(const Month *m, int today) {
    puts("\n--- Add Money to This Month ---");
    Money amount = ask_money("Amount to add (Rs.): ", 0);
    FILE *f = fopen(TOPUP_FILE, "a");
    if (!f) { puts("ERROR: Could not open top-up file."); return; }
    int ok = fprintf(f, "%d,%d,%d,%lld\n", m->year, m->month, today, amount) > 0;
    if (fclose(f)) ok = 0;
    if (!ok) { puts("ERROR: Could not save."); return; }
    puts("Money added.");
    show("Available this month:", available(m));
}

static void withdraw_unlocked(const Month *m) {
    Money open = wallet_unlocked(m);
    puts("\n--- Withdraw Unlocked Savings ---");
    show("Available to withdraw:", open);
    Money amount = ask_money("Amount to withdraw (Rs.): ", 0);
    if (amount > open) {
        puts("That is more than your unlocked savings.");
        return;
    }
    if (ask_int("Confirm withdrawal? 1. Yes  2. No: ", 1, 2) == 2) return;
    if (append_withdrawal("NORMAL", amount, "Matured")) {
        printf("Withdrawn ");
        print_money(amount);
        puts(".");
    } else {
        puts("ERROR: Could not save withdrawal.");
    }
}

static void withdraw_emergency(void) {
    Money balance = wallet_balance();
    puts("\n--- Emergency Withdrawal ---");
    puts("Emergency withdrawals can use savings that are still locked.");
    puts("Reason:");
    for (int i = 0; i < N_REASONS; ++i) printf("%d. %s\n", i + 1, reasons[i]);
    int r = ask_int("Choose reason: ", 1, N_REASONS) - 1;
    show("Wallet balance:", balance);
    Money amount = ask_money("Amount to withdraw (Rs.): ", 0);
    if (amount > balance) {
        puts("That is more than your wallet balance.");
        return;
    }
    if (ask_int("This breaks your savings lock. Is it a real emergency? 1. Yes  2. No: ", 1, 2) == 2)
        return;
    if (append_withdrawal("EMERGENCY", amount, reasons[r])) {
        printf("Emergency withdrawal of ");
        print_money(amount);
        puts(" recorded. Take care!");
    } else {
        puts("ERROR: Could not save withdrawal.");
    }
}

static void show_wallet(const Month *m) {
    enum { A_BACK, A_WITHDRAW, A_EMERGENCY };
    static const char *labels[] = {"Back to main menu", "Withdraw unlocked savings",
                                   "Emergency withdrawal"};
    while (1) {
        Money balance = wallet_balance();
        Money unlocked = wallet_unlocked(m);

        puts("\n--- Savings Wallet ---");
        show("Saved this month:", month_totals(m->year, m->month).saved);
        show("Wallet balance:", balance);
        show("Unlocked (withdrawable):", unlocked);
        show("Still locked:", balance - unlocked);
        show("Withdrawn so far:", wallet_withdrawn());
        printf("Savings unlock %d months after the month they were saved.\n", LOCK_MONTHS);

        int actions[3], n = 0;
        if (unlocked > 0) actions[n++] = A_WITHDRAW;
        if (balance > 0) actions[n++] = A_EMERGENCY;
        actions[n++] = A_BACK;
        for (int i = 0; i < n; ++i) printf("%d. %s\n", i + 1, labels[actions[i]]);

        switch (actions[ask_int("Choose: ", 1, n) - 1]) {
            case A_WITHDRAW:  withdraw_unlocked(m);  break;
            case A_EMERGENCY: withdraw_emergency();  break;
            case A_BACK:      return;
        }
    }
}

static void view_past_month(void) {
    puts("\n--- View Past Month ---");
    int y = ask_int("Year: ", 2000, 2100);
    int mo = ask_int("Month [1-12]: ", 1, 12);
    Month past;
    if (!get_month(y, mo, &past)) {
        puts("No record for that month.");
        return;
    }
    summary(&past, 0);
    view_expenses(&past);
}

/* ---------- main ---------- */

int main(void) {
    int year, month, day;
    today_date(&year, &month, &day);

    Month m;
    puts("==================================");
    puts("        EXPENSE TRACKER");
    puts("==================================");
    if (!initialize_month(&m, year, month)) {
        puts("ERROR: Could not load/save monthly budget.");
        return 1;
    }

    while (1) {
        /* Re-read the date every loop so a new month starts on the 1st,
           even if the program was left open overnight. */
        today_date(&year, &month, &day);
        if (year != m.year || month != m.month) {
            puts("\nA new month has started!");
            if (!initialize_month(&m, year, month)) {
                puts("ERROR: Could not save new month.");
                return 1;
            }
        }

        Totals t = month_totals(m.year, m.month);
        printf("\n========== MAIN MENU ==========\n");
        printf("Month: %04d-%02d  (day %d)\n", m.year, m.month, day);
        show("Available:", available(&m));
        show("Remaining to spend:", available(&m) - t.spent - t.saved);
        for (int i = 0; i < N_MENU; ++i) printf("%d. %s\n", i + 1, menu_items[i]);

        char prompt[40];
        snprintf(prompt, sizeof(prompt), "Choose an option [1-%d]: ", N_MENU);
        switch (ask_int(prompt, 1, N_MENU)) {
            case MENU_ADD:     add_expense(&m, day);   break;
            case MENU_VIEW:    view_expenses(&m);      break;
            case MENU_SUMMARY: summary(&m, day);       break;
            case MENU_WALLET:  show_wallet(&m);        break;
            case MENU_TOPUP:   add_money(&m, day);     break;
            case MENU_PAST:    view_past_month();      break;
            case MENU_EXIT:    puts("Goodbye!");       return 0;
        }
    }
}
