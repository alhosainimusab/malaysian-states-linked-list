/*
 * main.c - Menu-driven command-line interface for the states linked list.
 *
 * Usage: ./states [--no-color]     (colours are also disabled if NO_COLOR is set)
 */
#include "states_list.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF 128

/* ANSI colour codes; emptied when colour is disabled. */
static const char *RESET = "\033[0m", *RED = "\033[1;31m", *GREEN = "\033[1;32m",
                  *YELLOW = "\033[1;33m", *BLUE = "\033[1;34m", *CYAN = "\033[1;36m";

static void disable_colour(void)
{
    RESET = RED = GREEN = YELLOW = BLUE = CYAN = "";
}

/* Remove leading/trailing whitespace in place. */
static void trim(char *s)
{
    char *start = s;
    while (isspace((unsigned char)*start))
        start++;
    memmove(s, start, strlen(start) + 1);
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
}

/*
 * Read one line from stdin into buf (newline removed, whitespace trimmed).
 * Returns -1 on EOF, 1 if the line was too long (rest is discarded), 0 otherwise.
 */
static int read_line(char *buf, size_t size)
{
    if (!fgets(buf, (int)size, stdin))
        return -1;

    int truncated = 0;
    if (!strchr(buf, '\n') && !feof(stdin)) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
        truncated = 1;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
    trim(buf);
    return truncated;
}

static int prompt(const char *label, char *buf, size_t size)
{
    printf("%s%s%s", YELLOW, label, RESET);
    fflush(stdout);
    return read_line(buf, size);
}

static void menu(void)
{
    printf("%s\n========== Malaysian States Linked List Menu ==========%s\n", CYAN, RESET);
    printf("1. Display Ascending Order (A-Z)\n");
    printf("2. Display Descending Order (Z-A)\n");
    printf("3. Display Current Order\n");
    printf("4. Insert State\n");
    printf("5. Delete State\n");
    printf("6. Search State\n");
    printf("7. Exit\n");
}

static void display(const Node *head, const char *title)
{
    printf("%s\n--- %s (%lu entries) ---%s\n", CYAN, title, (unsigned long)list_length(head), RESET);
    if (!head) {
        printf("%s(list is empty)%s\n", RED, RESET);
        return;
    }
    int i = 1;
    for (; head; head = head->next)
        printf("%s%2d. %s%s\n", BLUE, i++, head->name, RESET);
}

static void print_match(size_t position, const char *name, void *ctx)
{
    (void)ctx;
    printf("%sMatch found at position %lu: %s%s\n", GREEN, (unsigned long)position, name, RESET);
}

static void error(const char *msg)
{
    printf("%s%s%s\n", RED, msg, RESET);
}

/* Parse a whole line as an integer; returns 0 on failure. */
static int parse_choice(const char *s, long *out)
{
    char *end;
    if (*s == '\0')
        return 0;
    long v = strtol(s, &end, 10);
    if (*end != '\0')
        return 0;
    *out = v;
    return 1;
}

int main(int argc, char *argv[])
{
    if (getenv("NO_COLOR") || (argc > 1 && strcmp(argv[1], "--no-color") == 0))
        disable_colour();

    Node *head = NULL;
    if (list_load_defaults(&head) != LIST_OK) {
        error("Failed to build the initial list.");
        list_free(&head);
        return EXIT_FAILURE;
    }
    printf("%sLoaded %lu states and federal territories.%s\n", GREEN, (unsigned long)list_length(head), RESET);

    char line[LINE_BUF];
    int running = 1;

    while (running) {
        menu();
        int rc = prompt("\nEnter your choice: ", line, sizeof line);
        if (rc < 0)
            break; /* EOF: exit cleanly */

        long choice;
        if (rc > 0 || !parse_choice(line, &choice)) {
            error("Invalid choice. Please enter a number from 1 to 7.");
            continue;
        }

        ListStatus status;
        char removed[MAX_NAME];
        size_t matches;

        switch (choice) {
        case 1:
            list_sort(&head, SORT_ASCENDING);
            display(head, "States in Ascending Order");
            break;
        case 2:
            list_sort(&head, SORT_DESCENDING);
            display(head, "States in Descending Order");
            break;
        case 3:
            display(head, "States in Current Order");
            break;
        case 4:
            rc = prompt("Enter state to insert: ", line, sizeof line);
            if (rc < 0) { running = 0; break; }
            status = rc > 0 ? LIST_ERR_TOO_LONG : list_append(&head, line);
            if (status == LIST_OK)
                printf("%sState inserted: %s%s\n", GREEN, line, RESET);
            else
                error(list_status_message(status));
            break;
        case 5:
            rc = prompt("Enter state to delete: ", line, sizeof line);
            if (rc < 0) { running = 0; break; }
            status = rc > 0 ? LIST_ERR_TOO_LONG : list_delete(&head, line, removed, &matches);
            if (status == LIST_OK) {
                printf("%sState deleted: %s%s\n", GREEN, removed, RESET);
            } else {
                error(list_status_message(status));
                if (status == LIST_ERR_AMBIGUOUS)
                    list_search(head, line, print_match, NULL);
            }
            break;
        case 6:
            rc = prompt("Enter state to search: ", line, sizeof line);
            if (rc < 0) { running = 0; break; }
            if (rc > 0 || line[0] == '\0') {
                error(rc > 0 ? list_status_message(LIST_ERR_TOO_LONG)
                             : list_status_message(LIST_ERR_EMPTY_NAME));
                break;
            }
            if (list_search(head, line, print_match, NULL) == 0)
                error(list_status_message(LIST_ERR_NOT_FOUND));
            break;
        case 7:
            running = 0;
            break;
        default:
            error("Invalid choice. Please enter a number from 1 to 7.");
        }
    }

    printf("%sExiting program. Goodbye!%s\n", GREEN, RESET);
    list_free(&head);
    return EXIT_SUCCESS;
}
