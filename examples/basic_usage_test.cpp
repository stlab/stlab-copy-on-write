/**
 * @file basic_usage.cpp
 * @brief Basic usage example of stlab::copy_on_write
 *
 * This example demonstrates the key features of copy_on_write:
 * - Efficient copying through shared data
 * - Copy-on-write semantics when modifying
 */
#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <stlab/copy_on_write.hpp>

using namespace std;
using namespace stlab;

namespace {

// This is a simple document class that uses copy-on-write to manage its lines.
class document {
    stlab::copy_on_write<std::vector<std::string>> _lines;

public:
    // We can default all the special member functions and still have value semantics.
    document() = default;
    // Copy operations are noexcept and only cost an atomic increment.
    document(const document& other) noexcept = default;
    document& operator=(const document& other) noexcept = default;

    // Move operations are noexcept and minimal cost.
    document(document&& other) noexcept = default;
    document& operator=(document&& other) noexcept = default;

    // We only expose const iterators - exposing non-const iterators may require a copy and
    // would invalidate any const iterators. This is unusual for a container and may catch
    // developers off guard.

    using iterator = std::vector<std::string>::const_iterator;

    iterator begin() const { return _lines.read().begin(); }
    iterator end() const { return _lines.read().end(); }
    size_t size() const { return _lines.read().size(); }
    bool empty() const { return _lines.read().empty(); }

    /// Return true if the two documents share the same underlying data.
    bool identity(const document& other) const { return _lines.identity(other._lines); }

    /// Insert a line at the given index.
    void insert(std::string&& line, size_t index) {
        assert(index <= size() && "index out of bounds");
        _lines.write(
            [&](const std::vector<std::string>& lines) {
                std::vector<std::string> new_lines;
                new_lines.reserve((lines.size() + 1) * 2);
                new_lines.insert(new_lines.end(), lines.begin(), lines.begin() + index);
                new_lines.insert(new_lines.end(), std::move(line));
                new_lines.insert(new_lines.end(), lines.begin() + index, lines.end());
                return new_lines;
            },
            [&](std::vector<std::string>& lines) {
                // If the object is unique, we can modify the underlying data in place
                lines.insert(lines.begin() + index, std::move(line));
            });
    }

    /// Erase a line at the given index.
    void erase(size_t index) {
        assert(index < size() && "index out of bounds");
        _lines.write(
            [&](const std::vector<std::string>& lines) {
                std::vector<std::string> new_lines;
                new_lines.reserve((lines.size() - 1) * 2);
                new_lines.insert(new_lines.end(), lines.begin(), lines.begin() + index);
                new_lines.insert(new_lines.end(), lines.begin() + index + 1, lines.end());
                return new_lines;
            },
            [&](std::vector<std::string>& lines) {
                // If the object is unique, we can modify the underlying data in place
                lines.erase(lines.begin() + index);
            });
    }
};

} // namespace

int main() {
    cerr << "--- Test starting ---" << endl;
    document d0;
    d0.insert("Hello, world!", 0);
    d0.insert("After Hello", 1);

    document d1(d0);
    assert(d0.identity(d1));
    cerr << "main: calling d1.insert..." << endl;
    d1.insert("Start of d1", 0);
    cerr << "main: d1.insert returned." << endl;
    assert(!d0.identity(d1));

    cout << "d0:" << endl;
    for (const auto& line : d0) {
        cout << line << endl;
    }

    cout << "d1:" << endl;
    for (const auto& line : d1) {
        cout << line << endl;
    }
    cerr << "--- Test finished ---" << endl;
    return 0;
}
