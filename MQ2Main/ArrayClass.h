/*****************************************************************************
    MQ2Main.dll: MacroQuest2's extension DLL for EverQuest
    Copyright (C) 2002-2003 Plazmic, 2003-2005 Lax

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/

#pragma once

#include <cstdint>

#pragma pack(push)
#pragma pack(4)

//----------------------------------------------------------------------------

class CDynamicArrayBase;

struct CEQException {};
struct CExceptionApplication : CEQException {};

struct CExceptionMemoryAllocation : public CEQException {
	int size;

	CExceptionMemoryAllocation(int size_) : size(size_) {}
};

// inherits from CExceptionApplication and CEQException
struct CDynamicArrayException : public CExceptionApplication {
	const CDynamicArrayBase* obj;

	CDynamicArrayException(const CDynamicArrayBase* obj_) : obj(obj_) {}
};

// base class for the dynamic array types
class CDynamicArrayBase
{
	// TODO: Change Count to m_length, make it protected, and convert all code
	// thats using it over to GetCount()
public:
	/*0x00*/    int Count = 0;  
	/*0x04*/

public:
	// two names for the same thing
	int GetCount() const { return Count; }

protected:
	void ThrowArrayClassException() const
	{
		throw CDynamicArrayException(this);
	}
};

// split into two types - one that is read-only and does not allocate or modify
// memory, and one that can. Be careful using the ones that can modify memory,
// as you can't memcpy, memset, etc on them.

template <typename T>
class ArrayClass2_RO : public CDynamicArrayBase
{
#define GET_BIN_INDEX(x) (x >> static_cast<uint8_t>(m_binShift))
#define GET_SLOT_INDEX(x) (m_slotMask & index)
public:
	inline T& operator[](int index) { return Get(index); }
	inline const T& operator[](int index) const { return Get(index); }

	T& Get(int index) { return m_array[GET_BIN_INDEX(index)][GET_SLOT_INDEX(index)]; }
	inline const T& Get(int index) const { return m_array[GET_BIN_INDEX(index)][GET_SLOT_INDEX(index)]; }

	// try to get an element by index, returns pointer to the element.
	// if the index is out of bounds, returns null.
	T* SafeGet(int index)
	{
		if (index < Count)
		{
			int bin = GET_BIN_INDEX(index);
			if (bin < m_binCount)
			{
				int slot = GET_SLOT_INDEX(index);
				if (slot < m_maxPerBin)
				{
					return &m_array[bin][slot];
				}
			}
		}

		return nullptr;
	}

	bool IsMember(const T& element) const
	{
		if (Count <= 0)
			return false;
		for (int i = 0; i < Count; ++i) {
			if (Get(i) == element)
				return true;
		}
		return false;
	}

protected:
/*0x04*/ int m_maxPerBin = 0;
/*0x08*/ int m_slotMask = 0;
/*0x0c*/ int m_binShift = 0;
/*0x10*/ T** m_array = nullptr;
/*0x14*/ int m_binCount = 0;
/*0x18*/ bool m_valid = 0;
/*0x1c*/
};

#undef GET_BIN_INDEX
#undef GET_SLOT_INDEX

//----------------------------------------------------------------------------

// ArrayClass2 is a dynamic array implementation that makes use of bins
// to reduce the overhead of reallocation. This allows for faster resize
// operations as existing bins do not need to be relocated, just the
// list of bins. See Assure() for more information.

template <typename T>
class ArrayClass2 : public ArrayClass2_RO<T>
{
public:
	// constructs the array 
	ArrayClass2()
	{
		m_maxPerBin = 1;
		m_binShift = 0;

		do {
			m_maxPerBin <<= 1;
			m_binShift++;
		} while (m_maxPerBin < 32);

		m_slotMask = m_maxPerBin - 1;
		m_array = nullptr;
		Count = 0;
		m_binCount = 0;
		m_valid = true;
	}

	~ArrayClass2()
	{
		Reset();
	}

