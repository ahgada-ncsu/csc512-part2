#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*combine_function)(const void *a, const void *b, void *result);

typedef struct segment_tree
{
    void *root;
    void *identity;
    size_t elem_size;
    size_t length;
    combine_function combine;
} segment_tree;

void segment_tree_build(segment_tree *tree)
{
    size_t elem_size = tree->elem_size;
    int index = (tree->length - 2);
    size_t b, l, r;
    char *ptr = (char *)tree->root;
    for (; index >= 0; index--)
    {
        b = index * elem_size;
        l = (2 * index + 1) * elem_size;
        r = (2 * index + 2) * elem_size;
        tree->combine(ptr + l, ptr + r, ptr + b);
    }
}

void segment_tree_update(segment_tree *tree, size_t index, void *val)
{
    size_t elem_size = tree->elem_size;
    index = index + tree->length - 1;
    char *base = (char *)tree->root;
    char *t = base + index * elem_size;
    memcpy(t, val, elem_size);
    while (index > 0)
    {
        index = ((index - 1) >> 1);
        tree->combine(base + (2 * index + 1) * elem_size,
                      base + (2 * index + 2) * elem_size,
                      base + index * elem_size);
    }
}

void segment_tree_query(segment_tree *tree, long long l, long long r, void *res)
{
    size_t elem_size = tree->elem_size;
    memcpy(res, tree->identity, elem_size);
    elem_size = tree->elem_size;
    char *root = (char *)tree->root;
    l += tree->length - 1;
    r += tree->length - 1;
    while (l <= r)
    {
        if (!(l & 1))
        {
            tree->combine(res, root + l * elem_size, res);
        }
        if (r & 1)
        {
            tree->combine(res, root + r * elem_size, res);
        }
        r = (r >> 1) - 1;
        l = (l >> 1);
    }
}

segment_tree *segment_tree_init(void *arr, size_t elem_size, size_t len,
                                void *identity, combine_function func)
{
    segment_tree *tree = malloc(sizeof(segment_tree));
    tree->elem_size = elem_size;
    tree->length = len;
    tree->combine = func;
    tree->root = malloc(sizeof(char) * elem_size * (2 * len - 1));
    tree->identity = malloc(sizeof(char) * elem_size);
    char *ptr = (char *)tree->root;
    memset(ptr, 0, (len - 1) * elem_size);
    ptr = ptr + (len - 1) * elem_size;
    memcpy(ptr, arr, elem_size * len);
    memcpy(tree->identity, identity, elem_size);
    return tree;
}

void segment_tree_dispose(segment_tree *tree)
{
    free(tree->root);
    free(tree->identity);
    free(tree);
}

void segment_tree_print_int(segment_tree *tree)
{
    printf("Segment Tree Contents:\n");
    char *base = (char *)tree->root;
    size_t i = 0;
    for (; i < 2 * tree->length - 1; i++)
    {
        printf("%d ", *(int *)(base + i * tree->elem_size));
    }
    printf("\n");
}

void segment_tree_print_detailed_int(segment_tree *tree)
{
    printf("Detailed Segment Tree Information:\n");
    printf("Tree Length: %zu\n", tree->length);
    printf("Element Size: %zu bytes\n", tree->elem_size);
    
    printf("\nLeaf Nodes (Original Array):\n");
    char *base = (char *)tree->root;
    size_t leaf_start = tree->length - 1;
    for (size_t i = 0; i < tree->length; i++)
    {
        printf("Leaf %zu: %d\n", i, *(int *)(base + (leaf_start + i) * tree->elem_size));
    }
}

void minimum(const void *a, const void *b, void *c)
{
    *(int *)c = *(int *)a < *(int *)b ? *(int *)a : *(int *)b;
}

void maximum(const void *a, const void *b, void *c)
{
    *(int *)c = *(int *)a > *(int *)b ? *(int *)a : *(int *)b;
}

static void test1()
{
    printf("Running Test 1:\n");
    int32_t arr[10] = {1, 0, 3, 5, 7, 2, 11, 6, -2, 8};
    int32_t identity = __INT32_MAX__;
    segment_tree *tree = segment_tree_init(arr, sizeof(*arr), 10, &identity, minimum);
    segment_tree_build(tree);
    
    printf("Initial Tree:\n");
    segment_tree_print_int(tree);
    segment_tree_print_detailed_int(tree);
    
    int32_t result;
    segment_tree_query(tree, 3, 6, &result);
    printf("Minimum in range 3-6: %d\n", result);
    assert(result == 2);
    
    segment_tree_query(tree, 8, 9, &result);
    printf("Minimum in range 8-9: %d\n", result);
    assert(result == -2);
    
    result = 12;
    printf("Updating index 5 and 8 with value %d\n", result);
    segment_tree_update(tree, 5, &result);
    segment_tree_update(tree, 8, &result);
    
    segment_tree_query(tree, 0, 3, &result);
    printf("Minimum in range 0-3: %d\n", result);
    assert(result == 0);
    
    segment_tree_query(tree, 8, 9, &result);
    printf("Minimum in range 8-9: %d\n", result);
    assert(result == 8);
    
    segment_tree_dispose(tree);
    printf("Test 1 Completed Successfully\n\n");
}

static void test2()
{
    printf("Running Test 2:\n");
    int32_t arr[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    int32_t identity_min = INT32_MAX;
    int32_t identity_max = INT32_MIN;
    
    segment_tree *tree_min = segment_tree_init(arr, sizeof(*arr), 8, &identity_min, minimum);
    segment_tree *tree_max = segment_tree_init(arr, sizeof(*arr), 8, &identity_max, maximum);
    
    segment_tree_build(tree_min);
    segment_tree_build(tree_max);
    
    printf("Minimum Tree:\n");
    segment_tree_print_int(tree_min);
    segment_tree_print_detailed_int(tree_min);
    
    printf("Maximum Tree:\n");
    segment_tree_print_int(tree_max);
    segment_tree_print_detailed_int(tree_max);
    
    int32_t min_result, max_result;
    segment_tree_query(tree_min, 2, 5, &min_result);
    printf("Minimum in range 2-5: %d\n", min_result);
    assert(min_result == 30);
    
    segment_tree_query(tree_max, 2, 5, &max_result);
    printf("Maximum in range 2-5: %d\n", max_result);
    assert(max_result == 60);
    
    int32_t update_min = 5;
    int32_t update_max = 90;
    printf("Updating index 3 with values %d (min) and %d (max)\n", update_min, update_max);
    
    segment_tree_update(tree_min, 3, &update_min);
    segment_tree_update(tree_max, 3, &update_max);
    
    segment_tree_query(tree_min, 0, 4, &min_result);
    printf("New Minimum in range 0-4: %d\n", min_result);
    assert(min_result == 5);
    
    segment_tree_query(tree_max, 0, 4, &max_result);
    printf("New Maximum in range 0-4: %d\n", max_result);
    assert(max_result == 90);
    
    segment_tree_dispose(tree_min);
    segment_tree_dispose(tree_max);
    
    printf("Test 2 Completed Successfully\n\n");
}

int main()
{
    int choice;
    printf("Choose a test to run:\n");
    printf("1. Test 1 (Minimum Range Query)\n");
    printf("2. Test 2 (Minimum and Maximum Range Query)\n");
    printf("Enter your choice (1 or 2): ");
    scanf("%d", &choice);
    
    // for(int i=0;i<choice;i++) {
    //     test1();
    // }
    switch(choice)
    {
        case 1:
            test1();
            break;
        case 2:
            test2();
            break;
        default:
            printf("Invalid choice. Exiting.\n");
            return 1;
    }
    
    return 0;
}
