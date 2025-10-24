#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/*
  Documentation:
  v_simple(T):
    Declares type _v##T typedeffed with VEC_TYPE_NAME = v##T. This macro should be used for declaring vectors of simple types. This macro also declares simple automatic copy and free functions called simple_copy_##T and simple_free_##T.
  v(T):
    Declares type _v##T typedeffed with VEC_TYPE_NAME = v##T. It requires that deepcopy_##T and deepfree_##T functions are implemented.
    
  Vector's features:
    void pb_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, T val):
        pb is an abbreviation for push back. Appends val to the  end of vec. The operation performs in amortized constant time.
      void alloc_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, int n):
        Allocates / reallocates vec->d to at least n * sizeof(T) bytes. vec's content is copied to the new location. If n is less than current vec->size, vec->size is set to n and remainging elements are discarded.
      void resize_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, int n):
        Equivalent to calling alloc_##VEC_TYPE_NAME(vec, n) and setting vec->size to n.
      T* ref_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, int ind):
        Returns pointer to vec.d[ind]
      T get_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, int ind):
        Returns vec.d[ind]
      VEC_TYPE_NAME make_##VEC_TYPE_NAME():
        Returns a clean, initialized vector
      VEC_TYPE_NAME deepcopy_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec):
        Performs a deep copy of vector vec.
      void deepfree_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec):
        Performs a deep freeing of the vector's memory. It also zeros all members.
    members:
      int capacity:
        number of maximum entries this vector can contain before a need to reallocate memory.
      int size:
        number of entries the vector contains.
      T* d:
        Abbreviation for "data". Array with vector's entries.
    NOTE: The structure does not supply deep copy behaviour.
*/
#define simple_deepcopy_fun_factory(T) T simple_copy_##T(const T* val) { return *val; }
#define simple_deepfree_fun_factory(T) void simple_free_##T(T* val) { (void) val; }
#define v_simple(T) \
  simple_deepcopy_fun_factory(T) \
  simple_deepfree_fun_factory(T) \
  v_base(T, v##T, simple_copy_##T, simple_free_##T)
#define v(T) v_base(T, v##T, deepcopy_##T, deepfree_##T)
#define v_named(T, VEC_TYPE_NAME) v_base(T, VEC_TYPE_NAME, deepcopy_##T, deepfree_##T)
#define v_base(T, VEC_TYPE_NAME, DEEPCOPY_FUN, DEEPFREE_FUN) \
  typedef struct _v##T { \
    int size, capacity; \
    T* d; \
  } VEC_TYPE_NAME; \
  void alloc_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, int n) { \
    T* ptr = (T*) realloc(vec->d, sizeof(T) * (size_t) n); \
    if(ptr == NULL) { \
        fprintf(stderr, "Allocation failed in alloc_v"#T "\n"); \
        exit(EXIT_FAILURE); \
    } \
    vec->d = ptr; \
    vec->capacity = n; \
    if(vec->size > n) { \
      vec->size = n; \
    } \
  } \
  void pb_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, const T val) { \
    if(vec->size >= vec->capacity) { \
      alloc_##VEC_TYPE_NAME(vec, vec->capacity ? 2 * vec->capacity : 1); \
    } \
    vec->d[vec->size++] = val; \
  } \
  void resize_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, int n) { \
    alloc_##VEC_TYPE_NAME(vec, n); \
    vec->size = n; \
  } \
  T* ref_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, int ind) { \
    return &vec->d[ind]; \
  } \
  T get_##VEC_TYPE_NAME(const VEC_TYPE_NAME* vec, int ind) { \
    return vec->d[ind]; \
  } \
  VEC_TYPE_NAME make_##VEC_TYPE_NAME() { \
    VEC_TYPE_NAME vec; \
    vec.size = vec.capacity = 0; \
    vec.d = NULL; \
    return vec;  \
  } \
  VEC_TYPE_NAME deepcopy_##VEC_TYPE_NAME(const VEC_TYPE_NAME* vec) { \
    VEC_TYPE_NAME res = make_##VEC_TYPE_NAME(); \
    resize_##VEC_TYPE_NAME(&res, vec->size); \
    for(int i = 0; i < vec->size; i++) { \
      res.d[i] = DEEPCOPY_FUN(vec->d + i); \
    } \
    return res; \
  } \
  void deepfree_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec) { \
    for(int i = 0; i < vec->size; i++) { \
      DEEPFREE_FUN(vec->d + i); \
    } \
    free(vec->d); \
    vec->size = vec->capacity = 0; \
    vec->d = NULL; \
  }

#endif