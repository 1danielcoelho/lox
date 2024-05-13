# Implementation of Lox
This follows [Crafting Interpreters](https://craftinginterpreters.com) by Robert Nystrom, implementing the jlox and clox interpreters. Those were intended to be in Java and C, respectively, but I implemented both using C++ in this repository.

The jlox implementation is missing the last couple of chapters on OOP concepts, and the clox implementation is missing the last chapter on optimizations (really just the NaN boxing section as the rest doesn't really apply).

I used C++ STL datastructures (std::vector, std::unordered_map, etc.) at times when the book spun off into implementing dynamically sized arrays and hash maps from scratch. This worked fine, but threw a wrench into things when chapter 26 (Garbage Collection) came along. I could provide my own allocator for std::vector and std::string in order to track total heap allocation size, but I ran into trouble when doing the same for std::unordered_map. This means the heap usage count is somewhat more imprecise than what C implementations should have ended up with, but it should largely be fine: Garbage collection itself still works as the book describes, just *when* to run it that should very slightly change.

Additionally, the `vm`'s `strings` member is used as kind of a hash set in the book, but in my implementation I just used effectively a `std::unordered_map<std::string, Lox::ObjectString*>`, so that it was trivial to get the string hashing behavior working as intended. I believe a more faithful implementation of the book would have used `std::unordered_map<Lox::ObjectString*, Lox::ObjectString*>` instead but I didn't feel like dealing with the `std::hash<>` implementation of `Lox::ObjectString*` (if that's even the right way of doing it). This mean we may store an additional copy of each interned string (as the `std::string` in the key avoids the string interning), but that is likely not that bad.

There are other minor `// TODO`s throughout the code.

# Building and running
For Windows:
- Run build.bat: It will use MSVC to compile and run.

For Linux:
- Run build.sh: It will use g++ to compile and run.

Note that build.bat will call vcvarsall.bat on a path hard-coded in setup_for_dev.bat, if it needs to. You will likely want to tweak that path to your MSVC version.

Tweak the build script itself to point at src/jlox/unity.cpp to compile jlox instead of clox, to switch the compiler used, and to toggle between interpreter and script file modes.

There are also .code-workspace and .vscode/launch.json files, so that the build can be debugged on Windows via VSCode.