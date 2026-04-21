#include <stlab/copy_on_write.hpp>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <string>
#include <utility>
#include <vector>

using stlab::copy_on_write;

TEST_CASE("copy_on_write basic construction") {
    SUBCASE("default construction") {
        copy_on_write<int> cow;
        CHECK(*cow == 0); // default constructed int is 0
        // Note: default constructed instances share a static model, so they're not unique
        copy_on_write<int> cow2;
        CHECK(cow.identity(cow2));
    }

    SUBCASE("value construction") {
        copy_on_write<int> cow(42);
        CHECK(*cow == 42);
        CHECK(cow.unique());
    }

    SUBCASE("string construction") {
        copy_on_write<std::string> cow(std::string("hello"));
        CHECK(*cow == "hello");
        CHECK(cow.unique());
    }

    SUBCASE("in-place construction") {
        copy_on_write<std::vector<int>> cow(5, 10); // 5 elements, all 10
        CHECK(cow->size() == 5);
        CHECK((*cow)[0] == 10);
        CHECK(cow.unique());
    }
}

TEST_CASE("copy_on_write copy semantics") {
    SUBCASE("copy construction shares data") {
        copy_on_write<int> cow1(42);
        copy_on_write<int> cow2(cow1);

        CHECK(*cow1 == 42);
        CHECK(*cow2 == 42);
        CHECK(cow1.identity(cow2)); // same underlying data
        CHECK_FALSE(cow1.unique());
        CHECK_FALSE(cow2.unique());
    }

    SUBCASE("copy assignment shares data") {
        copy_on_write<int> cow1(42);
        copy_on_write<int> cow2(100);

        cow2 = cow1;

        CHECK(*cow1 == 42);
        CHECK(*cow2 == 42);
        CHECK(cow1.identity(cow2));
        CHECK_FALSE(cow1.unique());
        CHECK_FALSE(cow2.unique());
    }

    SUBCASE("write triggers copy") {
        copy_on_write<int> cow1(42);
        copy_on_write<int> cow2(cow1);

        CHECK(cow1.identity(cow2));

        cow2.write([](int) { return 100; }); // This should trigger copy-on-write

        CHECK(*cow1 == 42);
        CHECK(*cow2 == 100);
        CHECK_FALSE(cow1.identity(cow2)); // no longer the same data
        CHECK(cow1.unique());
        CHECK(cow2.unique());
    }

    SUBCASE("write with transform and inplace not unique") {
        copy_on_write<int> cow1(42);
        copy_on_write<int> cow2(cow1);

        // increment by different amounts to check that the transform is applied
        cow2.write([](const int& value) { return value + 1; }, [](int& value) { value += 2; });

        CHECK(*cow1 == 42);
        CHECK(*cow2 == 43);
        CHECK_FALSE(cow1.identity(cow2));
        CHECK(cow1.unique());
        CHECK(cow2.unique());
    }

    SUBCASE("write with transform and inplace unique") {
        copy_on_write<int> cow(42);

        // increment by different amounts to check that the inplace function is applied
        cow.write([](const int& value) { return value + 1; }, [](int& value) { value += 2; });

        CHECK(*cow == 44);
    }
}

TEST_CASE("copy_on_write move semantics") {
    SUBCASE("move construction") {
        copy_on_write<std::string> cow1(std::string("hello"));
        copy_on_write<std::string> cow2(std::move(cow1));

        CHECK(*cow2 == "hello");
        CHECK(cow2.unique());
        // cow1 is now in moved-from state, don't access it
    }

    SUBCASE("move assignment") {
        copy_on_write<std::string> cow1(std::string("hello"));
        copy_on_write<std::string> cow2(std::string("world"));

        cow2 = std::move(cow1);

        CHECK(*cow2 == "hello");
        CHECK(cow2.unique());
        // cow1 is now in moved-from state, don't access it
    }
}

