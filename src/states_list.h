/*
 * states_list.h - Singly linked list of Malaysian states and federal territories.
 *
 * The list logic is kept free of any printing so it can be unit tested.
 * All user interaction lives in main.c.
 */
#ifndef STATES_LIST_H
#define STATES_LIST_H

#include <stddef.h>

#define MAX_NAME 50 /* buffer size, including the terminating '\0' */

typedef struct Node {
    char name[MAX_NAME];
    struct Node *next;
} Node;

typedef enum {
    LIST_OK = 0,
    LIST_ERR_EMPTY_NAME,  /* name is empty or only whitespace          */
    LIST_ERR_TOO_LONG,    /* name does not fit in MAX_NAME - 1 chars   */
    LIST_ERR_DUPLICATE,   /* name already exists (case-insensitive)    */
    LIST_ERR_NO_MEMORY,   /* malloc failed                             */
    LIST_ERR_NOT_FOUND,   /* nothing matched the query                 */
    LIST_ERR_AMBIGUOUS    /* several partial matches, none exact       */
} ListStatus;

typedef enum { SORT_ASCENDING, SORT_DESCENDING } SortOrder;

/* Called once per search hit with the 1-based position and the name. */
typedef void (*MatchCallback)(size_t position, const char *name, void *ctx);

/* The 13 states and 3 federal territories used to build the initial list. */
extern const char *const DEFAULT_STATES[];
extern const size_t DEFAULT_STATE_COUNT;

/* String helpers (portable replacements for POSIX strcasecmp etc.). */
int str_icmp(const char *a, const char *b);
int str_icontains(const char *haystack, const char *needle);
int is_blank(const char *s);

/* List operations. */
ListStatus list_append(Node **head, const char *name);
ListStatus list_load_defaults(Node **head);
size_t list_length(const Node *head);
const Node *list_find_exact(const Node *head, const char *name);
size_t list_search(const Node *head, const char *query, MatchCallback cb, void *ctx);
ListStatus list_delete(Node **head, const char *query, char removed[MAX_NAME], size_t *match_count);
void list_sort(Node **head, SortOrder order);
void list_free(Node **head);
const char *list_status_message(ListStatus status);

#endif /* STATES_LIST_H */
