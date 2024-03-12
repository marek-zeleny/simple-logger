# Simple Logger

Simple C++ logger, originally developed for private projects.
Written using modern C++ features (requires C++20).

I haven't done much research on existing solutions, but since I tended to need similar features on multiple projects,
I decided to finally separate it for easier reuse.

The whole logger is contained in a simple (and very short) header file that can be simply added to your project.

## Features

- Multiple log levels/verbosity
- Separate output streams for each log level (if desired)
  - e.g. `std::cout` for Debug/Info, `std::cerr` for Warning/Error
- Efficient and precise time information
- Simple and minimalistic implementation allowing high customization
- Use with or without macros (with equal functionality)

## Installation

Either just copy the [single header file](include/simple_logger.h) to your project, or clone this repository as a
submodule and use CMake:

```
git submodule add git@github.com:marek-zeleny/simple-logger.git
```

```
# your project's CMakeLists.txt
add_subdirectory(simple-logger/)
...
target_link_libraries(your_target PUBLIC simple_logger)
```
(adjust paths and target names accordingly)

## Usage

Use the `Log` class to create a temporary instance for each log message.

You can either use the class directly (if you don't like macros),

```c++
#include <simple_logger.h>

using namespace simple_logger;

void foo() {
    Log<LogLevel::Info>() << "My info message";
    Log<LogLevel::Debug>() << "My more detailed debug message";
}
```

or use the convenience macros to reduce verbosity (if you don't like `using namespace`).

```c++
#include <simple_logger.h>

void foo() {
    LOG_INFO << "My log message";
    LOG_DEBUG << "My more detailed debug message";
}
```

(Note: Macros can be disabled by commenting out `#define SIMPLE_LOGGER_ENABLE_MACROS`)

Example output:

```
[16:44:24.078][Info][example.cpp:6][void foo()] My info message
[16:44:24.079][Debug][example.cpp:7][void foo()] My more detailed debug message
```

The convenience macros use the default output streams set in the [Config class](#configuration).
So does the `Log` class when no arguments are given to the constructor, but it can take a custom stream as a parameter.

For more detailed control of the log message, e.g. printing information in a loop or giving the logger's stream as
an argument to a function, you can save the `Log` instance into a variable.
However, be aware that the destructor prints a newline and flushes the stream's buffer, so it's advised to enclose the
instance in a code block to limit its scope.

```c++
#include <vector>
#include <ostream>
#include <simple_logger.h>

using namespace simple_logger;

void printSomethingToStream(std::ostream &stream) {
    stream << "Fancy message";
}

int foo(const std::vector<int> &vec) {
    int x = 10;
    {
        Log<LogLevel::Debug>() log{};
        log << "Printing vector from " << x << "th element:"
        for (int i = x; i < vec.size(); ++i) {
            log << " " << vec[i];
        }
        log << "\n";
        printSomethingToStream(log.getStream());
        // log variable gets destroyed here, the destructor prints a newline and flushes the stream
    }
    return x;
}
```

For consistency, also macros are defined for this purpose, although we can't talk much about reducing verbosity here...

```c++
#include <vector>
#include <ostream>
#include <simple_logger.h>

void printSomethingToStream(std::ostream &stream) {
    stream << "Fancy message";
}

int foo(const std::vector<int> &vec) {
    int x = 10;
    {
        GET_LOG_STREAM_DEBUG(logStream);
        logStream << "Printing vector from " << x << "th element:"
        for (int i = x; i < vec.size(); ++i) {
            logStream << " " << vec[i];
        }
        logStream << "\n";
        printSomethingToStream(logStream);
        // log variable gets destroyed here, the destructor prints a newline and flushes the stream
    }
    return x;
}
```

```
[16:58:22.405][Debug][example.cpp:14][int foo(const std::vector<int> &vec)] Printing vector from 10th element: 10 11
Fancy message
```

## Configuration

Some behaviour of the logger can be configured in the `Config` class.
It contains a few constant settings that can be adjusted by directly editing the [header file](include/simple_logger.h),
and some mutable (but still static) settings that can be adjusted in your code.
These config options include:

- Logger verbosity (`logLevel`)
  - Can be set separately for debug and release builds using predefined macros
- Timezone adjustment if you want to see real time in the logs
- Log file name
  - Can be adjusted from code, useful e.g. to have a different file for application and for unit tests
- Default log stream for each `logLevel` (can use the log file)
