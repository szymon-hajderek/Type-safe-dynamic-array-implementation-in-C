#include "vector.h"

typedef struct range {
  int start, end;
} range;

v_pod(int) // pb_vint takes argument of type int by value
v_pod_large(range) // pb_vrange takes argument of type range by pointer
v(vrange)
v(vvrange)

int main() {
  // empty vector - no heap allocation performed
  {
    vrange vec = make_vrange();
  }
  
  // 2D vector of empty vectors of ranges - dimensions = (10, 20, 0).
  vvvrange range_vec_3D = init_vvvrange(10, init_vvrange(20, make_vrange()));

  // populating one of the empty vectors with some ranges
  for(size_t i = 0; i < 10; i++) {
    pb_vrange(
      &range_vec_3D.d[4].d[17], &(range) { i, i + 10 }
    );
  }

  // resize without initializing elements
  resize_vrange(&range_vec_3D.d[2].d[10], 10);

  // alternatives
  {
    range rng = {
      .start = 1000000 * (int64_t) 1000000,
      .end = 2 * 1000000 * (int64_t) 1000000 
    };
    pb_vrange(
      &range_vec_3D.d[4].d[17], &rng
    );
  }
  pb_vrange(
    &range_vec_3D.d[4].d[17],
    &(range) { 94283, 32470234 }
  );

  vvvrange copied_vector = deepcopy_vvvrange(&range_vec_3D); // deep copying
  deepfree_vvvrange(&copied_vector); // but we don't really need the copy (;

  // preallocate memory to make the pb_vrange even more efficient
  vrange vec = make_vrange();
  alloc_vrange(&vec, 100);

  for(size_t i = 0; i < 100; i++) {
    pb_vrange(&vec, &(range){ i, i + 5 }); // no reallocation in this loop
  }

}