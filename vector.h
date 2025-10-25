#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/*
  Documentation:
  v_pod(T):
    Declares type _v##T typedeffed with VEC_TYPE_NAME = v##T. This macro should be used for declaring vectors of POD types smaller than 8 bytes. This macro also declares simple automatic copy and free functions called pod_copy_##T and pod_free_##T (which is a no-op function). With this macro, PB_ARG_TYPE = (const T). That means pb_v##T takes val by value.
  v_pod_large(T):
    Declares type _v##T typedeffed with VEC_TYPE_NAME = v##T. This macro should be used for declaring vectors of POD types bigger than 8 bytes. This macro also declares simple automatic copy and free functions called pod_copy_##T and pod_free_##T (which is a no-op function).  With this macro, PB_ARG_TYPE = (const T*). That means pb_v##T takes val through a pointer. This is to ensure no overhead related to the need of copying large POD type.
  v(T):
    Declares type _v##T typedeffed with VEC_TYPE_NAME = v##T. This macro is intended to be used with non-POD types. It requires that deepcopy_##T and deepfree_##T functions are implemented. With this macro, PB_ARG_TYPE = (const T*). That means pb_v##T takes val through a pointer. This is to ensure no overhead related to the need of copying large non-POD type.
    
  Vector's features:
    void pb_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, T_ARG_TYPE val):
        pb is an abbreviation for push back. Appends val to the  end of vec. The operation performs in amortized constant time.
      void alloc_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, size_t n):
        Allocates / reallocates vec->d to at least n * sizeof(T) bytes. vec's content is copied to the new location. If n is less than current vec->size, vec->size is set to n and remainging elements are discarded.
      void resize_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, size_t n):
        Equivalent to calling alloc_##VEC_TYPE_NAME(vec, n) and setting vec->size to n.
      T* ref_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, size_t ind):
        Returns pointer to vec.d[ind]
      T get_##VEC_TYPE_NAME(const VEC_TYPE_NAME* vec, size_t ind):
        Returns vec.d[ind]
      VEC_TYPE_NAME make_##VEC_TYPE_NAME():
        Returns a clean, initialized vector
      VEC_TYPE_NAME deepcopy_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec):
        Performs a deep copy of vector vec.
      void deepfree_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec):
        Performs a deep freeing of the vector's memory. It also zeros all members.
      VEC_TYPE_NAME init_##VEC_TYPE_NAME(size_t n, INIT_ARG_TYPE val):
        Returns a vector, whose first element is a shallow copy of val and other elements are deep copies of val. That means ownership over non-POD val is transferred when calling this function. For a non-POD type, If n == 0, val is freed. This allows convienient chaining without any unnecessary deep copying:

        v_pod(int)
        v(vint)

        If you don't mind passing ownership over my_template, you should use it as follows:
        vint my_template = init_vint(10, 4);
        vvint vec_2D = init_vvint(20, my_template);

        or simply:
        vvint vec_2D = init_vvint(20, init_vint(10, 4));

        If you don't want to transfer ownership over my_template, you should use it as follows:
        vint my_template = init_vint(10, 4);
        vvint vec_2D = init_vvint(20, deep_copy_vint(&my_template));


    members:
      size_t capacity:
        number of maximum entries this vector can contain before a need to reallocate memory.
      size_t size:
        number of entries the vector contains.
      T* d:
        Abbreviation for "data". Array with vector's entries.
*/
#define val(T, ...) &((const T) { __VA_ARGS__ })
#define pod_deepcopy_fun_factory(T) T pod_copy_##T(const T* val) { return *val; }
#define pod_deepfree_fun_factory(T) void pod_free_##T(T* val) { (void) val; }
#define v_pod(T) \
  pod_deepcopy_fun_factory(T) \
  pod_deepfree_fun_factory(T) \
  v_base(T, v##T, pod_copy_##T, pod_free_##T, const T, &)
#define v_pod_large(T) \
  pod_deepcopy_fun_factory(T) \
  pod_deepfree_fun_factory(T) \
  v_base(T, v##T, pod_copy_##T, pod_free_##T, const T*, )
#define v(T) v_base(T, v##T, deepcopy_##T, deepfree_##T, const T*,)
#define v_base(T, VEC_TYPE_NAME, DEEPCOPY_FUN, DEEPFREE_FUN, T_ARG_TYPE, T_ARG_ACCESS_OP) \
  typedef struct _v##T { \
    size_t size, capacity; \
    T* d; \
  } VEC_TYPE_NAME; \
  void alloc_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, size_t n) { \
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
  void pb_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, T_ARG_TYPE val) { \
    if(vec->size >= vec->capacity) { \
      alloc_##VEC_TYPE_NAME(vec, vec->capacity ? 2 * vec->capacity : 1); \
    } \
    vec->d[vec->size++] = DEEPCOPY_FUN(T_ARG_ACCESS_OP val); \
  } \
  void resize_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, size_t n) { \
    alloc_##VEC_TYPE_NAME(vec, n); \
    vec->size = n; \
  } \
  T* ref_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec, size_t ind) { \
    return &vec->d[ind]; \
  } \
  T get_##VEC_TYPE_NAME(const VEC_TYPE_NAME* vec, size_t ind) { \
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
    for(size_t i = 0; i < vec->size; i++) { \
      res.d[i] = DEEPCOPY_FUN(vec->d + i); \
    } \
    return res; \
  } \
  VEC_TYPE_NAME init_##VEC_TYPE_NAME(size_t n, T val) { \
    VEC_TYPE_NAME res = make_##VEC_TYPE_NAME(); \
    if(n == 0) { \
      DEEPFREE_FUN(&val); \
      return res; \
    } \
    resize_##VEC_TYPE_NAME(&res, n); \
    res.d[0] = val; \
    for(size_t i = 1; i < n; i++) { \
      res.d[i] = DEEPCOPY_FUN(&val); \
    } \
    return res; \
  } \
  void deepfree_##VEC_TYPE_NAME(VEC_TYPE_NAME* vec) { \
    for(size_t i = 0; i < vec->size; i++) { \
      DEEPFREE_FUN(vec->d + i); \
    } \
    free(vec->d); \
    vec->size = vec->capacity = 0; \
    vec->d = NULL; \
  }

#endif