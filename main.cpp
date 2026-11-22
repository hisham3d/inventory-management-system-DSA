// Inventory and Stock Management System
// Data Structures and Algorithms - Open-Ended Lab
// Data structures : singly linked list (products), singly linked list (transactions), array of pointers
// Algorithms      : linear search, binary search, bubble sort, file handling
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <cctype>
using namespace std;

const int MAX_ID = 999999;
const int MAX_QTY = 1000000;
const int MAX_PRICE = 1000000;
const int MAX_PRODUCTS = 500;       // size of the array used for binary search

// ------------------------------------------------------------ structures (list nodes)
struct Product {
    int id;
    string name;
    int price;
    int qty;
    int reorder;        // low-stock level
    Product* next;
};

struct Transaction {
    int tid;
    int pid;
    string pname;
    char type;          // 'I' = stock-in, 'O' = stock-out
    int qty;
    int balance;        // stock left after this transaction
    Transaction* next;
};

// ------------------------------------------------------------ input helpers
string toLower(string s) {
    int n = s.length();
    for (int i = 0; i < n; i++)
        s[i] = tolower(s[i]);
    return s;
}

string readLine(string prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) {
        cout << "\nInput ended. Exiting.\n";
        exit(0);
    }
    return s;
}

bool isNumber(string s) {
    int n = s.length();
    if (n == 0 || n > 9) return false;
    int start = 0;
    if (s[0] == '-') start = 1;
    if (start == n) return false;
    for (int i = start; i < n; i++)
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}

int readInt(string prompt, int low, int high) {
    while (true) {
        string s = readLine(prompt);
        if (!isNumber(s)) {
            cout << "  Invalid input: enter a whole number.\n";
            continue;
        }
        int v = atoi(s.c_str());
        if (v < low || v > high) {
            cout << "  Out of range: allowed " << low << " to " << high << ".\n";
            continue;
        }
        return v;
    }
}

bool isBlank(string s) {
    int n = s.length();
    for (int i = 0; i < n; i++)
        if (s[i] != ' ') return false;
    return true;
}

string readName(string prompt) {
    while (true) {
        string s = readLine(prompt);
        if (isBlank(s)) { cout << "  Name cannot be empty.\n"; continue; }
        if (s.length() > 30) { cout << "  Name too long (max 30 characters).\n"; continue; }
        return s;
    }
}

// ------------------------------------------------------------ display helpers
string statusOf(Product* p) {
    if (p->qty == 0) return "OUT OF STOCK";
    if (p->qty <= p->reorder) return "LOW";
    return "OK";
}

void printHeader() {
    cout << left << setw(8) << "ID" << setw(32) << "Name" << right << setw(10) << "Price"
         << setw(10) << "Qty" << setw(10) << "Reorder" << "  Status\n";
    cout << "--------------------------------------------------------------------------------\n";
}

void printRow(Product* p) {
    cout << left << setw(8) << p->id << setw(32) << p->name << right << setw(10) << p->price
         << setw(10) << p->qty << setw(10) << p->reorder << "  " << statusOf(p) << "\n";
}

// sorting helpers: key 1 = ID, 2 = name, 3 = quantity
bool outOfOrder(Product* a, Product* b, int key) {
    if (key == 1) return a->id > b->id;
    if (key == 2) return toLower(a->name) > toLower(b->name);
    return a->qty > b->qty;
}

void swapData(Product* a, Product* b) {
    int tid = a->id;       a->id = b->id;           b->id = tid;
    string tn = a->name;   a->name = b->name;       b->name = tn;
    int tp = a->price;     a->price = b->price;     b->price = tp;
    int tq = a->qty;       a->qty = b->qty;         b->qty = tq;
    int tr = a->reorder;   a->reorder = b->reorder; b->reorder = tr;
}

// ============================================================ class ProductList
class ProductList {
private:
    Product* head;
    int count;

public:
    ProductList() {
        head = NULL;
        count = 0;
    }

    ~ProductList() {
        while (head != NULL) {
            Product* t = head;
            head = head->next;
            delete t;
        }
    }

    int getCount() { return count; }

    // linear search by ID
    Product* findById(int id) {
        Product* p = head;
        while (p != NULL) {
            if (p->id == id) return p;
            p = p->next;
        }
        return NULL;
    }

    // add a node at the end; fails for a duplicate ID or a full inventory
    bool insert(int id, string name, int price, int qty, int reorder) {
        if (count >= MAX_PRODUCTS) return false;
        if (findById(id) != NULL) return false;
        Product* n = new Product;
        n->id = id;
        n->name = name;
        n->price = price;
        n->qty = qty;
        n->reorder = reorder;
        n->next = NULL;
        if (head == NULL) {
            head = n;
        } else {
            Product* p = head;
            while (p->next != NULL) p = p->next;
            p->next = n;
        }
        count++;
        return true;
    }

