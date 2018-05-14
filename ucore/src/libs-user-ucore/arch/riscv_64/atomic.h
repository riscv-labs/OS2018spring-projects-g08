#ifndef __ARCH_UM_INCLUDE_ATOMIC_H
#define __ARCH_UM_INCLUDE_ATOMIC_H

#include <types.h>

/* Atomic operations that C can't guarantee us. Useful for resource counting
 * etc.. */

typedef struct {
	volatile int counter;
} atomic_t;

static inline int atomic_read(const atomic_t * v)
    __attribute__ ((always_inline));
static inline void atomic_set(atomic_t * v, int i)
    __attribute__ ((always_inline));
static inline void atomic_add(atomic_t * v, int i)
    __attribute__ ((always_inline));
static inline void atomic_sub(atomic_t * v, int i)
    __attribute__ ((always_inline));
static inline void atomic_inc(atomic_t * v) __attribute__ ((always_inline));
static inline void atomic_dec(atomic_t * v) __attribute__ ((always_inline));
static inline bool atomic_inc_test_zero(atomic_t * v)
    __attribute__ ((always_inline));
static inline bool atomic_dec_test_zero(atomic_t * v)
    __attribute__ ((always_inline));
static inline int atomic_add_return(atomic_t * v, int i)
    __attribute__ ((always_inline));
static inline int atomic_sub_return(atomic_t * v, int i)
    __attribute__ ((always_inline));


//TODO: risc-v

/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */

#define ATOMIC_INIT(i)	{ (i) }

/**
 * atomic_read - read atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically reads the value of @v.
 */
static inline int atomic_read(const atomic_t *v)
{
    //TODO: use gcc __atomic_load

	return (*(volatile int *)&(v)->counter);
}

/**
 * atomic_set - set atomic variable
 * @v: pointer of type atomic_t
 * @i: required value
 *
 * Atomically sets the value of @v to @i.
 */
static inline void atomic_set(atomic_t *v, int i)
{
    __atomic_store((volatile int *)&(v->counter), &i, __ATOMIC_SEQ_CST);
}

/**
 * atomic_add - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v.
 */
static inline void atomic_add(atomic_t *v, int i)
{
    __atomic_add_fetch((volatile int *)&(v->counter), i, __ATOMIC_SEQ_CST);
}

/**
 * atomic_sub - subtract integer from atomic variable
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v.
 */
static inline void atomic_sub(atomic_t *v, int i)
{
    __atomic_sub_fetch((volatile int *)&(v->counter), i, __ATOMIC_SEQ_CST);
}


/**
 * atomic_sub_and_test - subtract value from variable and test result
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic_sub_and_test(int i, atomic_t *v)
{
    int ret = __atomic_sub_fetch((volatile int *)&(v->counter), i, __ATOMIC_SEQ_CST);
    if (ret) return 0;
    else return 1;
}


/**
 * atomic_inc - increment atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1.
 */
static inline void atomic_inc(atomic_t *v)
{
	atomic_add(v, 1);
}


/**
 * atomic_dec - decrement atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1.
 */
static inline void atomic_dec(atomic_t *v)
{
	atomic_sub(v, 1);
}


/**
 * atomic_dec_and_test - decrement and test
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */
static inline bool atomic_dec_test_zero(atomic_t * v)
{
    return atomic_sub_and_test(1, v);
}

/**
 * atomic_inc_and_test - increment and test
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
static inline bool atomic_inc_test_zero(atomic_t * v)
{
    int ret = __atomic_add_fetch((volatile int *)&(v->counter), 1, __ATOMIC_SEQ_CST);
    if (ret) return 0;
    else return 1;
}

/* *
 * atomic_add_return - add integer and return
 * @i:  integer value to add
 * @v:  pointer of type atomic_t
 *
 * Atomically adds @i to @v and returns @i + @v
 * Requires Modern 486+ processor
 * */
static inline int atomic_add_return(atomic_t * v, int i)
{
	return __atomic_add_fetch((volatile int *)&(v->counter), i, __ATOMIC_SEQ_CST);
}

/* *
 * atomic_sub_return - subtract integer and return
 * @v:  pointer of type atomic_t
 * @i:  integer value to subtract
 *
 * Atomically subtracts @i from @v and returns @v - @i
 * */
