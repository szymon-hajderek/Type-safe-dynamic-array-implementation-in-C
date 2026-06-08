// This test is intended to be run with valgrind / address sanitizer
// to check for memory leaks and errors.

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "vector.h"

// The first test tests the functionality of the vector for holding a graph.
// This perfectly captures a scenerio of needing to hold a vector of non_pod type.

struct Node;
typedef struct Node* NodePtr;

// The vector of NodePtrs.
v_pod(NodePtr)

// Below is the Node struct and the implementations of its required methods.
typedef struct Node {
  int color;
  bool visited;
  vNodePtr neighbours;
} Node;

Node deepcopy_Node(Node const* node) {
  Node res;
  res = *node;
  res.neighbours = deepcopy_vNodePtr(&node->neighbours);
  return res;
}

void deepfree_Node(Node* node) {
  deepfree_vNodePtr(&node->neighbours);
}

void nullify_Node(Node* node) {
  node->color = node->visited = 0;
  nullify_vNodePtr(&node->neighbours);
}

// The vector of nodes - graph.
v(Node)

void test_v_complex_dfs(NodePtr node) {
  if (node->visited) {
    return;
  }
  node->visited = 1;
  for (size_t i = 0; i < node->neighbours.size; i++) {
    NodePtr neighbour = node->neighbours.d[i];
    
    if (!neighbour->visited) {
      neighbour->color = (node->color ^ 1);
    }
    test_v_complex_dfs(neighbour);
  }
}

// The actual test - creating a tree and coloring it.
void test_v_complex() {
  const size_t n = 100;
  vNode graph = init_vNode(n, (Node) { 0, 0, make_vNodePtr() });

  for (size_t i = 1; i < n; i++) {
    int parent = rand() % i;
    pb_vNodePtr(&graph.d[i].neighbours, &graph.d[parent]);
    pb_vNodePtr(&graph.d[parent].neighbours, &graph.d[i]);
  }

  test_v_complex_dfs(&graph.d[0]);
  
  // Verify DFS reached everything and colors alternate
  for (size_t i = 0; i < n; i++) {
    assert(graph.d[i].visited == 1);
    for (size_t j = 0; j < graph.d[i].neighbours.size; j++) {
        NodePtr neighbour = graph.d[i].neighbours.d[j];
        assert(graph.d[i].color != neighbour->color);
    }
  }

  deepfree_vNode(&graph);
}

v_pod(int)
v(vint)
v(vvint)
v(vvvint) // Needed for 4D deepcopy test

// 3D max sum path from [0, 0, 0] to [n - 1, m - 1, k - 1]
void test_dp_3d() {
  int n = 50, m = 100, o = 150;
  
  vvvint values = init_vvvint(n, init_vvint(m, init_vint(o, 0)));
  
  // Fill the random values in each node
  srand(2137);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      for (int k = 0; k < o; k++) {
        values.d[i].d[j].d[k] = rand() % 100;
      }
    }
  }

  // Calculate 3D max path - we do not care about the result much,
  // But rather about dereferencing the arrays and deallocating them later.
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      for (int k = 0; k < o; k++) {
        int max_prev = 0;
        if (i > 0 && values.d[i-1].d[j].d[k] > max_prev) max_prev = values.d[i-1].d[j].d[k];
        if (j > 0 && values.d[i].d[j-1].d[k] > max_prev) max_prev = values.d[i].d[j-1].d[k];
        if (k > 0 && values.d[i].d[j].d[k-1] > max_prev) max_prev = values.d[i].d[j].d[k-1];
        
        if (i > 0 || j > 0 || k > 0) {
          values.d[i].d[j].d[k] += max_prev;
        }
      }
    }
  }

  deepfree_vvvint(&values);
}

void test_resize() {
  // 1. Start with 5 nodes, each having a dynamically allocated neighbours vector
  vNode graph = init_vNode(5, (Node) { 1, 0, init_vNodePtr(10, NULL) });
  assert(graph.size == 5);
  
  // 2. Grow to 10
  resize_vNode(&graph, 10);
  assert(graph.size == 10);

  assert(graph.d[5].neighbours.size == 0);
  assert(graph.d[5].neighbours.capacity == 0);
  assert(graph.d[5].neighbours.d == NULL);

  // Indices 3 and 4 will have their actual heap memory cleanly freed.
  // Indices 5 through 9 will safely no-op during deepfree because they were nullified.
  resize_vNode(&graph, 3);
  assert(graph.size == 3);

  deepfree_vNode(&graph);
}

