/*
 * test_states_list.c - Unit tests for the linked list (no framework needed).
 * Build and run with:  make test
 */
#include "../src/states_list.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0, tests_failed = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        tests_run++;                                                       \
        if (!(cond)) {                                                     \
            tests_failed++;                                                \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
        }                                                                  \
    } while (0)

static const char *name_at(const Node *head, size_t index)
{
    for (size_t i = 0; head && i < index; i++)
        head = head->next;
    return head ? head->name : NULL;
}

static void count_cb(size_t pos, const char *name, void *ctx)
{
    (void)pos; (void)name;
    (*(int *)ctx)++;
}

static void last_pos_cb(size_t pos, const char *name, void *ctx)
{
    (void)name;
    *(size_t *)ctx = pos;
}

static void test_string_helpers(void)
{
    CHECK(str_icmp("Perak", "pErAk") == 0);
    CHECK(str_icmp("Johore", "Kedah") < 0);
    CHECK(str_icontains("Kuala Lumpur", "LUMP"));
    CHECK(!str_icontains("Sabah", "xyz"));
    CHECK(!str_icontains("Sabah", ""));
    CHECK(is_blank("   \t"));
    CHECK(!is_blank(" a "));
}

static void test_load_defaults(void)
{
    Node *head = NULL;
    CHECK(list_load_defaults(&head) == LIST_OK);
    CHECK(list_length(head) == 16);
    CHECK(DEFAULT_STATE_COUNT == 16);
    CHECK(strcmp(name_at(head, 0), "Johore") == 0);
    CHECK(strcmp(name_at(head, 15), "Kuala Lumpur") == 0);
    list_free(&head);
    CHECK(head == NULL);
}

static void test_append_validation(void)
{
    Node *head = NULL;
    char long_name[MAX_NAME + 10];
    memset(long_name, 'x', sizeof long_name - 1);
    long_name[sizeof long_name - 1] = '\0';

    CHECK(list_append(&head, "Cyberjaya") == LIST_OK);
    CHECK(list_append(&head, "cyberjaya") == LIST_ERR_DUPLICATE);
    CHECK(list_append(&head, "") == LIST_ERR_EMPTY_NAME);
    CHECK(list_append(&head, "   ") == LIST_ERR_EMPTY_NAME);
    CHECK(list_append(&head, long_name) == LIST_ERR_TOO_LONG);

    /* Boundary: 49 characters fit, 50 do not (buffer is MAX_NAME incl. '\0'). */
    char fits[MAX_NAME], too_long[MAX_NAME + 1];
    memset(fits, 'a', MAX_NAME - 1);
    fits[MAX_NAME - 1] = '\0';
    memset(too_long, 'b', MAX_NAME);
    too_long[MAX_NAME] = '\0';
    CHECK(list_append(&head, fits) == LIST_OK);
    CHECK(list_append(&head, too_long) == LIST_ERR_TOO_LONG);
    CHECK(list_length(head) == 2);
    list_free(&head);
}

static void test_sort(void)
{
    Node *head = NULL;
    list_load_defaults(&head);
    list_append(&head, "Cyberjaya"); /* not in the default list */

    list_sort(&head, SORT_ASCENDING);
    CHECK(list_length(head) == 17);
    CHECK(strcmp(name_at(head, 0), "Cyberjaya") == 0); /* alphabetical, not "unknown first" */
    CHECK(strcmp(name_at(head, 1), "Johore") == 0);
    CHECK(strcmp(name_at(head, 16), "Terengganu") == 0);
    for (const Node *n = head; n && n->next; n = n->next)
        CHECK(str_icmp(n->name, n->next->name) <= 0);

    list_sort(&head, SORT_DESCENDING);
    CHECK(strcmp(name_at(head, 0), "Terengganu") == 0);
    CHECK(strcmp(name_at(head, 16), "Cyberjaya") == 0);
    for (const Node *n = head; n && n->next; n = n->next)
        CHECK(str_icmp(n->name, n->next->name) >= 0);

    Node *empty = NULL;
    list_sort(&empty, SORT_ASCENDING); /* must not crash */
    CHECK(empty == NULL);
    list_free(&head);
}

static void test_search(void)
{
    Node *head = NULL;
    list_load_defaults(&head);
    int hits = 0;
    CHECK(list_search(head, "perak", count_cb, &hits) == 1);
    CHECK(hits == 1);
    size_t pos = 0;
    list_search(head, "Johore", last_pos_cb, &pos);
    CHECK(pos == 1);                      /* positions are 1-based */
    list_search(head, "Kuala", last_pos_cb, &pos);
    CHECK(pos == 16);
    CHECK(list_search(head, "la", NULL, NULL) == 7); /* Kelantan, Malacca, Labuan, Negeri Sembilan, Pulau Pinang, Selangor, Kuala Lumpur */
    CHECK(list_search(head, "Atlantis", NULL, NULL) == 0);
    CHECK(list_search(head, "", NULL, NULL) == 0);
    list_free(&head);
}

static void test_delete(void)
{
    Node *head = NULL;
    char removed[MAX_NAME];
    size_t matches;
    list_load_defaults(&head);

    /* Exact match at the head. */
    CHECK(list_delete(&head, "johore", removed, &matches) == LIST_OK);
    CHECK(strcmp(removed, "Johore") == 0);
    CHECK(strcmp(name_at(head, 0), "Kelantan") == 0);

    /* Exact match at the tail. */
    CHECK(list_delete(&head, "Kuala Lumpur", removed, NULL) == LIST_OK);
    CHECK(list_length(head) == 14);

    /* Unique partial match. */
    CHECK(list_delete(&head, "sembil", removed, NULL) == LIST_OK);
    CHECK(strcmp(removed, "Negeri Sembilan") == 0);

    /* Ambiguous partial match deletes nothing. */
    CHECK(list_delete(&head, "a", removed, &matches) == LIST_ERR_AMBIGUOUS);
    CHECK(matches > 1);
    CHECK(list_length(head) == 13);

    /* Empty input deletes nothing (submitted version deleted the head). */
    CHECK(list_delete(&head, "", removed, NULL) == LIST_ERR_EMPTY_NAME);
    CHECK(list_delete(&head, "Atlantis", removed, NULL) == LIST_ERR_NOT_FOUND);
    CHECK(list_length(head) == 13);

    /* "Perak" exactly matches even though "Perlis" shares a prefix. */
    CHECK(list_delete(&head, "per", removed, &matches) == LIST_ERR_AMBIGUOUS);
    CHECK(list_delete(&head, "Perak", removed, NULL) == LIST_OK);

    list_free(&head);
    Node *empty = NULL;
    CHECK(list_delete(&empty, "Sabah", removed, NULL) == LIST_ERR_NOT_FOUND);
}

int main(void)
{
    test_string_helpers();
    test_load_defaults();
    test_append_validation();
    test_sort();
    test_search();
    test_delete();

    printf("%d checks, %d failed\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}
