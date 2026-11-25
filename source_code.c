#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_ITEMS 100
#define MENU_FILE "menu.dat"
#define ADMIN_USER "admin"
#define ADMIN_PASS "admin123"

#define BILLS_FILE "bills.txt"
#define TOKEN_FILE "token.dat"
#define SALES_ITEMS_FILE "salesitems.txt"
#define MAX_CART 20
#define DISCOUNT_LIMIT 200.0   /* discount applies above this bill amount */
#define DISCOUNT_RATE 0.10     /* 10% */

/* One food item on the menu */
typedef struct {
    int id;
    char name[50];
    char category[30];
    float price;
    int stock;
} Item;

/* Global array that holds the whole menu in memory */
Item menu[MAX_ITEMS];
int itemCount = 0;

/* ---------- INPUT HELPERS ---------- */

/* Throws away leftover characters (like a wrongly typed word) in the input */
void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Keeps asking until the user types a valid whole number */
int readInt(const char *prompt) {
    int value;
    int result;
    printf("%s", prompt);
    while ((result = scanf("%d", &value)) != 1) {
        if (result == EOF) {      /* input closed, nothing more to read */
            exit(0);
        }
        clearBuffer();
        printf("Invalid input! Enter a number: ");
    }
    return value;
}

/* Same idea, but for decimal numbers (money) */
float readFloat(const char *prompt) {
    float value;
    int result;
    printf("%s", prompt);
    while ((result = scanf("%f", &value)) != 1) {
        if (result == EOF) {
            exit(0);
        }
        clearBuffer();
        printf("Invalid input! Enter an amount: ");
    }
    return value;
}

/* ---------- FILE HANDLING ---------- */

/* Fills the menu with the starting items (used only on the very first run) */
void loadDefaultMenu() {
    Item defaults[9] = {
        {1, "Veg Sandwich", "Snacks", 45.0, 20},
        {2, "Masala Dosa",  "Meals",  60.0, 15},
        {3, "Paneer Roll",  "Snacks", 80.0, 0},   /* out of stock */
        {4, "Cold Coffee",  "Drinks", 50.0, 25},
        {5, "Veg Thali",    "Meals",  120.0, 10},
        {6, "Mojito",       "Drinks", 90.0, 0},   /* out of stock */
        {7, "Veg Maggi",    "Snacks", 20.0, 20},
        {8, "Cheese Maggi", "Snacks", 40.0, 15},
        {9, "Samosa",       "Snacks", 20.0, 30}
    };

    for (int i = 0; i < 9; i++) {
        menu[i] = defaults[i];
    }
    itemCount = 9;
}

void saveMenu();   /* declared here because loadMenu uses it before it is defined */

void loadMenu() {
    FILE *fp = fopen(MENU_FILE, "rb");
    if (fp == NULL) {          /* file doesn't exist yet (first run) */
        loadDefaultMenu();
        saveMenu();
        return;
    }
    itemCount = fread(menu, sizeof(Item), MAX_ITEMS, fp);
    fclose(fp);
}

void saveMenu() {
    FILE *fp = fopen(MENU_FILE, "wb");
    if (fp == NULL) {
        printf("Error: could not save menu.\n");
        return;
    }
    fwrite(menu, sizeof(Item), itemCount, fp);
    fclose(fp);
}

/* ---------- HELPER ---------- */

/* Returns the position of the item with this id, or -1 if not found */
int findItemIndex(int id) {
    for (int i = 0; i < itemCount; i++) {
        if (menu[i].id == id) {
            return i;
        }
    }
    return -1;
}

/* ---------- MENU DISPLAY ---------- */

/* showStock = 1 -> admin view with a Stock column
   showStock = 0 -> customer/cashier view, only "Unavailable" for 0 stock */
