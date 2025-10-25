# Type-safe multidimensional dynamic array in C

This header-only library provides macros for automated definition of typed dynamic arrays (vectors) in C.

The library is focused on providing the friendliest usage possible, while maintaining near native-array efficiency in all operations.

# Features

The vector implements:

- automatic `deep copy` functionality (to make copying vectors trivial)
- automatic `deep free` functionality (to avoid memory leaks)
- specialized implementation of vector for small POD types, large POD types and non-POD types.
- chained initialization of multidimensional vectors (just like `std::vector`'s constructor)

# Usage

`v_pod(T)` - Creates a vector type `vT` of copied by value POD typed elements.
`v_pod_large(T)` - Creates a vector type `vT` of POD typed elements, which should be passed by pointer due to their large size.
`v(T)` - Creates a vector type `vT` of complex, non-POD typed elements. Assumes existance of `deep_copy_T` and `deep_free_T`.

Each macro defines functions `deep_copy_vT` and `deep_free_vT`.

File `usage.c`:

```cc
#include "vector.h"

typedef struct range {
  int64_t start, end;
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

  // do not use my_vec here, as it's already owned my range_vec_3D!

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
  // deepfree function brings vector back to clean state (as after calling make)

  // preallocate memory to make the pb_vrange even more efficient
  vrange vec = make_vrange();
  alloc_vrange(&vec, 100);

  for(size_t i = 0; i < 100; i++) {
    pb_vrange(&vec, &(range){ i, i + 5 }); // no reallocation in this loop
  }

  // WARNING! As stated in the header, init function takes ownership over non-POD typed val.
  // This means that val passed that way should not be used in any way by the user afterwards.
  {
    vvrange my_vec = init_vvrange(20, make_vrange());
    vvvrange owner_of_my_vec = init_vvvrange(10, my_vec);

    // frees owner_of_my_vec, including my_vec
    deepfree_vvvrange(&owner_of_my_vec);
  }

  // in order to enable future use of my_vec, one should do it as follows:
  {
    vvrange my_vec = init_vvrange(20, make_vrange());
    vvvrange not_owner_of_my_vec = init_vvvrange(10, deepcopy_vvrange(&my_vec));
    // my_vec can still safely be used here, without anyhow affecting not_owner_of_my_vec and vice versa.
    // ...
    // frees not_owner_of_my_vec, but does not free my_vec
    deepfree_vvvrange(&not_owner_of_my_vec);
  }

}
```