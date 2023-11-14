#ifndef READERWRITERQUEUE_H
#define READERWRITERQUEUE_H

// ©2013-2020 Cameron Desrochers.
// Distributed under the simplified BSD license (see the license file that
// should have come with this header).

#pragma once

#define NDEBUG

#include "atomicops.h"
#include <new>
#include <type_traits>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <new>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <chrono>


// A lock-free queue for a single-consumer, single-producer architecture.
// The queue is also wait-free in the common path (except if more memory
// needs to be allocated, in which case malloc is called).
// Allocates memory sparingly, and only once if the original maximum size
// estimate is never exceeded.
// Tested on x86/x64 processors, but semantics should be correct for all
// architectures (given the right implementations in atomicops.h), provided
// that aligned integer and pointer accesses are naturally atomic.
// Note that there should only be one consumer thread and producer thread;
// Switching roles of the threads, or using multiple consecutive threads for
// one role, is not safe unless properly synchronized.
// Using the queue exclusively from one thread is fine, though a bit silly.

#ifndef MOODYCAMEL_CACHE_LINE_SIZE
#define MOODYCAMEL_CACHE_LINE_SIZE 64
#endif

#ifndef MOODYCAMEL_EXCEPTIONS_ENABLED
#if (defined(_MSC_VER) && defined(_CPPUNWIND)) || (defined(__GNUC__) && defined(__EXCEPTIONS)) || (!defined(_MSC_VER) && !defined(__GNUC__))
#define MOODYCAMEL_EXCEPTIONS_ENABLED
#endif
#endif

#ifndef MOODYCAMEL_HAS_EMPLACE
#if !defined(_MSC_VER) || _MSC_VER >= 1800 // variadic templates: either a non-MS compiler or VS >= 2013
#define MOODYCAMEL_HAS_EMPLACE    1
#endif
#endif

#ifndef MOODYCAMEL_MAYBE_ALIGN_TO_CACHELINE
#if defined (__APPLE__) && defined (__MACH__) && __cplusplus >= 201703L
// This is required to find out what deployment target we are using
#include <CoreFoundation/CoreFoundation.h>
#if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_14
// C++17 new(size_t, align_val_t) is not backwards-compatible with older versions of macOS, so we can't support over-alignment in this case
#define MOODYCAMEL_MAYBE_ALIGN_TO_CACHELINE
#endif
#endif
#endif

#ifndef MOODYCAMEL_MAYBE_ALIGN_TO_CACHELINE
#define MOODYCAMEL_MAYBE_ALIGN_TO_CACHELINE AE_ALIGN(MOODYCAMEL_CACHE_LINE_SIZE)
#endif

template<typename T, size_t MAX_BLOCK_SIZE = 512>
class MOODYCAMEL_MAYBE_ALIGN_TO_CACHELINE ReaderWriterQueue
{
    // Design: Based on a queue-of-queues. The low-level queues are just
    // circular buffers with front and tail indices indicating where the
    // next element to dequeue is and where the next element can be enqueued,
    // respectively. Each low-level queue is called a "block". Each block
    // wastes exactly one element's worth of space to keep the design simple
    // (if front == tail then the queue is empty, and can't be full).
    // The high-level queue is a circular linked list of blocks; again there
    // is a front and tail, but this time they are pointers to the blocks.
    // The front block is where the next element to be dequeued is, provided
    // the block is not empty. The back block is where elements are to be
    // enqueued, provided the block is not full.
    // The producer thread owns all the tail indices/pointers. The consumer
    // thread owns all the front indices/pointers. Both threads read each
    // other's variables, but only the owning thread updates them. E.g. After
    // the consumer reads the producer's tail, the tail may change before the
    // consumer is done dequeuing an object, but the consumer knows the tail
    // will never go backwards, only forwards.
    // If there is no room to enqueue an object, an additional block (of
    // equal size to the last block) is added. Blocks are never removed.

public:
    typedef T value_type;