void viewMenu(int showStock) {
    printf("\n============ CANTEEN MENU ============\n");
    if (itemCount == 0) {
        printf("No items yet. Ask the admin to add items.\n");
        return;
    }
    printf("%-5s %-20s %-12s %-8s", "ID", "Name", "Category", "Price");
    if (showStock) {
        printf(" %-6s", "Stock");
    }
    printf("\n-----------------------------------------------------------\n");
    for (int i = 0; i < itemCount; i++) {
        printf("%-5d %-20s %-12s %-8.2f",
               menu[i].id, menu[i].name, menu[i].category, menu[i].price);
        if (showStock) {
            printf(" %-6d", menu[i].stock);
        }
        if (menu[i].stock == 0) {
            printf(" Unavailable");
        }
        printf("\n");
    }
}

/* ---------- ADMIN FEATURES ---------- */

void addItem() {
    if (itemCount >= MAX_ITEMS) {
        printf("Menu is full!\n");
        return;
    }

    Item newItem;

    /* new id = highest existing id + 1, so an id is never reused after a delete */
    int maxId = 0;
    for (int i = 0; i < itemCount; i++) {
        if (menu[i].id > maxId) {
            maxId = menu[i].id;
        }
    }
    newItem.id = maxId + 1;

    printf("\n--- Add New Item ---\n");
    printf("Item name: ");
    scanf(" %49[^\n]", newItem.name);
    printf("Category (e.g. Snacks, Drinks): ");
    scanf(" %29[^\n]", newItem.category);
    newItem.price = readFloat("Price: ");
    newItem.stock = readInt("Starting stock: ");

    if (newItem.price <= 0 || newItem.stock < 0) {
        printf("Invalid price or stock. Item not added.\n");
        return;
    }

    menu[itemCount] = newItem;
    itemCount++;
    saveMenu();
    printf("Item added with ID %d.\n", newItem.id);
}

void manageStock() {
    viewMenu(1);
    if (itemCount == 0) {
        return;
    }

    int id = readInt("\nEnter the ID of the item to update: ");

    int index = findItemIndex(id);
    if (index == -1) {
        printf("No item with ID %d.\n", id);
        return;
    }

    printf("Selected: %s (current stock: %d)\n", menu[index].name, menu[index].stock);
    printf("1. Increase stock\n");
    printf("2. Decrease stock\n");
    int choice = readInt("Enter choice: ");

    if (choice != 1 && choice != 2) {
        printf("Invalid choice.\n");
        return;
    }

    int amount = readInt("Enter amount: ");

    if (amount <= 0) {
        printf("Amount must be greater than 0.\n");
        return;
    }

    if (choice == 1) {
        menu[index].stock += amount;
    } else {
        if (amount > menu[index].stock) {
            printf("Cannot decrease by %d. Only %d in stock.\n", amount, menu[index].stock);
            return;
        }
        menu[index].stock -= amount;
    }

    saveMenu();
    printf("Stock updated. %s now has ", menu[index].name);
    if (menu[index].stock == 0) {
        printf("no stock (Unavailable).\n");
    } else {
        printf("%d in stock.\n", menu[index].stock);
    }
}

void editItem() {
    viewMenu(1);
    if (itemCount == 0) {
        return;
    }

    int id = readInt("\nEnter the ID of the item to edit: ");
    int index = findItemIndex(id);
    if (index == -1) {
        printf("No item with ID %d.\n", id);
        return;
    }

    printf("Editing: %s | %s | Rs. %.2f\n",
           menu[index].name, menu[index].category, menu[index].price);
    printf("1. Change name\n");
    printf("2. Change category\n");
    printf("3. Change price\n");
    int choice = readInt("Enter choice: ");

    if (choice == 1) {
        printf("New name: ");
        scanf(" %49[^\n]", menu[index].name);
    } else if (choice == 2) {
        printf("New category: ");
        scanf(" %29[^\n]", menu[index].category);
    } else if (choice == 3) {
        float price = readFloat("New price: Rs. ");
        if (price <= 0) {
            printf("Price must be greater than 0. Nothing changed.\n");
            return;
        }
        menu[index].price = price;
    } else {
        printf("Invalid choice.\n");
        return;
    }

    saveMenu();
    printf("Item updated.\n");
}

