#pragma once

#include "utility/UniquePtrSlice.h"
#include "utility/serialization/NamedValue.h"

#include <assert.h>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

namespace utility
{
template<typename T>
class SingletonNamedObjectContainer;

/// @brief A helper class that provides a reference to an object in the templated
/// SingletonNamedObjectContainer.
template<typename T>
struct SingletonNamedObjectContainerRef
{
    using Index                            = int16_t;
    static const Index UNINITIALIZED_INDEX = -1;

    Index index;

    SingletonNamedObjectContainerRef()
      : index(SingletonNamedObjectContainer<T>::UNINITIALIZED_INDEX)
    {
    }
    SingletonNamedObjectContainerRef(Index i)
      : index(i)
    {
    }
    SingletonNamedObjectContainerRef(const SingletonNamedObjectContainerRef& other)
      : index(other.index)
    {
    }

    /// @brief Provides convenient access (similar to std::reference_wrapper) to the object in
    /// the templated SingletonNamedObjectContainer.
    const T& get() const
    {
        assert(index != SingletonNamedObjectContainer<T>::UNINITIALIZED_INDEX);
        return SingletonNamedObjectContainer<T>::singleton().get(index);
    }
};

/// @brief Stores a collection of polymorphic objects and allows access via an index or name.
///
/// This template class is used extensively in other modules of the framework to store properties,
/// criteria, etc. for the analysis.  Each field/enrichment will then hold an index instead of a
/// pointer and accesses the appropriate object through the index.  However, the objects are
/// assigned to a field/enrichment via the name.  For example, a set of elastic properties may be
/// given a name based on the material it models, and fields are assigned the properties by the
/// material name.
///
/// Follows the singleton pattern, so there will only be one instace of the object.  However, since
/// the container may be modified and accessed by multiple threads, locks are used (mutex type).
///
/// Performance note: shared locks are used to access the objects (preventing data races if add()
/// were to be called at the same time by another thread).  However, the container is likely to be
/// used in a way that precludes this from happening.  For example, properties, criteria, etc. are
/// all added before the finite element analysis takes place, which is when access is needed.
/// If benchmarks show that the thread locks create a bottleneck, consider making this change.
///
/// @tparam T - type of objects for the container (each will be wrapped in a unique_ptr internally)
template<typename T>
class SingletonNamedObjectContainer
{
  public:
    using Index                            = int16_t;
    static const Index UNINITIALIZED_INDEX = -1;

    /// @brief Access singleton instance of container for the given templated type.
    /// Note: this is thread-safe if the compiler supports thread-safe function-local static
    /// initialization.
    static SingletonNamedObjectContainer& singleton()
    {
        static SingletonNamedObjectContainer singleton;
        return singleton;
    };

    /// Adds an object to the container
    /// @tparam DerivedT - a derived class of type T
    /// @param name - used to refer to the new object (must be unique in the container)
    /// @param object - a r-value to forward to a move-constructor while creating the unique_ptr
    /// @return The index of the new object
    template<typename DerivedT>
    SingletonNamedObjectContainerRef<T> add(std::string name, DerivedT&& object);
    /// Adds an object to the container
    /// @param name - used to refer to the new object (must be unique in the container)
    /// @param object_ptr - a unique_ptr that will be moved to the container (passed parameter will
    ///        be empty after the method)
    /// @return The index of the new object
    SingletonNamedObjectContainerRef<T> add(std::string name, std::unique_ptr<T>&& object_ptr);
    /// Adds an object to the container by cloning the given object
    /// @param name - used to refer to the new object (must be unique in the container)
    /// @param object_ptr - a object that will be cloned (must have a clone() method
    ///        that returns a unique_ptr<T> object).
    /// @return The index of the new object
    SingletonNamedObjectContainerRef<T> add(std::string name, const T& object);

    /// Get the index corresponding to the given object name.
    Index index(std::string name);
    /// Access an object by its name.
    const T& get(std::string name);
    /// Access an object by its index.
    const T& get(Index index);
    /// Get a reference to an object by its index.
    SingletonNamedObjectContainerRef<T> get_ref(Index index);
    /// Query the number of objects in the container.
    Index size() const;

    template<typename SerializerType>
    void serialize(SerializerType& s) const
    {
        s(named_value("n_objects", objects_.size()));

        s(named_value("objects", unique_ptr_slice(objects_.data(), objects_.size())));

        std::vector<std::string> names;
        names.reserve(name_to_index_map_.size());
        for (auto& key_val : name_to_index_map_)
        {
            assert(key_val.second == names.size());
            names.push_back(key_val.first);
        }

        assert(names.size() == objects_.size());
        s(named_value("names", slice(names)));
    }

    template<typename SerializerType>
    void deserialize(SerializerType& s)
    {
        throw _UTIL_MSG_EXCEPT_C("TODO implement");
    }

  private:
    std::vector<std::unique_ptr<T>> objects_;
    std::map<std::string, Index> name_to_index_map_;

    mutable std::shared_timed_mutex mutex_;

    /// Hidden constructor to force singleton pattern
    SingletonNamedObjectContainer() {}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// Template definitions

template<typename T>
template<typename DerivedT>
inline SingletonNamedObjectContainerRef<T> SingletonNamedObjectContainer<T>::add(std::string name,
                                                                                 DerivedT&& object)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    assert(name_to_index_map_.count(name) == 0);

    objects_.push_back(std::make_unique<DerivedT>(std::move(object)));
    Index new_index          = static_cast<Index>(objects_.size() - 1);
    name_to_index_map_[name] = new_index;
    return { new_index };
}

template<typename T>
inline SingletonNamedObjectContainerRef<T> SingletonNamedObjectContainer<T>::add(
  std::string name,
  std::unique_ptr<T>&& object_ptr)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    assert(name_to_index_map_.count(name) == 0);

    objects_.push_back(std::move(object_ptr));
    Index new_index          = static_cast<Index>(objects_.size() - 1);
    name_to_index_map_[name] = new_index;
    return { new_index };
}

template<typename T>
inline SingletonNamedObjectContainerRef<T> SingletonNamedObjectContainer<T>::add(std::string name,
                                                                                 const T& object)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    assert(name_to_index_map_.count(name) == 0);

    objects_.push_back(object.clone());
    Index new_index          = static_cast<Index>(objects_.size() - 1);
    name_to_index_map_[name] = new_index;
    return { new_index };
}

template<typename T>
inline typename SingletonNamedObjectContainer<T>::Index SingletonNamedObjectContainer<T>::index(
  std::string name)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return name_to_index_map_[name];
}

template<typename T>
inline const T& SingletonNamedObjectContainer<T>::get(std::string name)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return *objects_[name_to_index_map_[name]];
}

template<typename T>
inline const T& SingletonNamedObjectContainer<T>::get(Index index)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return *objects_[index];
}

template<typename T>
inline SingletonNamedObjectContainerRef<T> SingletonNamedObjectContainer<T>::get_ref(Index index)
{
    return { index };
}

template<typename T>
inline typename SingletonNamedObjectContainer<T>::Index SingletonNamedObjectContainer<T>::size()
  const
{
    return static_cast<Index>(objects_.size());
}
}