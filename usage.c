#include "vector.h"

// declares type `vint`
// pb_vint takes argument of type int by value
v_pod(int)

typedef struct range {
  int64_t start, end;
} range;

// declares type `vrange`
// pb_vrange takes argument of type range by pointer
v_pod_large(range)

// declares type `vvrange`
v(vrange)

// declares type `vvvrange`
v(vvrange)

int main() {
  // empty vector - no heap allocation performed
  {
    vrange vec = make_vrange();
  }
  
  // 2D vector of empty vectors of ranges - dimensions = (10, 20, 0).
  vvvrange range_vec_3D = init_vvvrange(10, init_vvrange(20, make_vrange()));

  // do not use my_vec here, as it's already owned my range_vec_3D!

  // populating one of the empty vectors with some ranges
  for(size_t i = 0; i < 10; i++) {
    pb_vrange(
      &range_vec_3D.d[4].d[17], &(range) { i, i + 10 }
    );
  }

  // resize without initializing elements
  resize_vrange(&range_vec_3D.d[2].d[10], 10);

  // alternative ways of appending to the vector
  {
    range rng = {
      .start = 1000000 * (int64_t) 1000000,
      .end = 2 * 1000000 * (int64_t) 1000000 
    };
    pb_vrange(
      &range_vec_3D.d[4].d[17], &rng
    );
    pb_vrange(
      &range_vec_3D.d[4].d[17],
      &(range) { 94283, 32470234 }
    );
  }

  vvvrange copied_vector = deepcopy_vvvrange(&range_vec_3D); // deep copying
  deepfree_vvvrange(&copied_vector); // but we don't really need the copy (;
  // deepfree function brings vector back to clean state (as after calling make)

  // preallocate memory to make the pb_vrange even more efficient
  vrange vec = make_vrange();
  alloc_vrange(&vec, 100);

  for(size_t i = 0; i < 100; i++) {
    pb_vrange(&vec, &(range){ i, i + 5 }); // no reallocation in this loop
  }

  // pb vs pb_move
  {
    vvrange vec_2D = make_vvrange();
    // vec_2D now contains a three deep copies of vec
    for(int i = 0; i < 3; i++) {
      pb_vvrange(&vec_2D, &vec);
    }
    // vec_2D now also contains a shallow copy of vec
    pb_move_vvrange(&vec_2D, &vec);
    // now, (vec == make_vvrange()), as it's been nullified.
    // it can be used safely, but its content has been consumed.
  }

  {
    // WARNING: This is a convenience function that can be used in chained expressions.
    // For non-POD types, make sure that `val` is either a temporary variable not used after this function call, or a constant expression. This is because `val` is shallow-copied into the structure.
    vvrange my_vec = init_vvrange(20, make_vrange());
    vvvrange owner_of_my_vec = init_vvvrange(10, my_vec);

    // frees owner_of_my_vec, including my_vec
    deepfree_vvvrange(&owner_of_my_vec);
    // deepfree_vvrange(&my_vec) <-> this would result in double-free error!
  }

  // If you intend to use `val` only as a template, consider passing deepcopy_T(&val) instead of val. 
  {
    vvrange my_vec = init_vvrange(20, make_vrange());
    vvvrange not_owner_of_my_vec = init_vvvrange(10, deepcopy_vvrange(&my_vec));
    // my_vec can still safely be used here, without anyhow affecting not_owner_of_my_vec and vice versa.
    // ...
    // frees not_owner_of_my_vec, but does not free my_vec
    deepfree_vvvrange(&not_owner_of_my_vec);

    // my_vec needs to be fred separately
    deepfree_vvvrange(&my_vec);
  }

}