void deleteItem() {
    viewMenu(1);
    if (itemCount == 0) {
        return;
    }

    int id = readInt("\nEnter the ID of the item to delete: ");
    int index = findItemIndex(id);
    if (index == -1) {
        printf("No item with ID %d.\n", id);
        return;
    }

    printf("Delete %s? (y/n): ", menu[index].name);
    char confirm;
    scanf(" %c", &confirm);
    if (confirm != 'y' && confirm != 'Y') {
        printf("Cancelled. Nothing deleted.\n");
        return;
    }

    /* shift every later item one place to the left to fill the gap */
    for (int i = index; i < itemCount - 1; i++) {
        menu[i] = menu[i + 1];
    }
    itemCount--;
    saveMenu();
    printf("Item deleted.\n");
}

/* ---------- TOKEN NUMBER ---------- */

int getNextToken() {
    int token = 1001;                     /* first token ever */
    FILE *fp = fopen(TOKEN_FILE, "r");
    if (fp != NULL) {
        fscanf(fp, "%d", &token);
        fclose(fp);
    }
    return token;
}

void saveToken(int token) {
    FILE *fp = fopen(TOKEN_FILE, "w");
    if (fp != NULL) {
        fprintf(fp, "%d", token);
        fclose(fp);
    }
}

/* ---------- UPI QR CODE ---------- */

#define UPI_ID   "nalinanand11974@okaxis"
#define UPI_NAME "Nalin Anand"
#define QR_SIZE  37

/* The QR code as a grid: '1' = black square, '0' = white square */
const char *qrRows[QR_SIZE] = {
    "1111111011011110110110110010101111111",
    "1000001001101101001100000100101000001",
    "1011101000111111011001000001001011101",
    "1011101010010100101000111111101011101",
    "1011101010011001110000111111101011101",
    "1000001010110010110111000000101000001",
    "1111111010101010101010101010101111111",
    "0000000010100011000001110011100000000",
    "1000101110101110100010100100111111001",
    "1011000101011010001101111011100011000",
    "1101001101001111000101110001011010100",
    "1001110001111111111111000011000111110",
    "1010001011110100011001010001001001101",
    "1011000110100111101001111011110010100",
    "0001111000010101000010010001100100110",
    "1100100000000010000000011010010001111",
    "1000111011101000000000000101101101110",
    "1111110100110000000000000011000010010",
    "1101101010101100000000010111101101000",
    "0001010011010000000000011001001001100",
    "1001101100000100000000001000011101111",
    "1111010111100100000000110111110010100",
    "1000011001110101100000110101000001000",
    "0100000010000010000101001011011001100",
    "1010101011110001100010111101011100100",
    "1101010101100100001100100111100010010",
    "0010011010110101010101011001010011100",
    "0011000100000111110111111011110110110",
    "1110111101011000010010111001111110000",
    "0000000010000010101000010111100011100",
    "1111111010111001010010111000101010000",
    "1000001000010001011111110011100011110",
    "1011101011000011001010101000111110110",
    "1011101001010000110110100011011000001",
    "1011101001001000101011111111111100100",
    "1000001000011010010001101001010110110",
    "1111111010011001111111101001011000111"
};

/* 1 if the square at (row, col) is black. Outside the grid is white (the border) */
int qrDark(int row, int col) {
    if (row < 0 || row >= QR_SIZE || col < 0 || col >= QR_SIZE) {
        return 0;
    }
    return qrRows[row][col] == '1';
}

/* Prints the QR code in the terminal using colours, so it scans on any theme.
   Each printed character is the half-block "▀": its text colour is the top
   square and its background colour is the bottom square. That packs two
   rows into one line, so the code is only about 21 lines tall. */
