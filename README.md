# C++ abstract map proof of concept #

This demonstrates container classes that interoperate with eachother
using a common abstract base class, and a common generic iterator
class. Each instance of the generic iterator class wraps an underlying
implementation, using an approach that is analogous to the one used
by [AnyIterator](https://github.com/TinyTinni/AnyIterator).
