#define DTL_NAMESPACE_BEGIN namespace dtl {
#define DTL_NAMESPACE_END };
#define DTL ::dtl

#define DTL_STR_IMPL(x) #x
#define DTL_STR(x) DTL_IMPL(x)

#define DTL_EXPORT

#if __cplusplus >= 201703L
#define DTL_CPP17
#endif

#ifdef DTL_CPP17
#define DTL_EXPLICIT_FALSE explicit(false)
#else
#define DTL_EXPLICIT_FALSE
#endif


