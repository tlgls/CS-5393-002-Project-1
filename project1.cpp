#include <iostream>
#include <unordered_map>
#include <queue>
#include <stack>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm> // For std::remove
using namespace std;

// Book structure
struct Book {
    string ISBN;
    string Title;
    string Author;
    double Price;
    int Quantity; // Number of available copies
    int borrowedCount = 0; // Count of borrowed copies
    queue<string> reservationQueue; // Queue for reservations

    // Default constructor
    Book() : ISBN(""), Title(""), Author(""), Price(0.0), Quantity(0) {}

    // Parameterized constructor
    Book(const string& isbn, const string& title, const string& author, double price, int quantity)
        : ISBN(isbn), Title(title), Author(author), Price(price), Quantity(quantity) {}
};


// Node structure for Binary Search Tree (BST)
struct BSTNode {
    Book book;
    BSTNode* left;
    BSTNode* right;
    BSTNode(Book b) : book(b), left(nullptr), right(nullptr) {}
};

// Library Management System class
class LibrarySystem {
private:
    unordered_map<string, Book> books_by_isbn;  // Hash table for books by ISBN
    BSTNode* bst_root;                          // Root of the BST
    queue<string> reservation_queue;            // Queue for book reservations
    stack<Book> recently_borrowed;              // Stack for recently borrowed books

    // Helper function to insert a book into the BST by Title
    BSTNode* insert_bst(BSTNode* root, Book book) {
        if (root == nullptr) return new BSTNode(book);
        if (book.Title < root->book.Title)
            root->left = insert_bst(root->left, book);
        else
            root->right = insert_bst(root->right, book);
        return root;
    }

    // Helper function to search for a book by Title in the BST
    BSTNode* search_bst(BSTNode* root, const string& title) {
        if (root == nullptr || root->book.Title == title)
            return root;
        if (title < root->book.Title)
            return search_bst(root->left, title);
        else
            return search_bst(root->right, title);
    }

    // Helper function to retrieve the book by title
    Book* getBookByTitle(const string& title) {
        string lowerTitle = toLowerCase(trim(title)); // Trim and convert to lowercase
        for (auto& entry : books_by_isbn) {
            if (toLowerCase(trim(entry.second.Title)) == lowerTitle) {
                return &entry.second;
            }
        }
        return nullptr;
    }


