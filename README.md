# Q11 – BST vs Linear Search on Government Identification Numbers

Keys: `A102, A25, A7, B100, B12, A120, B3, A45` (strings, compared lexicographically with `strcmp` order).

## Files
| File | Purpose |
|---|---|
| `bst.c` | C source: BST insert, inorder, BST search, linear search, experiments |
| `input.txt` | Input data (insertion order as given) |
| `output.txt` | Full program output (trace, tables, experiments) |
| `README.md` | Trace table, complexity analysis, comparison table, conclusion |

Build and run: `gcc -O2 -o bst bst.c -lm && ./bst input.txt`

---

## (a) BST construction and inorder traversal

### Trace table (insertion)
| Step | Key | Path (node compared, direction) | Placed as |
|---|---|---|---|
| 1 | A102 | (empty tree) | root |
| 2 | A25 | A102(R) | right of A102 |
| 3 | A7 | A102(R) A25(R) | right of A25 |
| 4 | B100 | A102(R) A25(R) A7(R) | right of A7 |
| 5 | B12 | A102(R) A25(R) A7(R) B100(R) | right of B100 |
| 6 | A120 | A102(R) A25(L) | left of A25 |
| 7 | B3 | A102(R) A25(R) A7(R) B100(R) B12(R) | right of B12 |
| 8 | A45 | A102(R) A25(R) A7(L) | left of A7 |

### Resulting tree
```
A102
  \
   A25
   /  \
A120   A7
      /  \
   A45   B100
            \
             B12
               \
                B3
```

**Inorder traversal:** `A102 A120 A25 A45 A7 B100 B12 B3` (sorted, in string order, so "A25" comes before "A7").

### Structure analysis
- 8 nodes, height **5 edges (6 levels)**. A perfectly balanced tree of 8 nodes needs only 3 edges (4 levels).
- The tree is **right-skewed**: the root A102 has no left child, and the chain A102 → A25 → A7 → B100 → B12 → B3 is 6 nodes deep. Only A120 and A45 are on left branches.
- Cause: the input order is almost sorted (A102, A25, A7, B100, B12 are already ascending), and a BST does no rebalancing.
- Note: ordering is lexicographic, not numeric. "A25" < "A7" and "A102" < "A25" because characters are compared left to right.

---

## (b) BST search vs linear search (number of key comparisons)

| Key | BST comparisons | Linear comparisons | Found |
|---|---|---|---|
| A102 | 1 | 1 | yes |
| A25 | 2 | 2 | yes |
| A7 | 3 | 3 | yes |
| B100 | 4 | 4 | yes |
| B12 | 5 | 5 | yes |
| A120 | 3 | 6 | yes |
| B3 | 6 | 7 | yes |
| A45 | 4 | 8 | yes |
| A999 | 4 | 8 | no |
| B1 | 4 | 8 | no |
| C50 | 6 | 8 | no |

| Measure | BST | Linear |
|---|---|---|
| Total, 8 successful searches | 28 | 36 |
| Average, successful | 3.50 | 4.50 |
| Average, unsuccessful (3 keys) | 4.67 | 8.00 |

On this small, skewed tree the BST wins only modestly (22% fewer comparisons on hits). The first five keys cost the same in both, because they lie on one chain.

---

## (c) Effect of insertion order and key length

Same 8 keys, different insertion orders (from `output.txt`):

| Scenario | Height (edges) | Avg BST comps | Avg Linear comps |
|---|---|---|---|
| Given order | 5 | 3.50 | 4.50 |
| Sorted ascending (worst case) | 7 | 4.50 | 4.50 |
| Sorted descending (worst case) | 7 | 4.50 | 4.50 |
| Median-first (best case) | 3 | 2.62 | 4.50 |

Scaling with n (keys `A00000…`), random rows averaged over 20 shuffles:

| Scenario | n | Height | Avg BST comps | Avg Linear comps |
|---|---|---|---|---|
| Random order | 1000 | 21.4 | 11.91 | 500.50 |
| Sorted order | 1000 | 999 | 500.50 | 500.50 |
| Random order | 4000 | 26.0 | 14.61 | 2000.50 |
| Sorted order | 4000 | 3999 | 2000.50 | 2000.50 |

Key length (same 8 numbers, same order):