	ArrayClass2& operator=(const ArrayClass2& rhs)
	{
		if (this != &rhs)
		{
			Assure(0);
			if (m_array)
				Count = 0;
			if (rhs.Count) {
				Assure(rhs.Count);
				if (m_valid) {
					for (int i = 0; i < rhs.Count; ++i)
						Get(i) = rhs.Get(i);
				}
				Count = rhs.Count;
			}
		}
		return *this;
	}

	// clear the contents of the array and make it empty
	void Reset()
	{
		for (int i = 0; i < m_binCount; ++i)
			delete[] m_array[i];
		delete[] m_array;
		m_array = nullptr;
		m_binCount = 0;
		Count = 0;
	}

	void Add(const T& value)
	{
		InsertElement(Count, value);
	}

	void InsertElement(int index, const T& value)
	{
		if (index >= 0) {
			if (index < Count) {
				Assure(Count + 1);
				for (int idx = Count; idx > index; --idx)
					Get(idx) = Get(idx - 1);
				Get(index) = value;
				++Count
			}
			else {
				SetElementIdx(index, value);
			}
		}
	}

	void SetElementIdx(int index, const T& value)
	{
		if (index >= 0) {
			if (index >= Count) {
				Assure(index + 1);
				if (m_valid)
					Count = index + 1;
			}
			if (m_valid) {
				Get(index) = value;
			}
		}
	}

	void DeleteElement(int index)
	{
		if (index >= 0 && index < Count && m_array) {
			for (; index < Count - 1; ++index)
				Get(index) = Get(index + 1);
			--Count;
		}
	}

private:
	// Assure() makes sure that there is enough allocated space for
	// the requested size. This is the primary function used for allocating
	// memory in ArrayClass2. Because the full array is broken down into
	// a set of bins, it is more efficient at growing than ArrayClass.
	// When the array needs to be resized, it only needs to reallocate the
	// list of bins and create more bins. Existing bins do not need to be
	// reallocated, they can just be copied to the new list of bins.
	bool Assure(int requestedSize)
	{
		if (m_valid && requestedSize > 0) {
			int newBinCount = ((requestedSize - 1) >> static_cast<int8_t>(m_binShift)) + 1;
			if (newBinCount > m_binCount) {
				T** newArray = new T*[newBinCount];
				if (newArray) {
					for (int i = 0; i < m_binCount; ++i)
						newArray[i] = m_array[i];
					for (int curBin = m_binCount; curBin < newBinCount; ++curBin) {
						T* newBin = new T[m_maxPerBin];
						newArray[curBin] = newBin;
						if (!newBin) {
							m_valid = false;
							break;
						}
					}
					if (m_valid) {
						delete[] m_array;
						m_array = newArray;
						m_binCount = newBinCount;
					}
				} else {
					m_valid = false;
				}
			}
			// special note about this exception: the eq function was written this way,
			// but its worth noting that new will throw if it can't allocate, which means
			// this will never be hit anyways. The behavior would not change if we removed
			// all of the checks for null returns values from new in this function.
			if (!m_valid) {
				Reset();
				ThrowArrayClassException();
			}
		}
		return m_valid;
	}
};

//----------------------------------------------------------------------------

// simpler than ArrayClass2, ArrayClass is a simple wrapper around a dynamically
// allocated array. To grow this array requires reallocating the entire array and
// copying objects into the new array.

template <typename T>
class ArrayClass_RO : public CDynamicArrayBase
{
public:
	T& Get(int index)
	{
		if (index >= Count || index < 0 || m_array == nullptr)
			ThrowArrayClassException();
		return m_array[index];
	}

	const T& Get(int index) const
	{
		if (index >= Count || index < 0 || m_array == nullptr)
			ThrowArrayClassException();
		return m_array[index];
	}

	T& operator[](int index) { return Get(index); }
	const T& operator[](int index) const { return Get(index); }

