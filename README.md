# Size Demo

A demonstration of how to get the size of an array.

The demo code is intended for use on [Arduboy](https://arduboy.com/),  
but the `Size.h` file should compile in any C++ environment thanks to conditional compilation.

## Rationale

Prior to C++17 when [`std::size`](https://en.cppreference.com/w/cpp/iterator/size) was introduced,  
the C++ standard library had no facility to get the size of an array.

This code provides that capability by using a subset of the mechanism used by `std::size`.  
Specifically it makes use of a template function and the compiler's ability to infer template parameters.

## How does it work?

When the compiler sees code that is trying to use a function and that function is templated and no template arguments are provided, it will try to infer the template parameters from the function's usage.

In other words, given the function:
```cpp
template< typename Type >
const Type & max(const Type & left, const Type & right)
{
	return (left < right) ? left : right;
}
```

And the function usage:
```cpp
const auto result = max(5, 10);
```

The compiler will see that the provided arguments are `int` and `int`,  
and thus deduce that `Type` is `int` because it can subtitute `Type` with `int` to internally produce the following valid template specialisation:
```cpp
template<>
const int & max<int>(const int & left, const int & right)
{
	return (left < right) ? left : right;
}
```

This same logic applies to the `getSize` function.

When the compiler sees:
```cpp
unsigned char array[10];

size_t arraySize = getSize(array);
```

It compares `unsigned char array[10]` (whose type is `unsigned char (&)[10]`) with `const Type (&)[size]`,  
and deduces that it can substitute `Type` with `unsigned char` and `size` with `10`,  
thus internally producing the following valid template specialisation:
```cpp
template<>
constexpr std::size_t getSize<int, 10>(const int (&)[10]) noexcept
{
	return 10;
}
```

## What about the `sizeof(x) / sizeof(x[0])` technique?

### It's not that easy...

This technique could be used instead, but it's not as easy as you might initially think.

Most people would probably attempt to write something like this:
```cpp
constexpr std::size_t getSize(const int array[]) noexcept
{
	return (sizeof(array) / sizeof(array[0]));
}
```
But this won't work!

It won't work because when you pass an array to a function, it instead decays into a pointer.  
Thus the aforementioned function would actually become:
```cpp
constexpr std::size_t getSize(const int * array) noexcept
{
	return (sizeof(array) / sizeof(array[0]));
}
```
And `sizeof(array)` would actually be the size of a pointer on the system in question.  
On an AVR environment, the size of a pointer is usually 2 bytes and the size of `int` is usually 2 bytes.  
That would mean that `(sizeof(array) / sizeof(array[0]))` is equivalent to `2 / 2`, which evaluates to `1`.  
Which in turn means that this version of `getSize` would return `1`, no matter what argument you gave it.

But that's far from the only problem with this example.

Firstly, because it actually accepts a pointer, you could pass a pointer to it instead of an array.  
For example `getSize(nullptr)` would be perfectly valid, and would return `1`.  
This is actually the root of the previous problem.  
If the function couldn't accept pointers, then miscalculation wouldn't be an issue.

And secondly, you'd have to write a new version for every type of array you'd want to know the size of.  
`char`, `int`, `short`, `long`, `unsigned char`, `signed char`, `unsigned int`, `unsigned short`, `unsigned long`...  
That's going to become very tedious.

### Two problems, one solution

So essentially there are three problems:
1. Having to write a new function for every single type variation
2. The function accepts pointers when it shouldn't, which leads to the calculated result being incorrect

Is there something that can solve all these problems?

Yes! It's templates!

Templates (and generics in general) were practically invented to solve both of these problems!

### Templates to the rescue!

This is where templates come to the rescue!
```cpp
template< typename Type, std::size_t size >
constexpr std::size_t getSize(const Type (&array)[size]) noexcept
{
	return (sizeof(array) / sizeof(array[0]));
}
```
This solves both problems.

1. There's no need to write a new function for every type because templates work for all valid types.
2. Specifying `const Type (&array)[size]` tells the compiler that the argument has to be an array type.  
Thus it's now impossible to pass a pointer by accident because the function is type safe,  
and as a result you won't get any silly miscalculation errors.

But if you've read the actual code or any of the earlier sections,  
you'll know that this isn't the code I've used.

Why? Because it's redundant!

In order to detect the array, you also have to detect its size,  
so by the time you're trying to calculate `(sizeof(array) / sizeof(array[0]))`,  
you already know what the answer is going to be!
`(sizeof(array) / sizeof(array[0])) == size`!

So rather than complicating the code with `(sizeof(array) / sizeof(array[0]))`,  
it's much simpler and easier to just return the inferred size.
```cpp
template< typename Type, std::size_t size >
constexpr std::size_t getSize(const Type (&)[size]) noexcept
{
	return size;
}
```

Et voila, the ideal implementation.  
Type safe, `constexpr`, almost impossible to misuse.

### But what about `#define SIZE(x) (sizeof(x) / sizeof(x[0])`?

Macros are _**EVIL**_ for a number of reasons.  
You should avoid using macros for anything other than conditional compilation.

In this case, macros are evil because they aren't type safe.
If you did `#define SIZE(x) (sizeof(x) / sizeof(x[0])`, then yes, the following would work:
```cpp
unsigned char array[10];

size_t arraySize = SIZE(array);
```
So problem 1. would be solved.

However, this code would also work:
```cpp
int x = 0;
int * pointer = &x;

size_t arraySize = SIZE(x);
```
And suddenly we're back to the earlier example of `sizeof(int *) / sizeof(int)` evaluating to `1`.  
Which means that problem 2. isn't solved!

Macros are not type safe. Macros are _**evil**_. Avoid them!
