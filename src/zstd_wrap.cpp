#include "zstd_wrap.hpp"
#if defined(ACX_HAVE_ZSTD)
#include <zstd.h>
#endif

namespace acx {

bool zstd_available(){
#if defined(ACX_HAVE_ZSTD)
    return true;
#else
    return false;
#endif
}

bool zstd_compress(const std::string& in, std::string& out){
#if defined(ACX_HAVE_ZSTD)
    size_t bound = ZSTD_compressBound(in.size());
    out.resize(bound);
    size_t n = ZSTD_compress(out.data(), bound, in.data(), in.size(), 3);
    if (ZSTD_isError(n)) return false;
    out.resize(n);
    return true;
#else
    (void)in; (void)out; return false;
#endif
}
bool zstd_decompress(const std::string& in, std::string& out){
#if defined(ACX_HAVE_ZSTD)
    unsigned long long sz = ZSTD_getFrameContentSize(in.data(), in.size());
    if (sz == ZSTD_CONTENTSIZE_UNKNOWN || sz == ZSTD_CONTENTSIZE_ERROR) return false;
    out.resize((size_t)sz);
    size_t n = ZSTD_decompress(out.data(), out.size(), in.data(), in.size());
    if (ZSTD_isError(n)) return false;
    return true;
#else
    (void)in; (void)out; return false;
#endif
}

} // namespace acx