| Key format | Height | Avg key comparisons | Avg character comparisons |
|---|---|---|---|
| Variable length (`A102`, `A7`, …) | 4 | 3.00 | 8.88 |
| Fixed 3 digits (`A102`, `A007`, …) | 3 | 2.88 | 10.00 |
| Fixed 10 digits | 3 | 2.88 | 30.12 |

(These three rows use the same numbers with an `A` prefix for all keys, i.e. A100/A12/A3 instead of B100/B12/B3, so the variable-length row differs from the "given order" tree in part (a), which is why its height is 4 rather than 5.)

### Findings
1. **Insertion order controls height.** Sorted input gives a chain (height n−1) and BST search degrades to linear search (4.50 = 4.50 for n=8; 500.50 = 500.50 for n=1000). Median-first order gives the minimum height ⌊log₂ n⌋.
2. **Random order stays close to balanced.** Height 21–26 for n = 1000–4000 versus 999–3999 for sorted input; average search cost ~12–15 comparisons versus 500–2000 for linear search. This matches the expected ≈ 1.39·log₂ n.
3. **Key length does not change the tree shape** for the same ordering relation; it changes the *cost per comparison*. Each key comparison costs up to L character comparisons (L = key length): 8.9 → 10 → 30.1 character comparisons per search as keys grow. Zero-padded fixed-length keys also change the ordering (numeric instead of lexicographic), which here happened to give a shorter tree.
4. **Observed vs theory:**

| Case | Theory | Observed |
|---|---|---|
| Best (balanced) | height ⌊log₂ n⌋, search O(log n) | n=8: height 3, 2.62 avg |
| Average (random) | O(log n), ≈1.39 log₂ n | n=4000: 14.61 avg (log₂ n = 12) |
| Worst (sorted) | height n−1, search O(n) | n=4000: 2000.5 avg = (n+1)/2 |

---

## Complexity analysis

| Operation | Best | Average | Worst | Space |
|---|---|---|---|---|
| BST insert / search | O(log n) (balanced) | O(log n) (random order) | O(n) (sorted input / chain) | O(n) nodes (+ O(h) recursion for traversals/height) |
| Linear search | O(1) (first element) | O(n) | O(n) | O(n) array, O(1) extra |
| BST inorder traversal | O(n) | O(n) | O(n) | O(h) stack |
| Balanced BST (AVL / Red-Black) insert / search | O(log n) | O(log n) | O(log n) | O(n) |

With character comparisons included, multiply each by O(L) for key length L.

---

## Comparison table

| Criterion | Linear search | Plain BST | Balanced BST (AVL/RB) |
|---|---|---|---|
| Search worst case | O(n) | O(n) | O(log n) |
| Search average | O(n) | O(log n) | O(log n) |
| Insert | O(1) append | O(h) | O(log n) |
| Sorted output | needs sort O(n log n) | inorder O(n) | inorder O(n) |
| Depends on input order | no | yes | no |
| Extra memory | none | 2 pointers per key | 2 pointers + balance info |
| Measured, n=4000 random | 2000.5 | 14.61 | ≈ 12–13 (theory) |
| Measured, n=4000 sorted | 2000.5 | 2000.5 | ≈ 12 (theory) |

---

## Final conclusion and recommended approach

- For the given 8 identification numbers a plain BST needs 3.50 comparisons on average versus 4.50 for linear search, but the tree is skewed (height 5, ideal 3) because the input is nearly sorted.
- The benefit of a BST depends entirely on its height. Random order keeps it logarithmic; sorted or nearly sorted arrivals (typical for sequentially issued IDs) degrade it to a linked list, losing all advantage.
- **Suggested approach for a growing database:** use a **self-balancing BST (AVL or Red-Black tree)**, which guarantees O(log n) search and insert regardless of insertion order. For very large or disk-resident data, use a **B-tree / B+ tree** (as in database indexes), or a **hash table** if only exact-match lookups are needed and sorted order is not required. Use fixed-length, normalised ID formats to keep per-comparison cost small and make ordering predictable.

---

## Upload to GitHub
```bash
cd q11
git init
git add bst.c input.txt output.txt README.md
git commit -m "Q11: BST vs linear search"
git branch -M main
git remote add origin https://github.com/<your-username>/<repo-name>.git
git push -u origin main
```
Submit the repository URL.