void printQR(float amount) {
#ifdef _WIN32
    /* Windows console: switch on UTF-8 and colour codes */
    SetConsoleOutputCP(65001);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) {
        SetConsoleMode(h, mode | 0x0004);   /* 0x0004 = virtual terminal processing */
    }
#endif

    printf("\n====== SCAN TO PAY WITH UPI ======\n");
    for (int r = -2; r < QR_SIZE + 2; r += 2) {          /* 2 squares high per line */
        for (int c = -2; c < QR_SIZE + 2; c++) {
            int fg = qrDark(r, c)     ? 30 : 97;         /* top square colour    */
            int bg = qrDark(r + 1, c) ? 40 : 107;        /* bottom square colour */
            printf("\033[%d;%dm\xE2\x96\x80", fg, bg);
        }
        printf("\033[0m\n");                              /* reset colours, new line */
    }
    printf("Pay to : %s\n", UPI_NAME);
    printf("UPI ID : %s\n", UPI_ID);
    printf("Amount : Rs. %.2f  (type this amount in your UPI app)\n", amount);
}

/* ---------- BILLING ---------- */

void takeOrder() {
    int cartIdx[MAX_CART];    /* position of each chosen item in menu[] */
    int cartQty[MAX_CART];    /* quantity of each chosen item */
    int cartCount = 0;

    viewMenu(0);
    if (itemCount == 0) {
        return;
    }

    /* --- Step A: build the cart --- */
    while (1) {
        int id = readInt("\nEnter item ID to add (0 to finish): ");
        if (id == 0) {
            break;
        }

        int index = findItemIndex(id);
        if (index == -1) {
            printf("Invalid item ID!\n");
            continue;
        }
        if (menu[index].stock == 0) {
            printf("Sorry, %s is Unavailable.\n", menu[index].name);
            continue;
        }

        int qty = readInt("Enter quantity: ");
        if (qty <= 0) {
            printf("Quantity must be at least 1.\n");
            continue;
        }

        /* is this item already in the cart? */
        int pos = -1;
        for (int i = 0; i < cartCount; i++) {
            if (cartIdx[i] == index) {
                pos = i;
            }
        }
        int alreadyInCart = (pos == -1) ? 0 : cartQty[pos];

        if (qty + alreadyInCart > menu[index].stock) {
            printf("Only %d of %s available.\n",
                   menu[index].stock - alreadyInCart, menu[index].name);
            continue;
        }

        if (pos == -1) {
            if (cartCount >= MAX_CART) {
                printf("Cart is full!\n");
                continue;
            }
            cartIdx[cartCount] = index;
            cartQty[cartCount] = qty;
            cartCount++;
        } else {
            cartQty[pos] += qty;
        }
        printf("Added %d x %s.\n", qty, menu[index].name);
    }

    if (cartCount == 0) {
        printf("No items selected. Order cancelled.\n");
        return;
    }

    /* --- Step B: calculate the bill --- */
    float subtotal = 0;
    for (int i = 0; i < cartCount; i++) {
        subtotal += menu[cartIdx[i]].price * cartQty[i];
    }

    float discount = 0;
    if (subtotal > DISCOUNT_LIMIT) {
        discount = subtotal * DISCOUNT_RATE;
    }
    float total = subtotal - discount;

    /* --- Step C: show the bill --- */
    int token = getNextToken();

    char dateTime[30];
    time_t now = time(NULL);
    strftime(dateTime, sizeof(dateTime), "%d-%m-%Y %H:%M", localtime(&now));

    printf("\n=========== BILL ===========\n");
    printf("Token No : %d\n", token);
    printf("Date     : %s\n", dateTime);
    printf("----------------------------\n");
    printf("%-18s %3s %9s\n", "Item", "Qty", "Amount");
    for (int i = 0; i < cartCount; i++) {
        Item *it = &menu[cartIdx[i]];
        printf("%-18s %3d %9.2f\n", it->name, cartQty[i], it->price * cartQty[i]);
    }
    printf("----------------------------\n");
    printf("%-22s %9.2f\n", "Subtotal", subtotal);
    if (discount > 0) {
        printf("%-22s %9.2f\n", "Discount (10%)", -discount);
    }
    printf("%-22s %9.2f\n", "TOTAL", total);
    printf("============================\n");

    /* --- Step D: payment --- */
    printf("\nPayment method:\n");
    printf("1. Cash\n2. Card\n3. UPI\n0. Cancel order\n");

    int pay;
    while (1) {
        pay = readInt("Enter choice: ");
        if (pay >= 0 && pay <= 3) {
            break;
        }
        printf("Invalid choice!\n");
    }

    if (pay == 0) {
        printf("Order cancelled. No stock was changed.\n");
        return;
    }

    const char *modeName = "Cash";
    if (pay == 1) {
        float given;
        while (1) {
            given = readFloat("Amount given by customer: Rs. ");
            if (given >= total) {
                break;
            }
            printf("Not enough! Total is Rs. %.2f.\n", total);
        }
        printf("Change to return: Rs. %.2f\n", given - total);
        modeName = "Cash";
    } else if (pay == 2) {
        printf("Card payment of Rs. %.2f received.\n", total);
        modeName = "Card";
    } else {
        printQR(total);
        int paid = readInt("\nPayment received? (1 = Yes, 0 = Cancel order): ");
        while (paid != 0 && paid != 1) {
            paid = readInt("Enter 1 or 0: ");
        }
        if (paid == 0) {
            printf("Order cancelled. No stock was changed.\n");
            return;
        }
        printf("UPI payment of Rs. %.2f received.\n", total);
        modeName = "UPI";
    }

    /* --- Step E: payment done, so now update stock and records --- */
    for (int i = 0; i < cartCount; i++) {
        menu[cartIdx[i]].stock -= cartQty[i];
    }
    saveMenu();

    FILE *fp = fopen(BILLS_FILE, "a");     /* "a" = append at the end */
    if (fp != NULL) {
        fprintf(fp, "%d|%s|%.2f|%.2f|%.2f|%s\n",
                token, dateTime, subtotal, discount, total, modeName);
        fclose(fp);
    }
    /* one line per item sold: token|date|item name|quantity|amount */
    FILE *fi = fopen(SALES_ITEMS_FILE, "a");
    if (fi != NULL) {
        for (int i = 0; i < cartCount; i++) {
            Item *it = &menu[cartIdx[i]];
            fprintf(fi, "%d|%s|%s|%d|%.2f\n",
                    token, dateTime, it->name, cartQty[i], it->price * cartQty[i]);
        }
        fclose(fi);
    }
    saveToken(token + 1);

    printf("\nThank you! Your token number is %d.\n", token);

    /* tell the cashier if anything just sold out */
    for (int i = 0; i < cartCount; i++) {
        if (menu[cartIdx[i]].stock == 0) {
            printf("Note: %s is now Unavailable.\n", menu[cartIdx[i]].name);
        }
    }
}