    bool remove(int id) {
        Product* prev = NULL;
        Product* p = head;
        while (p != NULL) {
            if (p->id == id) {
                if (prev == NULL) head = p->next;
                else prev->next = p->next;
                delete p;
                count--;
                return true;
            }
            prev = p;
            p = p->next;
        }
        return false;
    }

    // bubble sort: key 1 = ID, 2 = name, 3 = quantity
    void bubbleSort(int key) {
        bool swapped = true;
        while (swapped) {
            swapped = false;
            for (Product* p = head; p != NULL && p->next != NULL; p = p->next) {
                if (outOfOrder(p, p->next, key)) {
                    swapData(p, p->next);
                    swapped = true;
                }
            }
        }
    }

    // binary search by ID. The list is sorted by ID first and its nodes are
    // copied into an array, because binary search needs index access.
    Product* binarySearch(int id, int& steps) {
        steps = 0;
        if (count == 0) return NULL;
        bubbleSort(1);

        Product* arr[MAX_PRODUCTS];
        int n = 0;
        for (Product* p = head; p != NULL; p = p->next) {
            arr[n] = p;
            n++;
        }

        int low = 0;
        int high = n - 1;
        while (low <= high) {
            steps++;
            int mid = (low + high) / 2;
            if (arr[mid]->id == id) return arr[mid];
            else if (arr[mid]->id < id) low = mid + 1;
            else high = mid - 1;
        }
        return NULL;
    }

    // linear search by name (full or part of the name)
    void searchByName(string text) {
        text = toLower(text);
        int found = 0;
        for (Product* p = head; p != NULL; p = p->next) {
            if (toLower(p->name).find(text) != string::npos) {
                if (found == 0) printHeader();
                printRow(p);
                found++;
            }
        }
        if (found == 0) cout << "No product name matches \"" << text << "\".\n";
        else cout << found << " match(es) found.\n";
    }

    void displayAll() {
        if (count == 0) { cout << "No products in inventory.\n"; return; }
        printHeader();
        for (Product* p = head; p != NULL; p = p->next) printRow(p);
        cout << "Total products: " << count << "\n";
    }

    void inventoryReport() {
        cout << "\n=========== INVENTORY REPORT ===========\n";
        if (count == 0) { cout << "No products in inventory.\n"; return; }
        displayAll();
        int units = 0;
        double total = 0;
        for (Product* p = head; p != NULL; p = p->next) {
            units = units + p->qty;
            total = total + (double)p->price * p->qty;
        }
        cout << "Total units in stock: " << units << "\n";
        cout << "Total inventory value: " << fixed << setprecision(0) << total << "\n";
    }

    void lowStockReport() {
        cout << "\n=========== LOW-STOCK REPORT ===========\n";
        int n = 0;
        for (Product* p = head; p != NULL; p = p->next) {
            if (p->qty <= p->reorder) {
                if (n == 0) printHeader();
                printRow(p);
                n++;
            }
        }
        if (n == 0) cout << "No products are at or below their reorder level.\n";
        else cout << n << " product(s) need restocking.\n";
    }

    void saveToFile() {
        ofstream pf("products.txt");
        for (Product* p = head; p != NULL; p = p->next)
            pf << p->id << " " << p->price << " " << p->qty << " " << p->reorder << " " << p->name << "\n";
        pf.close();
    }

    void loadFromFile() {
        ifstream pf("products.txt");
        int id, price, qty, reorder;
        string name;
        while (pf >> id >> price >> qty >> reorder) {
            pf.ignore();                // skip the space before the name
            getline(pf, name);
            insert(id, name, price, qty, reorder);
        }
        pf.close();
    }
};

// ============================================================ class TransactionList
class TransactionList {
private:
    Transaction* head;
    Transaction* tail;
    int count;
    int nextTid;

public:
    TransactionList() {
        head = NULL;
        tail = NULL;
        count = 0;
        nextTid = 1;
    }

    ~TransactionList() {
        while (head != NULL) {
            Transaction* t = head;
            head = head->next;
            delete t;
        }
    }

    int getCount() { return count; }

    void add(int tid, int pid, string pname, char type, int qty, int balance) {
        Transaction* n = new Transaction;
        n->tid = tid;
        n->pid = pid;
        n->pname = pname;
        n->type = type;
        n->qty = qty;
        n->balance = balance;
        n->next = NULL;
        if (head == NULL) {
            head = n;
            tail = n;
        } else {
            tail->next = n;
            tail = n;
        }
        count++;
        if (tid >= nextTid) nextTid = tid + 1;
    }

    // record a stock-in or stock-out for product p (p->qty is already updated)
    void record(Product* p, char type, int qty) {
        add(nextTid, p->id, p->name, type, qty, p->qty);
    }

