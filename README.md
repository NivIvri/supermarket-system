Supermarket Management System (C)

A complete supermarket management system written in C, featuring product management, customer & club member handling, shopping carts, file storage, sorting/searching, and optional compressed binary format.

**Features**

Products: name, barcode, price, stock, expiration date

Customers + optional Club Members (with automatic discounts)

Shopping Cart implemented as a sorted linked list

Sorting products by name / quantity / price (qsort)

Binary search (bsearch) when sorted

Checkout and cancel purchase flow

Full input validation and memory cleanup

**File Storage**

Customers → Customers.txt (text file)

Products → SuperMarket.bin (binary file)

Compressed mode supported via bitwise packing

SuperMarket.exe <isCompressed> <filename>

**Compression**

Efficient bit-packing for:

product count

name length

barcode type + digits

quantity

price (agorot + whole)

expiration date

Example:

SuperMarket.exe 1 super_compress.bin

**Macros**

Located in myMacros.h:

Pointer validation

Error printing

Safe cleanup

Optional detailed cart printing (DETAIL_PRINT)

**Variadic Message**

Used on exit:

printMessage("Thank", "You", "For", "Shopping", "With", "Us", NULL);

Build & Run

Open the .sln in Visual Studio

Build → Rebuild

Run normally or with compressed mode parameters

Technologies

C programming, dynamic memory, linked lists, function pointers, qsort/bsearch, binary & text I/O, bitwise operations, macros, variadic functions.
