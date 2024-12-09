// VERIFIED test2.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_CONTACTS 100
#define MAX_NAME_LENGTH 50

typedef struct {
    char name[MAX_NAME_LENGTH];
    char phone[15];
    char email[50];
} Contact;

typedef struct {
    Contact contacts[MAX_CONTACTS];
    int count;
} ContactBook;

void initialize_contact_book(ContactBook *book) {
    book->count = 0;
}

int add_contact(ContactBook *book, const char *name, const char *phone, const char *email) {
    if (book->count >= MAX_CONTACTS) {
        printf("Contact book is full!\n");
        return 0;
    }

    Contact *new_contact = &book->contacts[book->count];
    strncpy(new_contact->name, name, MAX_NAME_LENGTH - 1);
    strncpy(new_contact->phone, phone, 14);
    strncpy(new_contact->email, email, 49);
    book->count++;

    return 1;
}

Contact* find_contact_by_name(ContactBook *book, const char *name) {
    for (int i = 0; i < book->count; i++) {
        if (strcmp(book->contacts[i].name, name) == 0) {
            return &book->contacts[i];
        }
    }
    return NULL;
}

void display_all_contacts(ContactBook *book) {
    if (book->count == 0) {
        printf("No contacts found.\n");
        return;
    }

    for (int i = 0; i < book->count; i++) {
        printf("Contact %d:\n", i + 1);
        printf("Name: %s\n", book->contacts[i].name);
        printf("Phone: %s\n", book->contacts[i].phone);
        printf("Email: %s\n\n", book->contacts[i].email);
    }
}

int main() {
    ContactBook book;
    initialize_contact_book(&book);

    int count;
    printf("Enter the number of contacts to add: ");
    scanf("%d", &count);

    for(int i = 0; i< count; i++){
        char name[MAX_NAME_LENGTH];
        char phone[15];
        char email[50];
        printf("Enter name, phone, and email for contact %d: ", i + 1);
        scanf("%s %s %s", name, phone, email);
        add_contact(&book, name, phone, email);
    }

    book.count = count;

    // Display all contacts
    display_all_contacts(&book);

    // Find a contact
    char search_name[MAX_NAME_LENGTH];
    printf("Enter name to search: ");
    scanf("%49s", search_name);
    search_name[strcspn(search_name, "\n")] = 0;  // Remove newline

    Contact *found = find_contact_by_name(&book, search_name);
    if (found) {
        printf("Contact found:\n");
        printf("Name: %s\n", found->name);
        printf("Phone: %s\n", found->phone);
        printf("Email: %s\n", found->email);
    } else {
        printf("Contact not found.\n");
    }

    return 0;
}