    // Constructs a queue that can hold at least `size` elements without further
    // allocations. If more than MAX_BLOCK_SIZE elements are requested,
    // then several blocks of MAX_BLOCK_SIZE each are reserved (including
    // at least one extra buffer block).
    AE_NO_TSAN explicit ReaderWriterQueue(size_t size = 256)
#ifndef NDEBUG
    : enqueuing(false)
		,dequeuing(false)
#endif
    {
        assert(MAX_BLOCK_SIZE == ceilToPow2(MAX_BLOCK_SIZE) && "MAX_BLOCK_SIZE must be a power of 2");
        assert(MAX_BLOCK_SIZE >= 2 && "MAX_BLOCK_SIZE must be at least 2");

        Block* firstBlock = nullptr;

        largestBlockSize = ceilToPow2(size + 1);		// We need a spare slot to fit size elements in the block
        if (largestBlockSize > MAX_BLOCK_SIZE * 2) {
            // We need a spare block in case the producer is writing to a different block the consumer is reading from, and
            // wants to enqueue the maximum number of elements. We also need a spare element in each block to avoid the ambiguity
            // between front == tail meaning "empty" and "full".
            // So the effective number of slots that are guaranteed to be usable at any time is the block size - 1 times the
            // number of blocks - 1. Solving for size and applying a ceiling to the division gives us (after simplifying):
            size_t initialBlockCount = (size + MAX_BLOCK_SIZE * 2 - 3) / (MAX_BLOCK_SIZE - 1);
            largestBlockSize = MAX_BLOCK_SIZE;
            Block* lastBlock = nullptr;
            for (size_t i = 0; i != initialBlockCount; ++i) {
                auto block = make_block(largestBlockSize);
                if (block == nullptr) {
#ifdef MOODYCAMEL_EXCEPTIONS_ENABLED
                    throw std::bad_alloc();
#else
                    abort();
#endif
                }
                if (firstBlock == nullptr) {
                    firstBlock = block;
                }
                else {
                    lastBlock->next = block;
                }
                lastBlock = block;
                block->next = firstBlock;
            }
        }
        else {
            firstBlock = make_block(largestBlockSize);
            if (firstBlock == nullptr) {
#ifdef MOODYCAMEL_EXCEPTIONS_ENABLED
                throw std::bad_alloc();
#else
                abort();
#endif
            }
            firstBlock->next = firstBlock;
        }
        frontBlock = firstBlock;
        tailBlock = firstBlock;

        // Make sure the reader/writer threads will have the initialized memory setup above:
        fence(memory_order_sync);
    }

    // Note: The queue should not be accessed concurrently while it's
    // being moved. It's up to the user to synchronize this.
    AE_NO_TSAN ReaderWriterQueue(ReaderWriterQueue&& other)
            : frontBlock(other.frontBlock.load()),
              tailBlock(other.tailBlock.load()),
              largestBlockSize(other.largestBlockSize)
#ifndef NDEBUG
    ,enqueuing(false)
		,dequeuing(false)
#endif
    {
        other.largestBlockSize = 32;
        Block* b = other.make_block(other.largestBlockSize);
        if (b == nullptr) {
#ifdef MOODYCAMEL_EXCEPTIONS_ENABLED
            throw std::bad_alloc();
#else
            abort();
#endif
        }
        b->next = b;
        other.frontBlock = b;
        other.tailBlock = b;
    }

    // Note: The queue should not be accessed concurrently while it's
    // being moved. It's up to the user to synchronize this.
    ReaderWriterQueue& operator=(ReaderWriterQueue&& other) AE_NO_TSAN
    {
        Block* b = frontBlock.load();
        frontBlock = other.frontBlock.load();
        other.frontBlock = b;
        b = tailBlock.load();
        tailBlock = other.tailBlock.load();
        other.tailBlock = b;
        std::swap(largestBlockSize, other.largestBlockSize);
        return *this;
    }

    // Note: The queue should not be accessed concurrently while it's
    // being deleted. It's up to the user to synchronize this.
    AE_NO_TSAN ~ReaderWriterQueue()
    {
        // Make sure we get the latest version of all variables from other CPUs:
        fence(memory_order_sync);

        // Destroy any remaining objects in queue and free memory
        Block* frontBlock_ = frontBlock;
        Block* block = frontBlock_;
        do {
            Block* nextBlock = block->next;
            size_t blockFront = block->front;
            size_t blockTail = block->tail;

            for (size_t i = blockFront; i != blockTail; i = (i + 1) & block->sizeMask) {
                auto element = reinterpret_cast<T*>(block->data + i * sizeof(T));
                element->~T();
                (void)element;
            }

            auto rawBlock = block->rawThis;
            block->~Block();
            std::free(rawBlock);
            block = nextBlock;
        } while (block != frontBlock_);
    }

    // Enqueues a copy of element on the queue.
    // Allocates an additional block of memory if needed.
    // Only fails (returns false) if memory allocation fails.
    AE_FORCEINLINE bool enqueue(T const& element) AE_NO_TSAN
    {
        return inner_enqueue(element);
    }

