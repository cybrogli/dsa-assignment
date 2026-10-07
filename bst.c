/*
 * Q11: BST vs Linear Search on government identification numbers.
 * Keys are strings (e.g. "A102") compared lexicographically.
 * Build: gcc -O2 -o bst bst.c
 * Run:   ./bst input.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXK 16
#define MAXN 5000

typedef struct Node {
    char key[MAXK];
    struct Node *left, *right;
} Node;

/* ---- comparison counters ---- */
static long key_cmps = 0;   /* number of key comparisons (one per node visited) */
static long char_cmps = 0;  /* character-level comparisons inside strcmp */

static int cmp(const char *a, const char *b) {
    key_cmps++;
    while (*a && *b) {
        char_cmps++;
        if (*a != *b) return (unsigned char)*a - (unsigned char)*b;
        a++; b++;
    }
    char_cmps++;
    return (unsigned char)*a - (unsigned char)*b;
}

static Node *new_node(const char *k) {
    Node *n = malloc(sizeof(Node));
    strncpy(n->key, k, MAXK - 1);
    n->key[MAXK - 1] = '\0';
    n->left = n->right = NULL;
    return n;
}

/* Insert iteratively; optionally print the path (trace). */
static Node *insert(Node *root, const char *k, int trace, int step) {
    char path[512] = "";
    if (!root) {
        if (trace) printf("| %2d | %-5s | %-42s | %-22s |\n", step, k, "(empty tree)", "root");
        return new_node(k);
    }
    Node *cur = root;
    for (;;) {
        int c = cmp(k, cur->key);
        if (trace) {
            char buf[48];
            snprintf(buf, sizeof buf, "%s%s ", cur->key, c < 0 ? "(L)" : "(R)");
            strncat(path, buf, sizeof path - strlen(path) - 1);
        }
        if (c == 0) return root; /* duplicate: ignore */
        Node **next = c < 0 ? &cur->left : &cur->right;
        if (!*next) {
            *next = new_node(k);
            if (trace) printf("| %2d | %-5s | %-42s | %s child of %-5s |\n", step, k, path,
                              c < 0 ? "left " : "right", cur->key);
            return root;
        }
        cur = *next;
    }
}

static int height(Node *n) { /* edges; empty = -1 */
    if (!n) return -1;
    int l = height(n->left), r = height(n->right);
    return 1 + (l > r ? l : r);
}
static int count(Node *n) { return n ? 1 + count(n->left) + count(n->right) : 0; }

static void inorder(Node *n) {
    if (!n) return;
    inorder(n->left);
    printf("%s ", n->key);
    inorder(n->right);
}

static void print_levels(Node *n, int depth) {
    if (!n) return;
    print_levels(n->right, depth + 1);
    printf("%*s%s\n", depth * 6, "", n->key);
    print_levels(n->left, depth + 1);
}

static void print_node_depths(Node *n, int d) {
    if (!n) return;
    print_node_depths(n->left, d + 1);
    printf("| %-5s | %5d | %-5s | %-5s |\n", n->key, d,
           n->left ? n->left->key : "-", n->right ? n->right->key : "-");
    print_node_depths(n->right, d + 1);
}

static void free_tree(Node *n) {
    if (!n) return;
    free_tree(n->left); free_tree(n->right); free(n);
}

/* ---- searches ---- */
static int bst_search(Node *root, const char *k) {
    long before = key_cmps;
    Node *cur = root;
    while (cur) {
        int c = cmp(k, cur->key);
        if (c == 0) break;
        cur = c < 0 ? cur->left : cur->right;
    }
    return (int)(key_cmps - before);
}

static int linear_search(char a[][MAXK], int n, const char *k) {
    int comps = 0;
    for (int i = 0; i < n; i++) {
        comps++;
        if (strcmp(a[i], k) == 0) break;
    }
    return comps;
}

/* ---- experiment helpers ---- */
static Node *build(char a[][MAXK], int n) {
    Node *r = NULL;
    for (int i = 0; i < n; i++) r = insert(r, a[i], 0, 0);
    return r;
}