/* ---------- SALES REPORTS ---------- */

/* Adds up the bills whose date matches. offset 0 matches the day (25-11-2026),
   offset 3 matches the month (11-2026), because the date is stored as
   "25-11-2026 15:20". */
void salesSummary(const char *pattern, int offset, const char *label) {
    FILE *fp = fopen(BILLS_FILE, "r");
    if (fp == NULL) {
        printf("No sales recorded yet.\n");
        return;
    }

    char line[200], date[20], mode[20];
    int token, bills = 0;
    float subtotal, discount, total;
    float sumTotal = 0, sumDiscount = 0, cash = 0, card = 0, upi = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "%d|%19[^|]|%f|%f|%f|%19s",
                   &token, date, &subtotal, &discount, &total, mode) != 6) {
            continue;                      /* skip a damaged line */
        }
        if (strncmp(date + offset, pattern, strlen(pattern)) != 0) {
            continue;                      /* a different day or month */
        }
        bills++;
        sumTotal += total;
        sumDiscount += discount;
        if (strcmp(mode, "Cash") == 0)      cash += total;
        else if (strcmp(mode, "Card") == 0) card += total;
        else                                upi += total;
    }
    fclose(fp);

    printf("\n===== SALES: %s =====\n", label);
    if (bills == 0) {
        printf("No sales found for this period.\n");
        return;
    }
    printf("Bills generated  : %d\n", bills);
    printf("Total sales      : Rs. %.2f\n", sumTotal);
    printf("Discount given   : Rs. %.2f\n", sumDiscount);
    printf("  Cash           : Rs. %.2f\n", cash);
    printf("  Card           : Rs. %.2f\n", card);
    printf("  UPI            : Rs. %.2f\n", upi);
}