    // Helper function to trim whitespace from a string
    string trim(const string& str) {
        size_t first = str.find_first_not_of(' ');
        if (string::npos == first) {
            return str; // no content
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

    string toLowerCase(const string& str) { // helper function to make title lowercase to ensure titles can be returned
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }


public:
    LibrarySystem() : bst_root(nullptr) {}

    void addBook(const Book& book) {
        books_by_isbn[book.ISBN] = book;
        bst_root = insert_bst(bst_root, book);
        
        // Only print the newly added book
        cout << "Added book: " << book.Title << " (ISBN: " << book.ISBN << ", Quantity: " << book.Quantity << ")" << endl;
    }

    // Load books from a CSV file
    void loadBooksFromCSV(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error opening file: " << filename << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue; // Skip empty lines

            stringstream ss(line);
            string isbn, title, author, price_str, quantity_str;

            getline(ss, isbn, ',');
            getline(ss, title, ',');
            getline(ss, author, ',');
            getline(ss, price_str, ',');
            getline(ss, quantity_str, ',');

            isbn = trim(isbn);
            title = trim(title);
            author = trim(author);
            price_str = trim(price_str);
            quantity_str = trim(quantity_str);

            if (price_str.empty() || quantity_str.empty()) {
                cout << "Error: Missing price or quantity in the CSV row: " << line << endl;
                continue; // Skip this row
            }

            try {
                double price = stod(price_str);
                int quantity = stoi(quantity_str);

                // Use the constructor to create a Book object
                Book book(isbn, title, author, price, quantity);
                addBook(book);
            } catch (const invalid_argument& e) {
                cout << "Error converting price or quantity: " << e.what() << " in row: " << line << endl;
            } catch (const out_of_range& e) {
                cout << "Error: Value out of range: " << e.what() << " in row: " << line << endl;
            }
        }

        file.close();
        cout << "Books loaded from " << filename << endl;
    }

    void searchByTitle(string title) {
        cout << "Searching for title: '" << title << "'" << endl; // Debug output
        BSTNode* result = search_bst(bst_root, title);
        if (result) {
            cout << "Book found by Title: '" << result->book.Title << "' by " << result->book.Author << " (Quantity: " << result->book.Quantity << ")" << endl;
        } else {
            cout << "Book not found by Title." << endl;
        }
    }

    // Reserve a book (enqueue the title)
    void reserveBook(const string& title) {
        reservation_queue.push(title); // Add the title to the reservation queue
        cout << "Reserved: '" << title << "'." << endl;
    }


    // Display reservation queue
    void displayReservations() {
        cout << "Current reservations: ";
        queue<string> temp_queue = reservation_queue; // Copy to preserve the queue
        while (!temp_queue.empty()) {
            cout << temp_queue.front() << " ";
            temp_queue.pop();
        }
        cout << endl;
    }

    // Lend a book (push onto the stack) and return the title of the lent book
    string lendBook() {
        if (!reservation_queue.empty()) {
            string title = reservation_queue.front();
            reservation_queue.pop(); // Dequeue the book title from the reservation queue

            Book* book = getBookByTitle(title); // Get the book by title
            if (book) {
                if (book->Quantity > 0) {
                    recently_borrowed.push(*book); // Push the book onto the stack of borrowed books
                    books_by_isbn[book->ISBN].Quantity--; // Decrease available quantity in the map
                    book->borrowedCount++; // Increase borrowed count
                    return "Lent: " + book->Title + " (Remaining copies: " + to_string(book->Quantity) + ")";
                } else {
                    // Re-enqueue the book because it's unavailable
                    reservation_queue.push(title);
                    return "No copies of '" + title + "' available. It will be lent when the next copy is returned.";
                }
            } else {
                return "Book not found: '" + title + "'";
            }
        } else {
            return "No books to lend.";
        }
    }

    // Return the last borrowed book and return the title of the returned book
    string returnBook() {
        if (!recently_borrowed.empty()) {
            Book lastBorrowedBook = recently_borrowed.top();
            recently_borrowed.pop();

            Book* book = getBookByTitle(lastBorrowedBook.Title);
            if (book != nullptr) {
                book->borrowedCount--; // Decrease borrowed count
                book->Quantity++; // Increase available copies
                return "Returned: " + book->Title + " (Available copies: " + to_string(book->Quantity) + ")";
            } else {
                return "Error: Book not found for returning: " + lastBorrowedBook.Title;
            }
        } else {
            return "No books to return.";
        }
    }


    void displayBooks() {
        cout << "Current book inventory:" << endl;
        for (const auto& pair : books_by_isbn) {
            cout << "ISBN: " << pair.second.ISBN 
                 << ", Title: " << pair.second.Title 
                 << ", Author: " << pair.second.Author 
                 << ", Price: " << pair.second.Price 
                 << ", Quantity: " << pair.second.Quantity << endl;
        }
    }
};

int main() {
    LibrarySystem library;

    // Load books from CSV file
    library.loadBooksFromCSV("/Users/tiffany/Desktop/CS 5393-002/CS 5393-002/Book Dataset.csv");

    // Reserve multiple copies of the same book
    library.reserveBook("1984");
    library.reserveBook("1984"); // Reserve another copy of "1984"
    library.reserveBook("To Kill a Mockingbird");
    library.reserveBook("The Catcher in the Rye");
    library.reserveBook("Pride and Prejudice");
    library.reserveBook("Pearl and Sir Orfeo");
    library.reserveBook("CHESS FOR YOUNG BEGINNERS");
    library.reserveBook("Which Colour?");
    library.reserveBook("ARE YOU MY MOTHER MINI PB (EXPORT)");
    library.reserveBook("The Great Gatsby");

    cout << endl; // for formatting

    // Display current reservations
    library.displayReservations(); // First set of reservations

    cout << endl; // for formatting

    // Lend books and print titles
    for (int i = 0; i < 3; ++i) {
        string lentBookTitle = library.lendBook(); // Lend a book from the reservation queue
        cout << lentBookTitle << endl; // Print the title of the lent book
    }

    cout << endl;

    library.displayReservations(); // Show remaining reservations

    // Return the last book and print the title of the returned book
    string returnedBookTitle = library.returnBook(); // Returns the last borrowed book
    if (!returnedBookTitle.empty()) {
        cout << returnedBookTitle << endl; // Print the returned book title
    }

    // Lend another book from the queue and print the title
    string nextLentBookTitle = library.lendBook(); // Lend the next book from the queue
    if (!nextLentBookTitle.empty()) {
        cout << nextLentBookTitle << endl; // Print the next lent book title
    }

    cout << endl;

    library.displayReservations(); // Show remaining reservations after lending

    return 0;
}

