## Components

All components define their stuff in namespaces:

```c++
namespace caney { namespace COMP { inline namespace vX { ... } } }
```

The `X` is a version number, to allow introducing backward compatible changes. The version should be reset to 1 on so-bumps.

The `std` component being an exception (`X` again being the version) to not shadow the `::std` namespace.

```c++
namespace caney { inline namespace stdX { ... } }
```

## Include order

All include files start with `#pragma once`.

The include order is as follows:
- for source files: the corresponding header
- headers from other caney libraries (`#include "caney/other/foo.h"`)
- headers from same caney library (`#include "local.h"`)
- std c++ headers
- 3rd party headers

These sections should be separated by an empty line, within a section no empty line is necessary.
Within a section sort by files before sub directories and then by name.
