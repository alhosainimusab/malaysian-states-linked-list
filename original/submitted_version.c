#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME 50


#define RESET       "\033[0m"
#define RED         "\033[1;31m"
#define GREEN       "\033[1;32m"
#define YELLOW      "\033[1;33m"
#define BLUE        "\033[1;34m"
#define CYAN        "\033[1;36m"

typedef struct Node {
    char name[MAX_NAME];
    struct Node* next;
} Node;

void toLower(char* dest, const char* src);
void createList(Node** head);
void displayList(Node* head);
void insertState(Node** head, char* name);
void deleteState(Node** head, char* name);
void searchState(Node* head, char* name);
void sortList(Node** head, int ascending);
Node* createNode(char* name);
void menu();
int getStateOrder(const char* name);

char* states[] = {
    "Johore", "Kelantan", "Sabah", "Perak", "Kedah", "Malacca", "Putrajaya",
    "Pahang", "Perlis", "Labuan", "Negeri Sembilan", "Sarawak", "Pulau Pinang",
    "Selangor", "Terengganu", "Kuala Lumpur"
};

int main() {
    Node* head = NULL;
    int choice;
    char name[MAX_NAME];

    createList(&head);

    while (1) {
        menu();
        printf(YELLOW "\nEnter your choice: " RESET);
        scanf("%d", &choice);
        getchar(); // Consume newline

        switch (choice) {
            case 1:
                sortList(&head, 1);
                printf(CYAN "\n--- States in Ascending Order ---\n" RESET);
                displayList(head);
                break;
            case 2:
                sortList(&head, 0);
                printf(CYAN "\n--- States in Descending Order ---\n" RESET);
                displayList(head);
                break;
            case 3:
                printf(YELLOW "Enter state to insert: " RESET);
                fgets(name, MAX_NAME, stdin);
                name[strcspn(name, "\n")] = 0;
                insertState(&head, name);
                break;
            case 4:
                printf(YELLOW "Enter state to delete: " RESET);
                fgets(name, MAX_NAME, stdin);
                name[strcspn(name, "\n")] = 0;
                deleteState(&head, name);
                break;
            case 5:
                printf(YELLOW "Enter state to search: " RESET);
                fgets(name, MAX_NAME, stdin);
                name[strcspn(name, "\n")] = 0;
                searchState(head, name);
                break;
            case 6:
                printf(GREEN "Exiting program. Goodbye!\n" RESET);
                exit(0);
            default:
                printf(RED "Invalid choice. Please try again.\n" RESET);
        }
    }

    return 0;
}

void menu() {
    printf(CYAN "\n========== Malaysian States Linked List Menu ==========\n" RESET);
    printf("1. Display Ascending Order\n");
    printf("2. Display Descending Order\n");
    printf("3. Insert State\n");
    printf("4. Delete State\n");
    printf("5. Search State\n");
    printf("6. Exit\n");
}

void createList(Node** head) {
    for (int i = 0; i < 16; i++) {
        insertState(head, states[i]);
    }
}

Node* createNode(char* name) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf(RED "Memory allocation failed.\n" RESET);
        exit(1);
    }
    strncpy(newNode->name, name, MAX_NAME);
    newNode->name[MAX_NAME - 1] = '\0';
    newNode->next = NULL;
    return newNode;
}

void insertState(Node** head, char* name) {
    Node* newNode = createNode(name);
    if (!*head) {
        *head = newNode;
    } else {
        Node* temp = *head;
        while (temp->next)
            temp = temp->next;
        temp->next = newNode;
    }
    printf(GREEN "State inserted: %s\n" RESET, name);
}

void displayList(Node* head) {
    int count = 1;
    while (head) {
        printf(BLUE "%2d. %s\n" RESET, count++, head->name);
        head = head->next;
    }
}

void deleteState(Node** head, char* name) {
    char inputLower[MAX_NAME];
    toLower(inputLower, name);

    Node *temp = *head, *prev = NULL;

    while (temp) {
        char nodeNameLower[MAX_NAME];
        toLower(nodeNameLower, temp->name);

        if (strstr(nodeNameLower, inputLower) != NULL) {
            if (prev == NULL) {
                *head = temp->next;
            } else {
                prev->next = temp->next;
            }
            printf(GREEN "State deleted: %s\n" RESET, temp->name);
            free(temp);
            return;
        }

        prev = temp;
        temp = temp->next;
    }

    printf(RED "No matching state found to delete.\n" RESET);
}

void searchState(Node* head, char* name) {
    char inputLower[MAX_NAME];
    toLower(inputLower, name);

    int found = 0, pos = 1;
    while (head) {
        char nodeNameLower[MAX_NAME];
        toLower(nodeNameLower, head->name);

        if (strstr(nodeNameLower, inputLower) != NULL) {
            printf(GREEN "Match found at position %d: %s\n" RESET, pos, head->name);
            found = 1;
        }
        pos++;
        head = head->next;
    }
    if (!found)
        printf(RED "No matching state found.\n" RESET);
}

void toLower(char* dest, const char* src) {
    int i = 0;
    while (src[i]) {
        dest[i] = tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

int getStateOrder(const char* name) {
    for (int i = 0; i < 16; i++) {
        if (strcasecmp(states[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

void sortList(Node** head, int ascending) {
    if (!*head) return;

    int swapped;
    Node* ptr1;
    do {
        swapped = 0;
        ptr1 = *head;
        while (ptr1->next) {
            int order1 = getStateOrder(ptr1->name);
            int order2 = getStateOrder(ptr1->next->name);
            if ((ascending && order1 > order2) || (!ascending && order1 < order2)) {
                char temp[MAX_NAME];
                strcpy(temp, ptr1->name);
                strcpy(ptr1->name, ptr1->next->name);
                strcpy(ptr1->next->name, temp);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
    } while (swapped);
}
