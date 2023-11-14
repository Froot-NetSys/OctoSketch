#ifndef ATOMICOPS_H
#define ATOMICOPS_H

// ©2013-2016 Cameron Desrochers.
// Distributed under the simplified BSD license (see the license file that
// should have come with this header).
// Uses Jeff Preshing's semaphore implementation (under the terms of its
// separate zlib license, embedded below).

#pragma once

#include <cerrno>
#include <cassert>
#include <type_traits>
#include <cerrno>
#include <cstdint>
#include <ctime>


// AE_UNUSED
#define AE_UNUSED(x) ((void)x)

// AE_NO_TSAN/AE_TSAN_ANNOTATE_*
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#if __cplusplus >= 201703L  // inline variables require C++17
namespace moodycamel { inline int ae_tsan_global; }
#define AE_TSAN_ANNOTATE_RELEASE() AnnotateHappensBefore(__FILE__, __LINE__, (void *)(&::moodycamel::ae_tsan_global))
#define AE_TSAN_ANNOTATE_ACQUIRE() AnnotateHappensAfter(__FILE__, __LINE__, (void *)(&::moodycamel::ae_tsan_global))
extern "C" void AnnotateHappensBefore(const char*, int, void*);
extern "C" void AnnotateHappensAfter(const char*, int, void*);
#else  // when we can't work with tsan, attempt to disable its warnings
#define AE_NO_TSAN __attribute__((no_sanitize("thread")))
#endif
#endif
#endif
#ifndef AE_NO_TSAN
#define AE_NO_TSAN
#endif
#ifndef AE_TSAN_ANNOTATE_RELEASE
#define AE_TSAN_ANNOTATE_RELEASE()
#define AE_TSAN_ANNOTATE_ACQUIRE()
#endif



#define AE_FORCEINLINE inline
#define AE_ALIGN(x) __attribute__((aligned(x)))


#include <atomic>
#include <utility>

// WARNING: *NOT* A REPLACEMENT FOR std::atomic. READ CAREFULLY:
// Provides basic support for atomic variables -- no memory ordering guarantees are provided.
// The guarantee of atomicity is only made for types that already have atomic load and store guarantees
// at the hardware level -- on most platforms this generally means aligned pointers and integers (only).

// Portable atomic fences implemented below:

enum memory_order {
    memory_order_relaxed,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst,

    // memory_order_sync: Forces a full sync:
    // #LoadLoad, #LoadStore, #StoreStore, and most significantly, #StoreLoad
            memory_order_sync = memory_order_seq_cst
};

AE_FORCEINLINE void compiler_fence(memory_order order) AE_NO_TSAN
{
    switch (order) {
        case memory_order_relaxed: break;
        case memory_order_acquire: std::atomic_signal_fence(std::memory_order_acquire); break;
        case memory_order_release: std::atomic_signal_fence(std::memory_order_release); break;
        case memory_order_acq_rel: std::atomic_signal_fence(std::memory_order_acq_rel); break;
        case memory_order_seq_cst: std::atomic_signal_fence(std::memory_order_seq_cst); break;
        default: assert(false);
    }
}

AE_FORCEINLINE void fence(memory_order order) AE_NO_TSAN
{
    switch (order) {
        case memory_order_relaxed: break;
        case memory_order_acquire: AE_TSAN_ANNOTATE_ACQUIRE(); std::atomic_thread_fence(std::memory_order_acquire); break;
        case memory_order_release: AE_TSAN_ANNOTATE_RELEASE(); std::atomic_thread_fence(std::memory_order_release); break;
        case memory_order_acq_rel: AE_TSAN_ANNOTATE_ACQUIRE(); AE_TSAN_ANNOTATE_RELEASE(); std::atomic_thread_fence(std::memory_order_acq_rel); break;
        case memory_order_seq_cst: AE_TSAN_ANNOTATE_ACQUIRE(); AE_TSAN_ANNOTATE_RELEASE(); std::atomic_thread_fence(std::memory_order_seq_cst); break;
        default: assert(false);
    }
}

template<typename T>
class weak_atomic
{
public:
    AE_NO_TSAN weak_atomic() : value() { }

    template<typename U> AE_NO_TSAN weak_atomic(U&& x) : value(std::forward<U>(x)) {  }

    AE_NO_TSAN weak_atomic(weak_atomic const& other) : value(other.load()) {  }
    AE_NO_TSAN weak_atomic(weak_atomic&& other) : value(std::move(other.load())) {  }

    AE_FORCEINLINE operator T() const AE_NO_TSAN { return load(); }

    template<typename U>
    AE_FORCEINLINE weak_atomic const& operator=(U&& x) AE_NO_TSAN
    {
        value.store(std::forward<U>(x), std::memory_order_relaxed);
        return *this;
    }

    AE_FORCEINLINE weak_atomic const& operator=(weak_atomic const& other) AE_NO_TSAN
    {
        value.store(other.value.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }

    AE_FORCEINLINE T load() const AE_NO_TSAN { return value.load(std::memory_order_relaxed); }

private:
    std::atomic<T> value;
};

#endif

