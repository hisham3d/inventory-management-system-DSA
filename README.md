# Inventory and Stock Management System

Data Structures and Algorithms - Open-Ended Lab, Military College of Signals, NUST.

A console application in C++ for managing product records, stock levels and stock transactions.

## Features
- Product records in a singly linked list inside a `ProductList` class (add, update, delete, search)
- Linear search by ID and by name (full or partial), binary search by ID
- Bubble sort by ID, name or quantity
- Stock-in and stock-out with a transaction history (`TransactionList` class)
- Inventory, low-stock and transaction reports
- Data saved to `products.txt` and `transactions.txt` and reloaded on start-up
- Input validation for duplicate IDs, zero stock, insufficient stock and invalid entries
- Maximum of 500 products

## Build and run
    g++ -o inventory main.cpp
    ./inventory

In Code::Blocks or Visual Studio, create a console project and add `main.cpp`. Keep the data files in the folder the program runs from.

## Files
- `main.cpp` - complete source code
- `products.txt`, `transactions.txt` - sample data (delete them to start with an empty inventory)

## Authors
Hisham, [second member name]
