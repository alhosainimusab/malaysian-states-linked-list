/*
 * states_list.c - Implementation of the Malaysian states linked list.
 */
#include "states_list.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const char *const DEFAULT_STATES[] = {
    "Johore", "Kelantan", "Sabah", "Perak", "Kedah", "Malacca", "Putrajaya",
    "Pahang", "Perlis", "Labuan", "Negeri Sembilan", "Sarawak", "Pulau Pinang",
    "Selangor", "Terengganu", "Kuala Lumpur"
};
const size_t DEFAULT_STATE_COUNT = sizeof DEFAULT_STATES / sizeof DEFAULT_STATES[0];

/* ---------- string helpers ---------- */

int str_icmp(const char *a, const char *b)
{
    unsigned char ca, cb;
    do {
        ca = (unsigned char)tolower((unsigned char)*a++);
        cb = (unsigned char)tolower((unsigned char)*b++);
    } while (ca != '\0' && ca == cb);
    return (int)ca - (int)cb;
}

int str_icontains(const char *haystack, const char *needle)
{
    size_t n = strlen(needle);
    if (n == 0)
        return 0; /* an empty query matches nothing */
    for (; *haystack; haystack++) {
        size_t i = 0;
        while (i < n && haystack[i] &&
               tolower((unsigned char)haystack[i]) == tolower((unsigned char)needle[i]))
            i++;
        if (i == n)
            return 1;
    }
    return 0;
}

int is_blank(const char *s)
{
    for (; *s; s++)
        if (!isspace((unsigned char)*s))
            return 0;
    return 1;
}

/* ---------- list operations ---------- */

static Node *create_node(const char *name)
{
    Node *node = malloc(sizeof *node);
    if (!node)
        return NULL;
    strcpy(node->name, name); /* length already validated by caller */
    node->next = NULL;
    return node;
}

ListStatus list_append(Node **head, const char *name)
{
    if (!name || is_blank(name))
        return LIST_ERR_EMPTY_NAME;
    if (strlen(name) >= MAX_NAME)
        return LIST_ERR_TOO_LONG;
    if (list_find_exact(*head, name))
        return LIST_ERR_DUPLICATE;

    Node *node = create_node(name);
    if (!node)
        return LIST_ERR_NO_MEMORY;

    Node **link = head; /* walk to the NULL link at the tail */
    while (*link)
        link = &(*link)->next;
    *link = node;
    return LIST_OK;
}

ListStatus list_load_defaults(Node **head)
{
    for (size_t i = 0; i < DEFAULT_STATE_COUNT; i++) {
        ListStatus s = list_append(head, DEFAULT_STATES[i]);
        if (s != LIST_OK && s != LIST_ERR_DUPLICATE)
            return s;
    }
    return LIST_OK;
}

size_t list_length(const Node *head)
{
    size_t n = 0;
    for (; head; head = head->next)
        n++;
    return n;
}

const Node *list_find_exact(const Node *head, const char *name)
{
    for (; head; head = head->next)
        if (str_icmp(head->name, name) == 0)
            return head;
    return NULL;
}

size_t list_search(const Node *head, const char *query, MatchCallback cb, void *ctx)
{
    size_t matches = 0, pos = 1;
    for (; head; head = head->next, pos++) {
        if (str_icontains(head->name, query)) {
            matches++;
            if (cb)
                cb(pos, head->name, ctx);
        }
    }
    return matches;
}

/*
 * Delete rules (safer than the submitted version, which deleted the first
 * partial match - so "a" deleted Kelantan and an empty line deleted Johore):
 *   1. an exact case-insensitive match is deleted;
 *   2. otherwise, if exactly one name contains the query, that one is deleted;
 *   3. several partial matches -> LIST_ERR_AMBIGUOUS, nothing is deleted.
 */
ListStatus list_delete(Node **head, const char *query, char removed[MAX_NAME], size_t *match_count)
{
    if (match_count)
        *match_count = 0;
    if (!query || is_blank(query))
        return LIST_ERR_EMPTY_NAME;

    Node **target = NULL;
    for (Node **link = head; *link; link = &(*link)->next) {
        if (str_icmp((*link)->name, query) == 0) {
            target = link;
            if (match_count)
                *match_count = 1;
            break;
        }
    }

    if (!target) {
        size_t partial = 0;
        for (Node **link = head; *link; link = &(*link)->next) {
            if (str_icontains((*link)->name, query)) {
                partial++;
                target = link;
            }
        }
        if (match_count)
            *match_count = partial;
        if (partial == 0)
            return LIST_ERR_NOT_FOUND;
        if (partial > 1)
            return LIST_ERR_AMBIGUOUS;
    }

    Node *victim = *target;
    *target = victim->next; /* same code path for head, middle and tail */
    if (removed)
        strcpy(removed, victim->name);
    free(victim);
    return LIST_OK;
}

/* ---------- merge sort (relinks nodes, O(n log n), stable) ---------- */

static int in_order(const Node *a, const Node *b, SortOrder order)
{
    int cmp = str_icmp(a->name, b->name);
    return order == SORT_ASCENDING ? cmp <= 0 : cmp >= 0;
}

static Node *merge(Node *a, Node *b, SortOrder order)
{
    Node dummy;
    Node *tail = &dummy;
    while (a && b) {
        if (in_order(a, b, order)) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = a ? a : b;
    return dummy.next;
}

static Node *merge_sort(Node *head, SortOrder order)
{
    if (!head || !head->next)
        return head;

    Node *slow = head, *fast = head->next; /* find the middle */
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    Node *back = slow->next;
    slow->next = NULL;

    return merge(merge_sort(head, order), merge_sort(back, order), order);
}

void list_sort(Node **head, SortOrder order)
{
    *head = merge_sort(*head, order);
}

void list_free(Node **head)
{
    Node *cur = *head;
    while (cur) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    *head = NULL;
}

const char *list_status_message(ListStatus status)
{
    switch (status) {
    case LIST_OK:             return "OK";
    case LIST_ERR_EMPTY_NAME: return "Name cannot be empty.";
    case LIST_ERR_TOO_LONG:   return "Name is too long (max 49 characters).";
    case LIST_ERR_DUPLICATE:  return "That state is already in the list.";
    case LIST_ERR_NO_MEMORY:  return "Memory allocation failed.";
    case LIST_ERR_NOT_FOUND:  return "No matching state found.";
    case LIST_ERR_AMBIGUOUS:  return "More than one state matches; please be more specific.";
    }
    return "Unknown error.";
}
