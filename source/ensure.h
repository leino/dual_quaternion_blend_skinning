#if defined(ENSURE_DEBUGBREAK_ON_ERROR)
#define ENSURE(expr) if(!(expr)){__debugbreak();}
#else
#define ENSURE(expr)
#endif

#define ENSURE_EQUIVALENT(a, b) ENSURE((a) == (b));

#define ENSURE_STATIC(expr) static_assert(expr, "failed to ensure static invariant");