/* Asks for a date typed as dd-mm-yyyy (or just Enter-0 for today) */
void dailyReport() {
    char date[20];
    printf("Enter date as dd-mm-yyyy (or 0 for today): ");
    scanf("%19s", date);

    if (strcmp(date, "0") == 0) {
        time_t now = time(NULL);
        strftime(date, sizeof(date), "%d-%m-%Y", localtime(&now));
    } else if (strlen(date) != 10 || date[2] != '-' || date[5] != '-') {
        printf("Wrong format. Use dd-mm-yyyy, for example 25-11-2026.\n");
        return;
    }
    salesSummary(date, 0, date);
}

void monthlyReport() {
    char month[20];
    printf("Enter month as mm-yyyy (or 0 for this month): ");
    scanf("%19s", month);

    if (strcmp(month, "0") == 0) {
        time_t now = time(NULL);
        strftime(month, sizeof(month), "%m-%Y", localtime(&now));
    } else if (strlen(month) != 7 || month[2] != '-') {
        printf("Wrong format. Use mm-yyyy, for example 11-2026.\n");
        return;
    }
    salesSummary(month, 3, month);
}

void bestSellers() {
    char month[20];
    printf("Enter month as mm-yyyy (or 0 for this month): ");
    scanf("%19s", month);

    if (strcmp(month, "0") == 0) {
        time_t now = time(NULL);
        strftime(month, sizeof(month), "%m-%Y", localtime(&now));
    } else if (strlen(month) != 7 || month[2] != '-') {
        printf("Wrong format. Use mm-yyyy, for example 11-2026.\n");
        return;
    }

    FILE *fp = fopen(SALES_ITEMS_FILE, "r");
    if (fp == NULL) {
        printf("No item sales recorded yet.\n");
        return;
    }

    char names[MAX_ITEMS][50];     /* parallel arrays: position i is one item */
    int qtys[MAX_ITEMS];
    float revenue[MAX_ITEMS];
    int count = 0;

    char line[200], date[20], name[50];
    int token, qty;
    float amount;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "%d|%19[^|]|%49[^|]|%d|%f",
                   &token, date, name, &qty, &amount) != 5) {
            continue;
        }
        if (strncmp(date + 3, month, strlen(month)) != 0) {
            continue;                          /* sold in a different month */
        }
        int pos = -1;                          /* have we seen this item before? */
        for (int i = 0; i < count; i++) {
            if (strcmp(names[i], name) == 0) {
                pos = i;
            }
        }
        if (pos == -1) {
            if (count >= MAX_ITEMS) continue;
            pos = count++;
            strcpy(names[pos], name);
            qtys[pos] = 0;
            revenue[pos] = 0;
        }
        qtys[pos] += qty;
        revenue[pos] += amount;
    }
    fclose(fp);

    if (count == 0) {
        printf("No item sales found for %s.\n", month);
        return;
    }

    /* bubble sort: highest quantity first (swap all three arrays together) */
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1 - i; j++) {
            if (qtys[j] < qtys[j + 1]) {
                int tq = qtys[j];       qtys[j] = qtys[j + 1];       qtys[j + 1] = tq;
                float tr = revenue[j];  revenue[j] = revenue[j + 1]; revenue[j + 1] = tr;
                char tn[50];
                strcpy(tn, names[j]);
                strcpy(names[j], names[j + 1]);
                strcpy(names[j + 1], tn);
            }
        }
    }

    printf("\n===== BEST-SELLING ITEMS: %s =====\n", month);
    printf("%-5s %-20s %8s %12s\n", "Rank", "Item", "Sold", "Revenue");
    printf("-------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %8d %12.2f\n", i + 1, names[i], qtys[i], revenue[i]);
    }
    printf("\nBest item of the month: %s (%d sold)\n", names[0], qtys[0]);
}