    // Enqueues a moved copy of element on the queue.
    // Allocates an additional block of memory if needed.
    // Only fails (returns false) if memory allocation fails.
    AE_FORCEINLINE bool enqueue(T&& element) AE_NO_TSAN
    {
        return inner_enqueue(std::forward<T>(element));
    }

#if MOODYCAMEL_HAS_EMPLACE
    // Like enqueue() but with emplace semantics (i.e. construct-in-place).
    //template<typename... Args>
    //AE_FORCEINLINE bool emplace(Args&&... args) AE_NO_TSAN
    //{
    //    return inner_enqueue(std::forward<Args>(args)...);
    //}
#endif

    // Attempts to dequeue an element; if the queue is empty,
    // returns false instead. If the queue has at least one element,
    // moves front to result using operator=, then returns true.
    template<typename U>
    bool try_dequeue(U& result) AE_NO_TSAN
    {
#ifndef NDEBUG
        ReentrantGuard guard(this->dequeuing);
#endif

        // High-level pseudocode:
        // Remember where the tail block is
        // If the front block has an element in it, dequeue it
        // Else
        //     If front block was the tail block when we entered the function, return false
        //     Else advance to next block and dequeue the item there

        // Note that we have to use the value of the tail block from before we check if the front
        // block is full or not, in case the front block is empty and then, before we check if the
        // tail block is at the front block or not, the producer fills up the front block *and
        // moves on*, which would make us skip a filled block. Seems unlikely, but was consistently
        // reproducible in practice.
        // In order to avoid overhead in the common case, though, we do a double-checked pattern
        // where we have the fast path if the front block is not empty, then read the tail block,
        // then re-read the front block and check if it's not empty again, then check if the tail
        // block has advanced.

        Block* frontBlock_ = frontBlock.load();
        size_t blockTail = frontBlock_->localTail;
        size_t blockFront = frontBlock_->front.load();

        if (blockFront != blockTail || blockFront != (frontBlock_->localTail = frontBlock_->tail.load())) {
            fence(memory_order_acquire);

            non_empty_front_block:
            // Front block not empty, dequeue from here
            auto element = reinterpret_cast<T*>(frontBlock_->data + blockFront * sizeof(T));
            result = std::move(*element);
            //element->~T();
            blockFront = (blockFront + 1) & frontBlock_->sizeMask;

            fence(memory_order_release);
            frontBlock_->front = blockFront;
        }
        else if (frontBlock_ != tailBlock.load()) {
            fence(memory_order_acquire);

            frontBlock_ = frontBlock.load();
            blockTail = frontBlock_->localTail = frontBlock_->tail.load();
            blockFront = frontBlock_->front.load();
            fence(memory_order_acquire);

            if (blockFront != blockTail) {
                // Oh look, the front block isn't empty after all
                goto non_empty_front_block;
            }

            // Front block is empty but there's another block ahead, advance to it
            Block* nextBlock = frontBlock_->next;
            // Don't need an acquire fence here since next can only ever be set on the tailBlock,
            // and we're not the tailBlock, and we did an acquire earlier after reading tailBlock which
            // ensures next is up-to-date on this CPU in case we recently were at tailBlock.

            size_t nextBlockFront = nextBlock->front.load();
            size_t nextBlockTail = nextBlock->localTail = nextBlock->tail.load();
            fence(memory_order_acquire);

            // Since the tailBlock is only ever advanced after being written to,
            // we know there's for sure an element to dequeue on it
            //assert(nextBlockFront != nextBlockTail);
            //AE_UNUSED(nextBlockTail);

            // We're done with this block, let the producer use it if it needs
            fence(memory_order_release);		// Expose possibly pending changes to frontBlock->front from last dequeue
            frontBlock = frontBlock_ = nextBlock;

            compiler_fence(memory_order_release);	// Not strictly needed

            auto element = reinterpret_cast<T*>(frontBlock_->data + nextBlockFront * sizeof(T));

            result = std::move(*element);
            //element->~T();
            nextBlockFront = (nextBlockFront + 1) & frontBlock_->sizeMask;

            fence(memory_order_release);
            frontBlock_->front = nextBlockFront;
        }
        else {
            // No elements in current block and no other block to advance to
            return false;
        }

        counts.fetch_sub(1, std::memory_order_relaxed);;
        return true;
    }

    // Returns the approximate number of items currently in the queue.
    // Safe to call from both the producer and consumer threads.
    inline size_t size_approx() const AE_NO_TSAN
    {
        return counts.load(std::memory_order_relaxed);
    }

