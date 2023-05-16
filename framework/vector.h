#ifndef _H_VECTOR_
#define _H_VECTOR_

#include "../platform/memory.h"
#include "../platform/assert.h"

template<typename T>
class Vector {
    T* mData;
    u32 mCount;
    u32 mCapacity;
public:
    inline Vector() {
        mData = 0;
        mCount = 0;
        mCapacity = 0;
    }

    inline Vector(u32 capacity) {
        mData = (T*)MemAlloc(sizeof(T) * capacity);
        mCount = 0;
        mCapacity = capacity;
    }


    inline Vector(T* array, u32 count) {
        mData = (T*)MemAlloc(sizeof(T) * count);
        MemCopy(mData, array, count * sizeof(T));
        mCount = count;
        mCapacity = count;
    }

    inline Vector(const Vector<T>& other) {
        mCapacity = other.mCapacity;
        mCount = other.mCount;
        mData = (T*)MemAlloc(sizeof(T) * mCapacity);
        MemCopy(mData, other.mData, mCapacity * sizeof(T));
    }

    inline Vector<T>& operator=(const Vector<T>& other) {
        PlatformAssert(this != &other, __LOCATION__);
        if (this != &other) {
            if (mData != 0) {
                MemRelease(mData);
            }
            mCapacity = other.mCapacity;
            mCount = other.mCount;
            mData = (T*)MemAlloc(sizeof(T) * mCapacity);
            MemCopy(mData, other.mData, mCapacity * sizeof(T));
        }

        return *this;
    }

    inline ~Vector() {
        if (mData != 0) {
            MemRelease(mData);
        }
        mData = 0;
        mCount = 0;
        mCapacity = 0;
    }
public:
    T& operator[](u32 i) { 
        PlatformAssert(i < mCount, __LOCATION__);
        PlatformAssert(mData != 0, __LOCATION__);
        return mData[i]; 
    }

    const T& operator[](u32 i) const { 
        PlatformAssert(i < mCount, __LOCATION__);
        PlatformAssert(mData != 0, __LOCATION__);
        return mData[i];
    }

    inline void Reserve(u32 count) {
        if (count < 1) {
            count = 1;
        }

        if (count > mCapacity) {
            T* newData = (T*)MemAlloc(sizeof(T) * count);
            if (mCapacity > 0) {
                MemCopy(newData, mData, mCapacity * sizeof(T));
            }
            mCapacity = count;
            if (mData != 0) {
                MemRelease(mData);
            }
            mData = newData;
        }
    }

    inline void Resize(u32 count) {
        Reserve(count);
        mCount = count;
    }
    
    inline void PushBack() {
        if (mCapacity == 0) {
            Reserve(2);
        }

        if (mCapacity - mCount == 0) {
            Reserve(mCapacity * 2);
        }

        MemSet(&mData[mCount++], 0, sizeof(T));
    }

    inline void PushBack(const T& val) {
        if (mCapacity == 0) {
            Reserve(2);
        }
        
        if (mCapacity - mCount == 0) {
            Reserve(mCapacity * 2);
        }

        MemCopy(&mData[mCount++], &val, sizeof(T));
    }

    inline void PopBack() {
        if (mCount > 0) {
            mCount -= 1;
        }
    }

    inline void Append(const T* array, u32 size) {
        Reserve(mCount + size);
        MemCopy(&mData[mCount], array, sizeof(T) * size);
        mCount += size;
    }

    inline T* PeekHead() {
        if (mCount != 0) {
            return &mData[0];
        }
        return 0;
    }

    inline T* PeekTail() {
        if (mCount != 0) {
            return &mData[mCount - 1];
        }
        return 0;
    }

    inline u32 Count() {
        return mCount;
    }

    inline u32 Capacity() {
        return mCapacity;
    }
};

#endif