static inline int atomic_sub_return(atomic_t * v, int i)
{
	return __atomic_sub_fetch((volatile int *)&(v->counter), i, __ATOMIC_SEQ_CST);
}

static inline int atomic_cmpxchg(atomic_t *v, int old, int new)
{
	// return cmpxchg(&v->counter, old, new);
    panic("Not implemented!\n");
    return 0;
}

static inline int atomic_xchg(atomic_t *v, int new)
{
	// return xchg(&v->counter, new);
    panic("Not implemented!\n");
    return 0;
}

#define atomic_compare_and_swap(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)



//TODO bit op on MP
static inline void set_bit(int nr, volatile void *addr)
    __attribute__((always_inline));
static inline void clear_bit(int nr, volatile void *addr)
    __attribute__((always_inline));
static inline void change_bit(int nr, volatile void *addr)
    __attribute__((always_inline));
static inline bool test_bit(int nr, volatile void *addr)
    __attribute__((always_inline));
static inline bool test_and_set_bit(int nr, volatile void *addr)
    __attribute__((always_inline));
static inline bool test_and_clear_bit(int nr, volatile void *addr)
    __attribute__((always_inline));

#define BITS_PER_LONG __riscv_xlen

#if (BITS_PER_LONG == 64)
#define __AMO(op) "amo" #op ".d"
#elif (BITS_PER_LONG == 32)
#define __AMO(op) "amo" #op ".w"
#else
#error "Unexpected BITS_PER_LONG"
#endif

#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)

#define __test_and_op_bit(op, mod, nr, addr)                         \
    ({                                                               \
        unsigned long __res, __mask;                                 \
        __mask = BIT_MASK(nr);                                       \
        __asm__ __volatile__(__AMO(op) " %0, %2, %1"                 \
                             : "=r"(__res), "+A"(addr[BIT_WORD(nr)]) \
                             : "r"(mod(__mask)));                    \
        ((__res & __mask) != 0);                                     \
    })

#define __op_bit(op, mod, nr, addr)                 \
    __asm__ __volatile__(__AMO(op) " zero, %1, %0"  \
                         : "+A"(addr[BIT_WORD(nr)]) \
                         : "r"(mod(BIT_MASK(nr))))

/* Bitmask modifiers */
#define __NOP(x) (x)
#define __NOT(x) (~(x))

/* *
 * set_bit - Atomically set a bit in memory
 * @nr:     the bit to set
 * @addr:   the address to start counting from
 *
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 * */
static inline void set_bit(int nr, volatile void *addr) {
    __op_bit(or, __NOP, nr, ((volatile unsigned long *)addr));
}

/* *
 * clear_bit - Atomically clears a bit in memory
 * @nr:     the bit to clear
 * @addr:   the address to start counting from
 * */
static inline void clear_bit(int nr, volatile void *addr) {
    __op_bit(and, __NOT, nr, ((volatile unsigned long *)addr));
}

/* *
 * change_bit - Atomically toggle a bit in memory
 * @nr:     the bit to change
 * @addr:   the address to start counting from
 * */
static inline void change_bit(int nr, volatile void *addr) {
    __op_bit (xor, __NOP, nr, ((volatile unsigned long *)addr));
}

/* *
 * test_bit - Determine whether a bit is set
 * @nr:     the bit to test
 * @addr:   the address to count from
 * */
static inline bool test_bit(int nr, volatile void *addr) {
    return (((*(volatile unsigned long *)addr) >> nr) & 1);
}

/* *
 * test_and_set_bit - Atomically set a bit and return its old value
 * @nr:     the bit to set
 * @addr:   the address to count from
 * */
static inline bool test_and_set_bit(int nr, volatile void *addr) {
    return __test_and_op_bit(or, __NOP, nr, ((volatile unsigned long *)addr));
}

/* *
 * test_and_clear_bit - Atomically clear a bit and return its old value
 * @nr:     the bit to clear
 * @addr:   the address to count from
 * */
static inline bool test_and_clear_bit(int nr, volatile void *addr) {
    return __test_and_op_bit(and, __NOT, nr, ((volatile unsigned long *)addr));
}

#endif /* !__ARCH_UM_INCLUDE_ATOMIC_H */
