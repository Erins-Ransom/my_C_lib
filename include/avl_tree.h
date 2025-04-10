/*  
    @file avl_tree.h
 
    @version 1.0

    @brief An AVL Tree index that stores pointers (void *) provided a comparison 
    function (int (*cmp)(const void *, const void *)).

    DESCRIPTION:

    An AVL tree, named after it's inventors [1], is a self-balancing binary 
    search tree that maintains the following invariants. 

    For every node in the tree:
        (1) node->left->entry < node->entry < node->right->entry;
        (2) | node->left->height - node->right->height | < 2;

    Assuming these are maintained, an AVL tree with n entries will have a 
    height of O(log n).  Lookups, insertions, and deletions are all O(log n)
    operations since they are limited by the depth of the tree.  If an 
    insertion or deletion breaks invariant (2), it can only be along the 
    path from the root to the added or removed entry. The work required to 
    reestablish invariant (2) is also limited by the height of the tree. 
    
    This implimentation also maintains a doubly linked list over the entries
    in the tree for quick in-order traversal.
    
    [1] Adelson-Velsky, Georgy; Landis, Evgenii (1962). "An algorithm for the 
        organization of information". Proceedings of the USSR Academy of 
        Sciences (in Russian). 146: 263–266. English translation by Myron 
        J. Ricci in Soviet Mathematics - Doklady, 3:1259–1263, 1962.

    E-mail: erins.ransom@gmail.com

    LICENSE:
    Copyright (c) 2025 Erin Ransom

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#ifndef ERINS_C_LIB_AVL_TREE_H_
#define ERINS_C_LIB_AVL_TREE_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* 
    A AVL Tree index that stores pointers (void *) and  also maintains a 
    doubly linked list for iteration through entries. 
*/
typedef struct _avl_tree avl_tree;

/*
    An Iterator for accessing pointers stored in an AVL Tree.
*/
typedef struct _avl_iter avl_iter;

/*
    Initializes a new AVL Tree that will use the provided comparison 
    function:
    
        int (*cmp)(const void *ptr1, const void *ptr2)
    
    DESCRIPTION

        The cmp() function compares the data referenced by two pointers, 
        ptr1 and ptr2.

        cmp() returns an interger indicating the result of the comparison 
        as follows:

        - 0, if ptr1 and ptr2 have the same key;
        - a negative value if ptr1 has a key less than that of ptr2;
        - a positive value if ptr1 has a key greater than that of ptr2.
*/
avl_tree *new_avl_tree(int (*cmp)(const void *, const void *));

/*
    Returns the pointer (void *) stored in <tree> that matches <target>
    according to the tree's cmp function.  Returns NULL if no such 
    pointer is found. 
*/
void *avl_get(avl_tree *tree, void *target);

/*
    Adds an entry for <data> to <tree>, maintaining that for all nodes in 
    <tree>:
    - node->left->data < node->data < node->right;
    - node->height = max(node->left->height, node->right->height) + 1;
    - node->prev->data < node->data < node->next->data.  
    
    If an entry matching <data> already exists, in <tree>, its contents are 
    overwritten with <data>.  

    Returns an unsigned integer (size_t) indicating the number of entries
    stored in the tree after this operation. 
*/
size_t avl_put(avl_tree *tree, void *data);

/*
    Removes the entry in <tree> that matches <target>, if one exists, 
    maintaining that for each node in <tree>:
    - node->left->data < node->data < node->right;
    - node->height = max(node->left->height, node->right->height) + 1;
    - node->prev->data < node->data < node->next->data. 

    Returns an unsigned integer (size_t) indicating the number of entries
    stored in the tree after this operation. 
*/
size_t avl_delete(avl_tree *tree, void *target);

/*
    Returns an unsigned integer (size_t) indicating the number of entries
    stored in <tree>.
*/
size_t avl_count(avl_tree *tree);

/*
    Returns an unsigned integer (size_t) indicating the current height of 
    <tree>.
*/
size_t avl_height(avl_tree *tree);

/*
    Initializes and returns a pointer to a new iterator for <tree>.  The 
    iterator will be initialized pointing to the first entry stored in 
    <tree>.
*/
avl_iter *new_avl_iter(avl_tree *tree);

/*
    Moves <iter> to the entry that matches <target>, if it exists.  The 
    current entry will be set to NULL if no match is found. 
*/
void avl_iter_seek(avl_iter *iter, void *target);

/*
    Moves <iter> to its first entry.
*/
void avl_iter_seek_start(avl_iter *iter);

/*
    Moves <iter> to its last entry.
*/
void avl_iter_seek_end(avl_iter *iter);

/*
    Advances <iter> to its next entry. The current entry will be set to 
    NULL if <iter> is currently set to its last entry. 
*/
void avl_iter_next(avl_iter *iter);

/*
    Moves <iter> to its previous entry. The current entry will be set to 
    NULL if <iter> is currently set to its first entry. 
*/
void avl_iter_prev(avl_iter *iter);

/*
    Returns the data pointer (void *) for the current entry of <iter>. The 
    return value will be NULL if <iter> has been set a non-existing entry. 
*/
void *avl_iter_data(avl_iter *iter);

/* 
    Runs a check on <tree> asserting that for every node in <tree>:
    - node->left->data < node->data < node->right;
    - node->height = max(node->left->height, node->right->height) + 1;
    - node->prev->data < node->data < node->next->data. 
*/
void avl_validate(avl_tree *tree);

/*
    Prints each entry in <tree> in order using <to_string>.
*/
void print_avl_tree(avl_tree *tree, void (*to_string)(void *data, FILE *out));

#endif