static int qsort_cmp(const void *a, const void *b) { return strcmp((const char *)a, (const char *)b); }

static void balanced_order(char sorted[][MAXK], char out[][MAXK], int lo, int hi, int *pos) {
    if (lo > hi) return;
    int mid = (lo + hi) / 2;
    strcpy(out[(*pos)++], sorted[mid]);
    balanced_order(sorted, out, lo, mid - 1, pos);
    balanced_order(sorted, out, mid + 1, hi, pos);
}

/* average BST comparisons over all keys present */
static double avg_bst(Node *root, char a[][MAXK], int n, double *avg_chars) {
    long total = 0, c0 = char_cmps;
    for (int i = 0; i < n; i++) total += bst_search(root, a[i]);
    *avg_chars = (double)(char_cmps - c0) / n;
    return (double)total / n;
}

static void report(const char *label, char a[][MAXK], int n) {
    Node *r = build(a, n);
    double ch;
    double avg = avg_bst(r, a, n, &ch);
    double lin = 0;
    for (int i = 0; i < n; i++) lin += linear_search(a, n, a[i]);
    lin /= n;
    printf("| %-34s | %4d | %6d | %9.2f | %9.2f | %10.2f |\n", label, n, height(r), avg, lin, ch);
    free_tree(r);
}