void totalRevenue() {
    FILE *fp = fopen(BILLS_FILE, "r");
    if (fp == NULL) {
        printf("No sales recorded yet.\n");
        return;
    }
    char line[200], date[20], mode[20];
    int token, bills = 0;
    float subtotal, discount, total, sumTotal = 0, sumDiscount = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "%d|%19[^|]|%f|%f|%f|%19s",
                   &token, date, &subtotal, &discount, &total, mode) == 6) {
            bills++;
            sumTotal += total;
            sumDiscount += discount;
        }
    }
    fclose(fp);

    printf("\n===== TOTAL REVENUE (ALL TIME) =====\n");
    printf("Bills generated  : %d\n", bills);
    printf("Total revenue    : Rs. %.2f\n", sumTotal);
    printf("Discount given   : Rs. %.2f\n", sumDiscount);
    if (bills > 0) {
        printf("Average bill     : Rs. %.2f\n", sumTotal / bills);
    }
}

void reportsMenu() {
    int choice;
    do {
        printf("\n===== SALES REPORTS =====\n");
        printf("1. Daily sales\n");
        printf("2. Monthly sales\n");
        printf("3. Best-selling items (by month)\n");
        printf("4. Total revenue\n");
        printf("0. Back\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: dailyReport(); break;
            case 2: monthlyReport(); break;
            case 3: bestSellers(); break;
            case 4: totalRevenue(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* ---------- ADMIN LOGIN & MENU ---------- */

int adminLogin() {
    char user[30], pass[30];
    printf("\n--- Admin Login ---\n");
    printf("Username: ");
    scanf("%29s", user);
    printf("Password: ");
    scanf("%29s", pass);

    if (strcmp(user, ADMIN_USER) == 0 && strcmp(pass, ADMIN_PASS) == 0) {
        printf("Login successful!\n");
        return 1;
    }
    printf("Wrong username or password.\n");
    return 0;
}

void adminMenu() {
    int choice;
    do {
        printf("\n===== ADMIN MENU =====\n");
        printf("1. View menu\n");
        printf("2. Add item\n");
        printf("3. Manage stock (increase / decrease)\n");
        printf("4. Edit item\n");
        printf("5. Delete item\n");
        printf("6. Sales reports\n");
        printf("0. Logout\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: viewMenu(1); break;
            case 2: addItem(); break;
            case 3: manageStock(); break;
            case 4: editItem(); break;
            case 5: deleteItem(); break;
            case 6: reportsMenu(); break;
            case 0: printf("Logged out.\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* ---------- MAIN ---------- */

int main() {
    int choice;
    loadMenu();

    do {
        printf("\n===== SMART CANTEEN BILLING SYSTEM =====\n");
        printf("1. New order / billing\n");
        printf("2. View menu\n");
        printf("3. Admin login\n");
        printf("0. Exit\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: takeOrder(); break;
            case 2: viewMenu(0); break;
            case 3:
                if (adminLogin()) adminMenu();
                break;
            case 0: printf("Goodbye!\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);

    saveMenu();
    return 0;
}