    void display() {
        cout << "\n=========== TRANSACTION HISTORY ===========\n";
        if (count == 0) { cout << "No transactions recorded.\n"; return; }
        cout << left << setw(6) << "TID" << setw(8) << "PID" << setw(30) << "Product" << setw(6) << "Type"
             << right << setw(8) << "Qty" << setw(10) << "Balance" << "\n";
        cout << "--------------------------------------------------------------------\n";
        for (Transaction* t = head; t != NULL; t = t->next) {
            string type = "OUT";
            if (t->type == 'I') type = "IN";
            cout << left << setw(6) << t->tid << setw(8) << t->pid << setw(30) << t->pname
                 << setw(6) << type << right << setw(8) << t->qty << setw(10) << t->balance << "\n";
        }
        cout << "Total transactions: " << count << "\n";
    }

    void saveToFile() {
        ofstream tf("transactions.txt");
        for (Transaction* t = head; t != NULL; t = t->next)
            tf << t->tid << " " << t->pid << " " << t->type << " " << t->qty << " "
               << t->balance << " " << t->pname << "\n";
        tf.close();
    }

    void loadFromFile() {
        ifstream tf("transactions.txt");
        int tid, pid, qty, balance;
        char type;
        string name;
        while (tf >> tid >> pid >> type >> qty >> balance) {
            tf.ignore();
            getline(tf, name);
            add(tid, pid, name, type, qty, balance);
        }
        tf.close();
    }
};

// ------------------------------------------------------------ menu functions
void saveAll(ProductList& pl, TransactionList& tl) {
    pl.saveToFile();
    tl.saveToFile();
}

void addProduct(ProductList& pl, TransactionList& tl) {
    cout << "\n--- Add Product ---\n";
    if (pl.getCount() >= MAX_PRODUCTS) {
        cout << "Error: inventory is full (maximum " << MAX_PRODUCTS << " products).\n";
        return;
    }
    int id = readInt("Product ID (1-999999): ", 1, MAX_ID);
    if (pl.findById(id) != NULL) {
        cout << "Error: Product ID " << id << " already exists. Product not added.\n";
        return;
    }
    string name = readName("Name: ");
    int price = readInt("Unit price: ", 0, MAX_PRICE);
    int qty = readInt("Opening quantity: ", 0, MAX_QTY);
    int reorder = readInt("Reorder (low-stock) level: ", 0, MAX_QTY);
    pl.insert(id, name, price, qty, reorder);
    if (qty > 0) tl.record(pl.findById(id), 'I', qty);
    saveAll(pl, tl);
    cout << "Product added successfully.\n";
}

void updateProduct(ProductList& pl, TransactionList& tl) {
    cout << "\n--- Update Product ---\n";
    if (pl.getCount() == 0) { cout << "No products to update.\n"; return; }
    int id = readInt("Product ID to update: ", 1, MAX_ID);
    Product* p = pl.findById(id);
    if (p == NULL) { cout << "Error: Product ID " << id << " not found.\n"; return; }
    cout << "Current record:\n";
    printHeader();
    printRow(p);
    cout << "1. Name\n2. Unit price\n3. Reorder level\n";
    int c = readInt("Field to update (1-3): ", 1, 3);
    if (c == 1) p->name = readName("New name: ");
    else if (c == 2) p->price = readInt("New unit price: ", 0, MAX_PRICE);
    else p->reorder = readInt("New reorder level: ", 0, MAX_QTY);
    saveAll(pl, tl);
    cout << "Product updated successfully.\n";
}

void deleteProduct(ProductList& pl, TransactionList& tl) {
    cout << "\n--- Delete Product ---\n";
    if (pl.getCount() == 0) { cout << "No products to delete.\n"; return; }
    int id = readInt("Product ID to delete: ", 1, MAX_ID);
    Product* p = pl.findById(id);
    if (p == NULL) { cout << "Error: Product ID " << id << " not found.\n"; return; }
    printHeader();
    printRow(p);
    string answer = readLine("Confirm delete (y/n): ");
    if (answer != "y" && answer != "Y") { cout << "Deletion cancelled.\n"; return; }
    pl.remove(id);
    saveAll(pl, tl);
    cout << "Product deleted successfully.\n";
}

void searchProduct(ProductList& pl) {
    cout << "\n--- Search Product ---\n";
    if (pl.getCount() == 0) { cout << "No products in inventory.\n"; return; }
    cout << "1. By ID (linear search)\n2. By ID (binary search)\n3. By name (full or part of name)\n";
    int c = readInt("Choice (1-3): ", 1, 3);
    if (c == 1) {
        int id = readInt("Product ID: ", 1, MAX_ID);
        Product* p = pl.findById(id);
        if (p == NULL) {
            cout << "Product ID " << id << " not found.\n";
        } else {
            printHeader();
            printRow(p);
        }
    } else if (c == 2) {
        int id = readInt("Product ID: ", 1, MAX_ID);
        int steps;
        Product* p = pl.binarySearch(id, steps);
        if (p == NULL) {
            cout << "Product ID " << id << " not found (" << steps << " comparison step(s)).\n";
        } else {
            printHeader();
            printRow(p);
            cout << "Found in " << steps << " comparison step(s).\n";
        }
    } else {
        string text = readLine("Name (or part of name): ");
        if (isBlank(text)) { cout << "Search text cannot be empty.\n"; return; }
        pl.searchByName(text);
    }
}

