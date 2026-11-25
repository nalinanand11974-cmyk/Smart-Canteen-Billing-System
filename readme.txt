# Smart Canteen Billing System

A terminal-based point-of-sale (POS) and inventory management application written in C. It handles customer order billing, real-time stock tracking, multiple payment modes (including an ASCII/Unicode terminal UPI QR code), and persistent sales reporting.

---

## Features

### Customer & Cashier Billing
* *Interactive Menu Display*: Displays item IDs, names, categories, prices, and automatically flags out-of-stock items as Unavailable.
* *Cart Management*: Add multiple items to a cart (up to 20 unique items per order) with automatic stock validation so orders cannot exceed available inventory.
* *Automated Discounts: Automatically applies a **10% discount* on orders with a subtotal above *Rs. 200.00*.
* *Token Generation*: Generates sequential, persistent customer order tokens starting from 1001.
* *Multiple Payment Modes*:
  * *Cash*: Validates tendered amount and calculates exact change to return.
  * *Card*: Logs card payment confirmation.
  * *UPI*: Renders a scannable terminal QR code using UTF-8 half-block characters (▀) and ANSI color codes, complete with UPI ID and payable amount.

### Admin Dashboard
* *Menu Management*: Add new food items (with auto-incrementing IDs that are never reused after deletion), edit existing item details (name, category, price), or delete items.
* *Inventory Control*: Increase or decrease stock quantities for individual items.
* *Sales Analytics & Reports*:
  * *Daily Sales*: Filter revenue, bill count, discounts given, and payment-mode breakdown (Cash/Card/UPI) by date (dd-mm-yyyy).
  * *Monthly Sales*: View aggregated monthly performance (mm-yyyy).
  * *Best-Selling Items*: Ranks items sold in a given month by quantity sold and revenue generated.
  * *All-Time Revenue*: Displays cumulative bills, total revenue, total discounts, and average bill value.

---

## Default Credentials & Configuration

| Setting | Default Value |
| :--- | :--- |
| *Admin Username* | admin |
| *Admin Password* | admin123 |
| *Discount Threshold* | Rs. 200.00 |
| *Discount Rate* | 10% |
| *Starting Token* | 1001 |

(You can modify these constants at the top of the source file before compiling.)

---

## Data Persistence

The application automatically creates and manages four local files in the working directory to persist state across runs:

* menu.dat — Binary file storing the menu catalog and current stock levels (initialized with 9 default items on first run).
* token.dat — Text file tracking the next available order token number.
* bills.txt — Pipe-delimited (|) log of all completed bills (token|datetime|subtotal|discount|total|payment_mode).
* salesitems.txt — Pipe-delimited (|) item-level sales log used to calculate monthly best-sellers (token|datetime|item_name|qty|amount).

---

## Getting Started

### Prerequisites
* A C compiler such as gcc or clang.
* A terminal supporting UTF-8 and ANSI escape codes (for proper UPI QR code rendering; Windows console VT processing is automatically enabled via #ifdef _WIN32).

### Compilation & Execution

bash
# Compile the source code
gcc canteen.c -o canteen

# Run on Linux / macOS
./canteen

# Run on Windows
canteen.exe


---

## Project Structure

text
├── canteen.c          # Main application source code
├── menu.dat           # Auto-generated binary menu & stock database
├── token.dat          # Auto-generated token counter
├── bills.txt          # Auto-generated transaction history
└── salesitems.txt     # Auto-generated itemized sales log