TEST_CASE("copy_on_write access methods") {
    copy_on_write<std::string> cow(std::string("hello"));

    SUBCASE("read access") {
        const auto& ref = cow.read();
        CHECK(ref == "hello");
        CHECK(cow.unique()); // read doesn't affect uniqueness
    }

    SUBCASE("const conversion operator") {
        const std::string& ref = cow;
        CHECK(ref == "hello");
        CHECK(cow.unique());
    }

    SUBCASE("dereference operator") {
        CHECK(*cow == "hello");
        CHECK(cow.unique());
    }

    SUBCASE("arrow operator") {
        CHECK(cow->length() == 5);
        CHECK(cow.unique());
    }

    SUBCASE("write access when unique") {
        cow.write([](std::string& v) { v = "world"; });
        CHECK(*cow == "world");
        CHECK(cow.unique());
    }
}

TEST_CASE("copy_on_write comparison operators") {
    copy_on_write<int> cow1(42);
    copy_on_write<int> cow2(42);
    copy_on_write<int> cow3(100);
    copy_on_write<int> cow4(cow1); // shared with cow1

    SUBCASE("equality") {
        CHECK(cow1 == cow2); // same value, different objects
        CHECK(cow1 == cow4); // same identity
        CHECK_FALSE(cow1 == cow3);

        CHECK(cow1 == 42); // compare with value
        CHECK(42 == cow1); // reverse comparison
    }

    SUBCASE("inequality") {
        CHECK_FALSE(cow1 != cow2);
        CHECK_FALSE(cow1 != cow4);
        CHECK(cow1 != cow3);

        CHECK_FALSE(cow1 != 42);
        CHECK(cow1 != 100);
    }

    SUBCASE("ordering") {
        CHECK_FALSE(cow1 < cow2); // equal values
        CHECK_FALSE(cow1 < cow4); // same identity
        CHECK(cow1 < cow3);       // 42 < 100
        CHECK_FALSE(cow3 < cow1);

        CHECK(cow1 < 100);
        CHECK_FALSE(cow1 < 42);
        CHECK_FALSE(cow1 < 10);
    }
}

TEST_CASE("copy_on_write assignment from value") {
    copy_on_write<std::string> cow(std::string("hello"));

    SUBCASE("assign when unique") {
        CHECK(cow.unique());
        cow = std::string("world");
        CHECK(*cow == "world");
        CHECK(cow.unique());
    }

    SUBCASE("assign when shared") {
        copy_on_write<std::string> cow2(cow);
        CHECK_FALSE(cow.unique());
        CHECK_FALSE(cow2.unique());

        cow = std::string("world");

        CHECK(*cow == "world");
        CHECK(*cow2 == "hello"); // cow2 unchanged
        CHECK(cow.unique());
        CHECK(cow2.unique());
    }
}

TEST_CASE("copy_on_write swap") {
    copy_on_write<int> cow1(42);
    copy_on_write<int> cow2(100);

    swap(cow1, cow2);

    CHECK(*cow1 == 100);
    CHECK(*cow2 == 42);
}

TEST_CASE("copy_on_write with complex types") {
    struct TestStruct {
        std::string name;
        int value;

        TestStruct(std::string n, int v) : name(std::move(n)), value(v) {}

        bool operator==(const TestStruct& other) const {
            return name == other.name && value == other.value;
        }

        bool operator<(const TestStruct& other) const {
            if (name != other.name) return name < other.name;
            return value < other.value;
        }
    };

    copy_on_write<TestStruct> cow(TestStruct{"test", 42});

    CHECK(cow->name == "test");
    CHECK(cow->value == 42);
    CHECK(cow.unique());

    copy_on_write<TestStruct> cow2(cow);
    CHECK(cow.identity(cow2));

    cow.write([](TestStruct& value) { value.value = 100; });
    CHECK(cow->value == 100);
    CHECK(cow2->value == 42);
    CHECK_FALSE(cow.identity(cow2));
}