int main(int argc, char **argv) {
    static char keys[MAXN][MAXK];
    int n = 0;
    FILE *f = fopen(argc > 1 ? argv[1] : "input.txt", "r");
    if (!f) { perror("input"); return 1; }
    while (n < MAXN && fscanf(f, "%15s", keys[n]) == 1) n++;
    fclose(f);

    /* ============ (a) build BST, trace, inorder ============ */
    printf("=== (a) BST CONSTRUCTION ===\n");
    printf("Input order: ");
    for (int i = 0; i < n; i++) printf("%s ", keys[i]);
    printf("\n\nTRACE TABLE (insertion)\n");
    printf("| St | Key   | Path (key compared, direction)             | Placed as              |\n");
    printf("|----|-------|--------------------------------------------|------------------------|\n");
    Node *root = NULL;
    for (int i = 0; i < n; i++) root = insert(root, keys[i], 1, i + 1);

    printf("\nInorder traversal: ");
    inorder(root);
    printf("\n\nTree (rotated 90 deg, root at left, right subtree on top):\n");
    print_levels(root, 0);
    printf("\nNode table (inorder):\n| Node  | Depth | Left  | Right |\n|-------|-------|-------|-------|\n");
    print_node_depths(root, 0);
    int minh = 0;
    for (int t = count(root); t > 1; t >>= 1) minh++;   /* floor(log2 n) */
    printf("\nNodes = %d, Height = %d edges (%d levels), minimum possible height = %d edges\n",
           count(root), height(root), height(root) + 1, minh);

    /* ============ (b) BST vs linear search ============ */
    printf("\n=== (b) BST SEARCH vs LINEAR SEARCH ===\n");
    printf("| Key   | BST comps | Linear comps | Found |\n|-------|-----------|--------------|-------|\n");
    long tb = 0, tl = 0;
    for (int i = 0; i < n; i++) {
        int b = bst_search(root, keys[i]);
        int l = linear_search(keys, n, keys[i]);
        tb += b; tl += l;
        printf("| %-5s | %9d | %12d | yes   |\n", keys[i], b, l);
    }
    const char *missing[] = {"A999", "B1", "C50"};
    long mb = 0, ml = 0;
    for (int i = 0; i < 3; i++) {
        int b = bst_search(root, missing[i]);
        int l = linear_search(keys, n, missing[i]);
        mb += b; ml += l;
        printf("| %-5s | %9d | %12d | no    |\n", missing[i], b, l);
    }
    printf("\nAverage (successful): BST = %.2f, Linear = %.2f\n", (double)tb / n, (double)tl / n);
    printf("Average (unsuccessful): BST = %.2f, Linear = %.2f\n", (double)mb / 3, (double)ml / 3);
    printf("Total (successful, 8 keys): BST = %ld, Linear = %ld\n", tb, tl);

    /* ============ (c) effect of insertion order / key length ============ */
    printf("\n=== (c) INSERTION ORDER AND KEY LENGTH ===\n");
    printf("| Scenario                           |    n | Height | Avg BST   | Avg Lin   | Avg chars  |\n");
    printf("|------------------------------------|------|--------|-----------|-----------|------------|\n");

    static char sorted[MAXN][MAXK], rev[MAXN][MAXK], bal[MAXN][MAXK], tmp[MAXN][MAXK];
    memcpy(sorted, keys, sizeof keys);
    qsort(sorted, n, MAXK, qsort_cmp);
    for (int i = 0; i < n; i++) strcpy(rev[i], sorted[n - 1 - i]);
    int pos = 0;
    balanced_order(sorted, bal, 0, n - 1, &pos);

    report("Given order", keys, n);
    report("Sorted ascending (worst case)", sorted, n);
    report("Sorted descending (worst case)", rev, n);
    report("Median-first (best case)", bal, n);

    /* key length: same 8 numbers, fixed-width (zero padded) vs variable */
    const char *nums[] = {"102", "25", "7", "100", "12", "120", "3", "45"};
    for (int i = 0; i < 8; i++) sprintf(tmp[i], "A%s", nums[i]);
    report("Variable length A+digits (given)", tmp, 8);
    for (int i = 0; i < 8; i++) sprintf(tmp[i], "A%03d", atoi(nums[i]));
    report("Fixed length A+3 digits, same order", tmp, 8);
    for (int i = 0; i < 8; i++) sprintf(tmp[i], "A%010d", atoi(nums[i]));
    report("Fixed length A+10 digits, same order", tmp, 8);

    /* scaling: random vs sorted insertion, random rows averaged over 20 shuffles */
    srand(42);
    int sizes[2] = {1000, 4000};
    for (int s = 0; s < 2; s++) {
        int m = sizes[s];
        static char r[MAXN][MAXK], so[MAXN][MAXK];
        double hsum = 0, asum = 0, csum = 0; int hmax = 0;
        const int TRIALS = 20;
        for (int t = 0; t < TRIALS; t++) {
            for (int i = 0; i < m; i++) sprintf(r[i], "A%05d", i);
            for (int i = m - 1; i > 0; i--) {   /* Fisher-Yates shuffle */
                int j = rand() % (i + 1);
                char tt[MAXK]; strcpy(tt, r[i]); strcpy(r[i], r[j]); strcpy(r[j], tt);
            }
            Node *rt = build(r, m);
            double ch, a = avg_bst(rt, r, m, &ch);
            int h = height(rt);
            hsum += h; asum += a; csum += ch; if (h > hmax) hmax = h;
            free_tree(rt);
        }
        char lab[64];
        sprintf(lab, "Random order, n=%d (avg of %d)", m, TRIALS);
        printf("| %-34s | %4d | %6.1f | %9.2f | %9.2f | %10.2f |\n", lab, m, hsum / TRIALS, asum / TRIALS, (m + 1) / 2.0, csum / TRIALS);
        for (int i = 0; i < m; i++) sprintf(so[i], "A%05d", i);   /* already sorted */
        sprintf(lab, "Sorted order, n=%d (degenerate)", m);
        Node *rt = build(so, m);
        double ch, a = avg_bst(rt, so, m, &ch);
        printf("| %-34s | %4d | %6d | %9.2f | %9.2f | %10.2f |\n", lab, m, height(rt), a, (m + 1) / 2.0, ch);
        free_tree(rt);
        printf("|   (max random height seen: %2d; ideal ~ log2(n) = %.1f)\n", hmax, __builtin_log2(m));
    }

    free_tree(root);
    return 0;
}