void sortProducts(ProductList& pl) {
    cout << "\n--- Sort Inventory ---\n";
    if (pl.getCount() == 0) { cout << "No products in inventory.\n"; return; }
    cout << "1. By product ID\n2. By name\n3. By quantity\n";
    int c = readInt("Sort by (1-3): ", 1, 3);
    pl.bubbleSort(c);
    cout << "Inventory sorted (bubble sort).\n";
    pl.displayAll();
}

void stockIn(ProductList& pl, TransactionList& tl) {
    cout << "\n--- Stock In ---\n";
    if (pl.getCount() == 0) { cout << "No products in inventory.\n"; return; }
    int id = readInt("Product ID: ", 1, MAX_ID);
    Product* p = pl.findById(id);
    if (p == NULL) { cout << "Error: Product ID " << id << " not found.\n"; return; }
    int q = readInt("Quantity to add: ", 1, MAX_QTY);
    if (p->qty + q > MAX_QTY) {
        cout << "Error: stock would exceed the maximum of " << MAX_QTY << ".\n";
        return;
    }
    p->qty = p->qty + q;
    tl.record(p, 'I', q);
    saveAll(pl, tl);
    cout << "Stock-in recorded. " << p->name << " now has " << p->qty << " unit(s).\n";
}

void stockOut(ProductList& pl, TransactionList& tl) {
    cout << "\n--- Stock Out ---\n";
    if (pl.getCount() == 0) { cout << "No products in inventory.\n"; return; }
    int id = readInt("Product ID: ", 1, MAX_ID);
    Product* p = pl.findById(id);
    if (p == NULL) { cout << "Error: Product ID " << id << " not found.\n"; return; }
    if (p->qty == 0) {
        cout << "Error: " << p->name << " is out of stock (quantity 0).\n";
        return;
    }
    int q = readInt("Quantity to remove: ", 1, MAX_QTY);
    if (q > p->qty) {
        cout << "Error: insufficient stock. Available: " << p->qty << ", requested: " << q << ".\n";
        return;
    }
    p->qty = p->qty - q;
    tl.record(p, 'O', q);
    saveAll(pl, tl);
    cout << "Stock-out recorded. " << p->name << " now has " << p->qty << " unit(s).\n";
    if (p->qty == 0)
        cout << "Warning: " << p->name << " is now OUT OF STOCK.\n";
    else if (p->qty <= p->reorder)
        cout << "Warning: " << p->name << " is at or below its reorder level (" << p->reorder << ").\n";
}

void reportsMenu(ProductList& pl, TransactionList& tl) {
    cout << "\n--- Reports ---\n1. Inventory report\n2. Low-stock report\n3. Transaction history\n";
    int c = readInt("Choice (1-3): ", 1, 3);
    if (c == 1) pl.inventoryReport();
    else if (c == 2) pl.lowStockReport();
    else tl.display();
}

void showMenu() {
    cout << "\n======== INVENTORY & STOCK MANAGEMENT SYSTEM ========\n"
         << " 1. Add product\n 2. Update product\n 3. Delete product\n 4. Search product\n"
         << " 5. Sort inventory\n 6. Stock in\n 7. Stock out\n 8. Display all products\n"
         << " 9. Reports\n 0. Save and exit\n";
}

int main() {
    ProductList products;
    TransactionList transactions;

    products.loadFromFile();
    transactions.loadFromFile();
    cout << "Loaded " << products.getCount() << " product(s) and "
         << transactions.getCount() << " transaction(s) from file.\n";

    int choice = -1;
    while (choice != 0) {
        showMenu();
        choice = readInt("Enter choice: ", 0, 9);
        if (choice == 1) addProduct(products, transactions);
        else if (choice == 2) updateProduct(products, transactions);
        else if (choice == 3) deleteProduct(products, transactions);
        else if (choice == 4) searchProduct(products);
        else if (choice == 5) sortProducts(products);
        else if (choice == 6) stockIn(products, transactions);
        else if (choice == 7) stockOut(products, transactions);
        else if (choice == 8) products.displayAll();
        else if (choice == 9) reportsMenu(products, transactions);
    }
    saveAll(products, transactions);
    cout << "Data saved. Goodbye.\n";
    return 0;
}