    // Returns the total number of items that could be enqueued without incurring
    // an allocation when this queue is empty.
    // Safe to call from both the producer and consumer threads.
    //
    // NOTE: The actual capacity during usage may be different depending on the consumer.
    //       If the consumer is removing elements concurrently, the producer cannot add to
    //       the block the consumer is removing from until it's completely empty, except in
    //       the case where the producer was writing to the same block the consumer was
    //       reading from the whole time.
    inline size_t max_capacity() const {
        size_t result = 0;
        Block* frontBlock_ = frontBlock.load();
        Block* block = frontBlock_;
        do {
            fence(memory_order_acquire);
            result += block->sizeMask;
            block = block->next.load();
        } while (block != frontBlock_);
        return result;
    }

#if MOODYCAMEL_HAS_EMPLACE
    template<typename... Args>
    bool inner_enqueue(Args&&... args) AE_NO_TSAN
#else
    template<typename U>
	bool inner_enqueue(U&& element) AE_NO_TSAN
#endif
    {
#ifndef NDEBUG
        ReentrantGuard guard(this->enqueuing);
#endif

        // High-level pseudocode (assuming we're allowed to alloc a new block):
        // If room in tail block, add to tail
        // Else check next block
        //     If next block is not the head block, enqueue on next block
        //     Else create a new block and enqueue there
        //     Advance tail to the block we just enqueued to

        Block* tailBlock_ = tailBlock.load();
        size_t blockFront = tailBlock_->localFront;
        size_t blockTail = tailBlock_->tail.load();
        counts.fetch_add(1, std::memory_order_relaxed);

        size_t nextBlockTail = (blockTail + 1) & tailBlock_->sizeMask;
        if (nextBlockTail != blockFront || nextBlockTail != (tailBlock_->localFront = tailBlock_->front.load())) {
            fence(memory_order_acquire);
            // This block has room for at least one more element
            char* location = tailBlock_->data + blockTail * sizeof(T);
#if MOODYCAMEL_HAS_EMPLACE
            new (location) T(std::forward<Args>(args)...);
#else
            new (location) T(std::forward<U>(element));
#endif

            fence(memory_order_release);
            tailBlock_->tail = nextBlockTail;
        }
        else {
            fence(memory_order_acquire);
            if (tailBlock_->next.load() != frontBlock) {
                // Note that the reason we can't advance to the frontBlock and start adding new entries there
                // is because if we did, then dequeue would stay in that block, eventually reading the new values,
                // instead of advancing to the next full block (whose values were enqueued first and so should be
                // consumed first).

                fence(memory_order_acquire);		// Ensure we get latest writes if we got the latest frontBlock

                // tailBlock is full, but there's a free block ahead, use it
                Block* tailBlockNext = tailBlock_->next.load();
                size_t nextBlockFront = tailBlockNext->localFront = tailBlockNext->front.load();
                nextBlockTail = tailBlockNext->tail.load();
                fence(memory_order_acquire);

                // This block must be empty since it's not the head block and we
                // go through the blocks in a circle
                assert(nextBlockFront == nextBlockTail);
                tailBlockNext->localFront = nextBlockFront;

                char* location = tailBlockNext->data + nextBlockTail * sizeof(T);
#if MOODYCAMEL_HAS_EMPLACE
                new (location) T(std::forward<Args>(args)...);
#else
                new (location) T(std::forward<U>(element));
#endif

                tailBlockNext->tail = (nextBlockTail + 1) & tailBlockNext->sizeMask;

                fence(memory_order_release);
                tailBlock = tailBlockNext;
            }
            else {
                // tailBlock is full and there's no free block ahead; create a new block
                auto newBlockSize = largestBlockSize >= MAX_BLOCK_SIZE ? largestBlockSize : largestBlockSize * 2;
                auto newBlock = make_block(newBlockSize);
                if (newBlock == nullptr) {
                    // Could not allocate a block!
                    return false;
                }
                largestBlockSize = newBlockSize;

#if MOODYCAMEL_HAS_EMPLACE
                new (newBlock->data) T(std::forward<Args>(args)...);
#else
                new (newBlock->data) T(std::forward<U>(element));
#endif
                assert(newBlock->front == 0);
                newBlock->tail = newBlock->localTail = 1;

                newBlock->next = tailBlock_->next.load();
                tailBlock_->next = newBlock;

                // Might be possible for the dequeue thread to see the new tailBlock->next
                // *without* seeing the new tailBlock value, but this is OK since it can't
                // advance to the next block until tailBlock is set anyway (because the only
                // case where it could try to read the next is if it's already at the tailBlock,
                // and it won't advance past tailBlock in any circumstance).

                fence(memory_order_release);
                tailBlock = newBlock;
            }
        }

        return true;
    }


