#include <glib.h>

static GHashTable *test_table = NULL;

struct item {
    gint id;
    gchar *data;
};

char data[10][100] = { 
        {"1.1"},
        {"2.22"},
        {"3.333"},
        {"4.4444"},
        {"5.55555"},
        {"6.666666"},
        {"7.7777777"},
        {"8.88888888"},
        {"9.999999999"},
        {"10.0000000000"}};

static void
test_table_cleanup(gpointer key, gpointer value, gpointer user_data)
{
    gint *lkey = key;
    struct item *lvalue = value;

#ifdef KILS_DEBUG
    printf("CLEANUP: %d:%d-%s\n", *lkey, lvalue->id, lvalue->data);
#endif

    if (lvalue) {
        g_free(lvalue->data);
        g_free(lvalue);
        g_free(lkey);
    }
}

static void
test_table_print(gpointer key, gpointer value, gpointer user_data)
{
    gint *lkey = key;
    struct item *lvalue = value;

#ifdef KILS_DEBUG
    printf("PRINT: %d:%d-%s\n", *lkey, lvalue->id, lvalue->data);
#endif
}

static gboolean
test_table_delete(gpointer key, gpointer value, gpointer user_data)
{
    gint *lkey = key;
    gint *luser_data = user_data;
    struct item *lvalue = value;

#ifdef KILS_DEBUG
    printf("REMOVE: %d:%d-%s [%d]\n", *lkey, lvalue->id, lvalue->data, *luser_data);
#endif

    if (lvalue) {
        if (lvalue->id == *luser_data) {
#ifdef KILS_DEBUG
            printf("REMOVE: OK (%d)\n", lvalue->id);
#endif
            g_free(lvalue->data);
            g_free(lvalue);
            g_free(lkey);
            return TRUE;
        }
    }

    return FALSE;
}

void
init_hash()
{
    if (test_table) {
        g_hash_table_foreach(test_table, test_table_cleanup, NULL);
        g_hash_table_destroy(test_table);
    }

    test_table = g_hash_table_new(g_int_hash, g_int_equal);
}

void
print_hash()
{
    if (test_table) {
        g_hash_table_foreach(test_table, test_table_print, NULL);
    }
}

void
delete_hash(int id)
{
    if (test_table) {
        g_hash_table_foreach_remove(test_table, test_table_delete, &id);
    }
}

int
add(int id, gpointer data)
{
    int *key = g_malloc0(sizeof(int));
    *key = id;

    g_hash_table_insert(test_table, key, data);

    return 1;
}

int
dummy_insert()
{
    struct item *pt;
    int i, j;

    for (i = 0, j = 10; i < 10; i++, j++) {
        pt = g_malloc0(sizeof(struct item));
        pt->id = j;
        pt->data = g_strdup(data[i]);
        add (i, pt);
    }
}

main()
{
    struct item *buff;
    int j;

    test_table = g_hash_table_new(g_int_hash, g_int_equal);

#ifdef KILS_DEBUG
    printf("\ndummy_insert()\n");
#endif
    dummy_insert();
#ifdef KILS_DEBUG
    printf("\nprintf_hash()\n");
    print_hash();
#endif



    j = 1;

    buff = g_hash_table_lookup(test_table, &j);
    if (buff) {
#ifdef KILS_DEBUG
        printf("FIND: %d:%d-%s\n", j, buff->id, buff->data);
#endif
        sprintf(buff->data, "kils\n");
    }
    else {
#ifdef KILS_DEBUG
        printf("NOT FOUND\n");
#endif
    }

#ifdef KILS_DEBUG
    printf("\nprintf_hash()\n");
    print_hash();
#endif

#if 0
    j = 2;

    buff = g_hash_table_lookup(test_table, &j);
    if (buff) {
        printf("FIND: %d:%d-%s\n", j, buff->id, buff->data);
    }
    else {
        printf("NOT FOUND\n");
    }


    printf("\ninit_hash()\n");
    init_hash();
    printf("\nprint_hash()\n");
    print_hash();

    printf("\ndummy_insert()\n");
    dummy_insert();
    printf("\nprint_hash()\n");
    print_hash();

    printf("\ndelete_hash()\n");
    delete_hash(17);
    printf("\nprint_hash()\n");
    print_hash();

    printf("\ndelete_hash()\n");
    delete_hash(13);
    printf("\nprint_hash()\n");
    print_hash();
#endif
}