	// const function that returns the element at the index *by value*
	T GetElementIdx(int index) const { return Get(index); }

protected:
	/*0x04*/    T* m_array = nullptr;
	/*0x08*/    int m_alloc = 0;
	/*0x0c*/    bool m_isValid = true;
	/*0x10*/
};

template <typename T>
class ArrayClass : public ArrayClass_RO<T>
{
public:
	ArrayClass() {}

	ArrayClass(int reserve)
	{
		m_array = new T[size];
		m_alloc = size;
	}

	ArrayClass(const ArrayClass& rhs)
	{
		if (rhs.Count) {
			AssureExact(rhs.Count);
			if (m_array) {
				for (int i = 0; i < rhs.Count; ++i)
					m_array[i] = rhs.m_array[i];
			}
			Count = rhs.Count;
		}
	}

	~ArrayClass()
	{
		Reset();
	}

	ArrayClass& operator=(const ArrayClass& rhs)
	{
		if (this == &rhs)
			return *this;
		Reset();
		if (rhs.Count) {
			AssureExact(rhs.Count);
			if (m_array) {
				for (int i = 0; i < rhs.Count; ++i)
					m_array[i] = rhs.m_array[i];
			}
			Count = rhs.Count;
		}
		return *this;
	}

	void Reset()
	{
		if (m_array) {
			delete[] m_array;
		}
		m_array = nullptr;
		m_alloc = 0;
		Count = 0;
	}

	void Add(const T& element)
	{
		SetElementIdx(Count, element);
	}

	void SetElementIdx(int index, const T& element)
	{
		if (index >= 0) {
			if (index >= Count) {
				Assure(index + 1);
				if (m_array) {
					Count = index + 1;
				}
			}
			if (m_array) {
				m_array[index] = element;
			}
		}
	}

	void InsertElement(int index, const T& element)
	{
		if (index >= 0) {
			if (index < Count) {
				Assure(Count + 1);
				if (m_array) {
					for (int idx = Count; idx > index; ++idx)
						m_array[idx] = m_array[idx - 1];
					m_array[index] = element;
					Count++;
				}
			} else {
				SetElementIdx(index, element);
			}
		}
	}

	void DeleteElement(int index)
	{
		if (index >= 0 && index < Count && m_array) {
			for (; index < Count - 1; ++index)
				m_array[index] = m_array[index + 1];
			Count--;
		}
	}

private:
	// this function will ensure that there is enough space allocated for the
	// requested size. the underlying array is one contiguous block of memory.
	// In order to grow it, we will need to allocate a new array and move
	// everything over.
	// this function will allocate 2x the amount of memory requested as an
	// optimization aimed at reducing the number of allocations that occur.
	void Assure(int requestedSize)
	{
		if (requestedSize && (requestedSize > m_alloc || !m_array)) {
			int allocatedSize = (requestedSize + 4) << 1;
			T* newArray = new T[allocatedSize];
			if (!newArray) {
				delete[] m_array;
				m_array = nullptr;
				m_alloc = 0;
				m_isValid = false;
				throw CExceptionMemoryAllocation{ allocatedSize };
			}
			if (m_array) {
				for (int i = 0; i < Count; ++i)
					newArray[i] = m_array[i];
				delete[] m_array;
			}
			m_array = newArray;
			m_alloc = allocatedSize;
		}
	}
	
	// this behaves the same as Assure, except for its allocation of memory
	// is exactly how much is requested.
	void AssureExact(int requestedSize)
	{
		if (requestedSize && (requestedSize > m_alloc || !m_array)) {
			T* newArray = new T[requestedSize];
			if (!newArray) {
				delete[] m_array;
				m_array = nullptr;
				m_alloc = 0;
				m_isValid = false;
				throw CExceptionMemoryAllocation(requestedSize);
			}
			if (m_array) {
				for (int i = 0; i < Count; ++i)
					newArray[i] = m_array[i];
				delete[] m_array;
			}
			m_array = newArray;
			m_alloc = requestedSize;
		}
	}
};

#pragma pack(pop)