void test_alloc() {
  vNode graph = make_vNode();
  alloc_vNode(&graph, 10);
  assert(graph.capacity == 10);

  resize_vNode(&graph, 5);
  
  // Should be nullified by resize, this should be correct and no-op.
  for (size_t i = 0; i < 5; i++) {
    deepfree_Node(&graph.d[i]);
    deepfree_Node(&graph.d[i]);
  }

  for (int i = 0; i < 5; i++) {
    pb_move_vNode(&graph, &(Node){ 0, 0, init_vNodePtr(10, NULL) });
  }

  // Resize to 5 + 5 pushes <-> 10 capacity needed.
  assert(graph.capacity == 10);

  alloc_vNode(&graph, 2);

  // Should shrink the size and deefree the elements.
  assert(graph.size == 2);

  alloc_vNode(&graph, 1);
  alloc_vNode(&graph, 20);
  alloc_vNode(&graph, 3);

  assert(graph.size == 1);
  alloc_vNode(&graph, 100);

  deepfree_vNode(&graph);
}

void test_deepcopy_4d_and_multi_deepfree() {
  size_t s = 5;
  vvvvint t1 = init_vvvvint(s, init_vvvint(s, init_vvint(s, init_vint(s, 42))));
  vvvvint t2 = deepcopy_vvvvint(&t1);
  
  assert(t1.size == s);
  assert(t2.size == s);
  assert(t2.d[s-1].d[s-1].d[s-1].d[s-1] == 42);

  for (size_t a = 0; a < s; a++) {
    for (size_t b = 0; b < s; b++) {
      for (size_t c = 0; c < s; c++) {
        for (size_t d = 0; d < s; d++) {
          assert(t1.d[a].d[b].d[c].d[d] == t2.d[a].d[b].d[c].d[d]);
        }
      }
    }
  }
  
  // Modify t1 to ensure t2 is entirely disconnected
  t1.d[0].d[0].d[0].d[0] = 99;
  assert(t2.d[0].d[0].d[0].d[0] == 42);
  
  deepfree_vvvvint(&t1);
  deepfree_vvvvint(&t2);
  deepfree_vvvvint(&t2);
  deepfree_vvvvint(&t2);
}

void test_move() {
  vNode graph = make_vNode();
  
  // Create a node with allocated heap memory
  Node n1 = { 1, 0, init_vNodePtr(5, NULL) };
  assert(n1.neighbours.capacity == 5);
  
  // Move it into the graph
  pb_move_vNode(&graph, &n1);

  deepfree_Node(&n1); // Run a multi-free on the moved n1.
  deepfree_Node(&n1); // Should work as expected, as n1 was nullfied, meaning it is safe to call deepfree on.
  deepfree_Node(&n1);

  assert(graph.size == 1);
  assert(graph.d[0].color == 1);
  assert(graph.d[0].neighbours.capacity == 5);
  
  assert(n1.neighbours.size == 0);
  assert(n1.neighbours.capacity == 0);
  assert(n1.neighbours.d == NULL);
  
  deepfree_vNode(&graph);
}

struct Test {
  char const* name;
  void (* test_fun)(void);
};

const struct Test tests[] = {
  { "test_alloc", test_alloc },
  { "test_resize", test_resize },
  { "test_move", test_move },
  { "test_v_complex", test_v_complex },
  { "test_dp_3d", test_dp_3d },
  { "test_deepcopy_4d_and_multi_deepfree", test_deepcopy_4d_and_multi_deepfree },
};

const int NUM_TESTS = sizeof(tests) / sizeof(struct Test);

int main() {
  for (int i = 0; i < NUM_TESTS; i++) {
    printf("----- Running test %s [%d/%d] -----\n", tests[i].name, (i + 1), NUM_TESTS);
    tests[i].test_fun();
  }
  printf("----- All assertions passed! -----\n");
  return 0;
}