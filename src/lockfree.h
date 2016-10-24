/* -*- Mode: C++ ; c-basic-offset: 4 -*- */
/*!
 * @file lockfree.h
 * @brief Implementation of the LockFreeStore and LockedData templates
 *
 *
 *      Copyright 2011 <qmidiarp-devel@lists.sourceforge.net>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 *
 */

#ifndef LOCKFREE_H__5C0B9D86_95EB_47E0_81EA_D2D148F3C394__INCLUDED
#define LOCKFREE_H__5C0B9D86_95EB_47E0_81EA_D2D148F3C394__INCLUDED

#include <QMutex>

template <typename T> class LockedData;

/**
 * @brief Template class that implements a lock-free data store
 *
 * The store has two parts, the writable part and a read-only part.
 * Both parts of the store are of type T (the template typename parameter).
 * The the access to the read-only part is guaranteed to be lock-free.
 * The access of the writable part involves obtaining of lock.
 * This lock guarantees that the read-only part will be consistent.
 *
 * The threading model that is implied is consists of two threads.
 * A gui/main thread accesses and updates the writable part of the store.
 * The restricted thread has read-only and lock-free access to the
 * read-only part of the store.
 *
 * Before the restricted thread accesses the read-only part of the store
 * it should try to update it from the write-only part by calling
 * try_lockfree_update(). If gui thread is currently not accessing the
 * writable store, the update will succeed. If the gui thread is accessing
 * the writable store, the update of the read-only store will not be made
 * and the restricted thread will access the older version of the data.
 *
 * Access to the read only part of the store is obtained by casting
 * of the LockFreeStore object to a const pointer to the template parameter
 * type T.
 *
 * Access to the writable part of the store is obtained by instantiating
 * a LockedData object. LockedData implements a scoped lock of the
 * writable part of the store.
 */
template <typename T>
class LockFreeStore
{
public:
    /**
     * @brief Obtain a read-only pointer to the read-only part of the store
     */
    operator const T * () { return &m_lockfree_data; }

    /**
     * @brief If possible, update the read-only part of the store from the
     * writable part of store
     *
     * @retval true the read-only store was successful update
     * @retval false the read-only store was not updated
     */
    bool try_lockfree_update()
    {
        if (!m_mutex.tryLock()) return false;

        m_lockfree_data = m_data;
        m_mutex.unlock();
        return true;
    }

private:
    T m_data;
    T m_lockfree_data;
    QMutex m_mutex;

    friend class LockedData<T>;
};

/**
 * @brief Template class that implements a read-write access to LockFreeStore
 *
 * When a LockedData object is constructed, the writable part of the LockFreeStore
 * is locked. When the LockedData object is destructed, the writable part of the
 * LockFreeStore is unlocked. I.e. LockedData implements a scoped lock.
 *
 * Access to the writable part of the LockFreeStore is obtained by casting
 * the LockFreeStore object to a pointer to the template parameter type T.
 *
 * The store associated by passing a LockFreeStore reference as parameter to
 * the LockedData constructor.
 */
template <typename T>
class LockedData
{
public:
    /**
     * @brief Constructor.
     *
     * Obtains the writable store lock.
     *
     * @param store a reference to existing LockFreeStore
     */
    LockedData(LockFreeStore<T> &store) : m_store(store)
    {
        m_store.m_mutex.lock();
    }

    /**
     * @brief Destructor.
     *
     * Releases the writable store lock.
     *
     */
    ~LockedData()
    {
        m_store.m_mutex.unlock();
    }

    /**
     * @brief Obtain a pointer to the writable part of the store
     */
    T * operator->() { return &m_store.m_data; }

private:
    LockFreeStore<T> & m_store;
};

#endif /* #ifndef LOCKFREE_H__5C0B9D86_95EB_47E0_81EA_D2D148F3C394__INCLUDED */