    // Disable copying
    ReaderWriterQueue(ReaderWriterQueue const&) {  }

    // Disable assignment
    ReaderWriterQueue& operator=(ReaderWriterQueue const&) {  }


    AE_FORCEINLINE static size_t ceilToPow2(size_t x)
    {
        // From http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
        --x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        for (size_t i = 1; i < sizeof(size_t); i <<= 1) {
            x |= x >> (i << 3);
        }
        ++x;
        return x;
    }

    template<typename U>
    static AE_FORCEINLINE char* align_for(char* ptr) AE_NO_TSAN
    {
        const std::size_t alignment = std::alignment_of<U>::value;
        return ptr + (alignment - (reinterpret_cast<std::uintptr_t>(ptr) % alignment)) % alignment;
    }

#ifndef NDEBUG
    struct ReentrantGuard
	{
		AE_NO_TSAN ReentrantGuard(weak_atomic<bool>& _inSection)
			: inSection(_inSection)
		{
			assert(!inSection && "Concurrent (or re-entrant) enqueue or dequeue operation detected (only one thread at a time may hold the producer or consumer role)");
			inSection = true;
		}

		AE_NO_TSAN ~ReentrantGuard() { inSection = false; }

	private:
		ReentrantGuard& operator=(ReentrantGuard const&);

	private:
		weak_atomic<bool>& inSection;
	};
#endif

    struct Block
    {
        // Avoid false-sharing by putting highly contended variables on their own cache lines
        weak_atomic<size_t> front;	// (Atomic) Elements are read from here
        size_t localTail;			// An uncontended shadow copy of tail, owned by the consumer

        char cachelineFiller0[MOODYCAMEL_CACHE_LINE_SIZE - sizeof(weak_atomic<size_t>) - sizeof(size_t)];
        weak_atomic<size_t> tail;	// (Atomic) Elements are enqueued here
        size_t localFront;

        char cachelineFiller1[MOODYCAMEL_CACHE_LINE_SIZE - sizeof(weak_atomic<size_t>) - sizeof(size_t)];	// next isn't very contended, but we don't want it on the same cache line as tail (which is)
        weak_atomic<Block*> next;	// (Atomic)

        char* data;		// Contents (on heap) are aligned to T's alignment

        const size_t sizeMask;


        // size must be a power of two (and greater than 0)
        AE_NO_TSAN Block(size_t const& _size, char* _rawThis, char* _data)
                : front(0UL), localTail(0), tail(0UL), localFront(0), next(nullptr), data(_data), sizeMask(_size - 1), rawThis(_rawThis)
        {
        }

    private:
        // C4512 - Assignment operator could not be generated
        Block& operator=(Block const&);

    public:
        char* rawThis;
    };


    static Block* make_block(size_t capacity) AE_NO_TSAN
    {
        // Allocate enough memory for the block itself, as well as all the elements it will contain
        auto size = sizeof(Block) + std::alignment_of<Block>::value - 1;
        size += sizeof(T) * capacity + std::alignment_of<T>::value - 1;
        auto newBlockRaw = static_cast<char*>(std::malloc(size));
        if (newBlockRaw == nullptr) {
            return nullptr;
        }

        auto newBlockAligned = align_for<Block>(newBlockRaw);
        auto newBlockData = align_for<T>(newBlockAligned + sizeof(Block));
        return new (newBlockAligned) Block(capacity, newBlockRaw, newBlockData);
    }

    weak_atomic<Block*> frontBlock;		// (Atomic) Elements are dequeued from this block
    std::atomic<int64_t> counts{0};

    char cachelineFiller[MOODYCAMEL_CACHE_LINE_SIZE - sizeof(weak_atomic<Block*>)];
    weak_atomic<Block*> tailBlock;		// (Atomic) Elements are enqueued to this block

    size_t largestBlockSize;

#ifndef NDEBUG
    weak_atomic<bool> enqueuing;
	mutable weak_atomic<bool> dequeuing;
#endif
};

#ifdef AE_VCPP
#pragma warning(pop)
#endif

#endif //PTHREAD_READERWRITERQUEUE_H

