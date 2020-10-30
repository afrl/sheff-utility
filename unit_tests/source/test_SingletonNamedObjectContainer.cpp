#include "utility/SingletonNamedObjectContainer.h"

#include <assert.h>

struct Foo
{
    int a;

    Foo(int val)
      : a(val)
    {
    }
    virtual ~Foo() {}
};

struct Bar : public Foo
{
    int b;

    Bar(int val1, int val2)
      : Foo(val1)
      , b(val2)
    {
    }
    virtual ~Bar() {}
};

int main()
{
    auto& foo_collection = utility::SingletonNamedObjectContainer<Foo>::singleton();
    
    foo_collection.add("A", Foo(1));
    std::unique_ptr<Foo> bar_ptr = std::make_unique<Bar>(2, 3);
    foo_collection.add("B", std::move(bar_ptr));
    auto C_ref = foo_collection.add("C", std::make_unique<Foo>(4));

    assert(C_ref.get().a == 4);

    assert(foo_collection.index("A") == 0);
    assert(foo_collection.index("B") == 1);
    assert(foo_collection.index("C") == 2);


    assert(foo_collection.get("A").a == 1);
    assert(foo_collection.get("B").a == 2);
    assert(foo_collection.get("C").a == 4);

    assert(foo_collection.get(0).a == 1);
    assert(foo_collection.get(1).a == 2);
    assert(foo_collection.get(2).a == 4);

    assert(dynamic_cast<const Bar&>(foo_collection.get(1)).b == 3);
}