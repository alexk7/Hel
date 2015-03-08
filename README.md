# Hel
Hel is a general purpose C++14 library.

## Design
With the help of C++14 features, I try to design this library with a few unusual constraints aimed at making the interface and the implementation simpler (compared to e.g. Boost libraries):
* No explicit or partial specialization of templates
* No explicit function template arguments
    * Yes, I even plan to stop using std::forward. It is annoying because the calls shows up in the debugger.
        `std::forward<T>(t)` is just syntactic sugar for `static_cast<T&&>(t)` anyway.
* No "details" namespaces, every namespace scoped entities are part of the public interface.

## Why "Hel"?
Hel is the daughter of Loki.
* http://en.wikipedia.org/wiki/Loki_%28C%2B%2B%29
* http://en.wikipedia.org/wiki/Loki
* http://en.wikipedia.org/wiki/Hel_(being)
