# Hel
Hel is a general purpose C++14 library.

## Design
With the help of C++14 features, I try to design this library with a few unusual constraints aimed at making the interface and the implementation simpler (compared to e.g. Boost libraries):
* All namespace scoped functions are extension points for the library.
   * They can be overloaded by client code.
   * They are always called using their non-qualified name by the library (thus invoking ADL, a.k.a. Koenig Lookup).
   * There is always at least one parameter that is dependent on a type defined by the library.
   * When the library needs an utility that can't follow those rules, it defines a global constexpr function object. It makes it non-overloadable and, as a bonus, a first-class object.
* No explicit or partial specialization of templates
* No explicit function template arguments
    * Yes, I even plan to stop using std::forward. It is annoying because the calls shows up in the debugger.
        `std::forward<T>(t)` is just syntactic sugar for `static_cast<T&&>(t)` anyway.
* No "details" namespaces
   * All namespace scoped entities are part of the public interface.

## Why "Hel"?
Hel is the daughter of Loki.
* http://en.wikipedia.org/wiki/Loki_%28C%2B%2B%29
* http://en.wikipedia.org/wiki/Loki
* http://en.wikipedia.org/wiki/Hel_(being)
