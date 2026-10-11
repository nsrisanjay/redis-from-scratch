
#include "storage/keyspace.hpp"

#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

int main() {
    Keyspace ks;

    // 1. Empty keyspace
    assert(ks.dbSize() == 0);

    // 2. SET and GET
    ks.set("name", "Sanjay");
    auto name = ks.get("name");
    assert(name.has_value());
    assert(name.value() == "Sanjay");

    // 3. Overwrite existing key
    ks.set("name", "Alex");
    assert(ks.get("name").value() == "Alex");

    // 4. Missing key
    assert(!ks.get("missing").has_value());

    // 5. Empty string is a valid value
    ks.set("empty", "");
    auto empty = ks.get("empty");
    assert(empty.has_value());
    assert(empty.value().empty());

    // 6. EXISTS
    assert(ks.exists("name"));
    assert(ks.exists("empty"));
    assert(!ks.exists("missing"));

    // 7. DEL existing key
    std::vector<std::string> oneKey{"name"};
    assert(ks.del(oneKey) == 1);
    assert(!ks.exists("name"));

    // 8. DEL missing key
    std::vector<std::string> missingKey{"missing"};
    assert(ks.del(missingKey) == 0);

    // 9. DEL multiple keys, only some exist
    ks.set("a", "1");
    ks.set("b", "2");
    std::vector<std::string> multiple{"a", "b", "missing"};
    assert(ks.del(multiple) == 2);

    // 10. Duplicate keys count only once
    ks.set("duplicate", "value");
    std::vector<std::string> duplicates{
        "duplicate", "duplicate"
    };
    assert(ks.del(duplicates) == 1);

    // 11. Deleting from an empty keyspace
    std::vector<std::string> emptyKeys;
    assert(ks.del(emptyKeys) == 0);

    // 12. DBSIZE reflects current contents
    assert(ks.dbSize() == 1);
    assert(ks.exists("empty"));

    std::cout << "All Keyspace tests passed!\n";
    return 0;
}
