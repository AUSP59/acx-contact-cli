#include "utils.hpp"
#include "jsonl.hpp"
#include "validation.hpp"
#include "model.hpp"
#include "storage.hpp"
#include "hmac.hpp"
#include "csv.hpp"
#include "disk_index.hpp"
#include <cassert>
#include <iostream>

using namespace acx;

int main(){
    // SHA-256 KAT
    assert(sha256_hex("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    // JSON roundtrip
    J obj = std::map<std::string,J>{{"a",1.0},{"b",true},{"c","x"}};
    std::string dumped = obj.dump(2);
    size_t idx=0; (void)idx;
    J parsed = J::parse(dumped);

    // Validation
    assert(is_valid_email("user@example.com"));
    assert(!is_valid_email("bad@@example"));
    assert(is_valid_phone("+12345678901"));

    // Model serialization
    HistoryEntry h{ now_iso8601_utc(), "tester", "created" };
    Contact c{ uuid_v4(), "Test User", "user@example.com", "+12345678901", "note", now_iso8601_utc(), {h} };
    J jj = c.to_json();
    Contact c2 = Contact::from_json(jj);
    assert(c2.email == "user@example.com");

    std::cout << "Selftests passed.\n";
    return 0;
}

    // HMAC known vectors
    assert(hmac_sha256_hex("key","data") == std::string("5031fe3d989c6d1537a013fa6e739da23463fdaec3b70137d828e36ace221bd0"));
    assert(hmac_sha256_hex("secret","hello") == std::string("88aab3ede8d3adf94d26ab90d3bafd4a2083070c3bcce9c014ee04a443847c0b"));

    // CSV RFC4180 basic
    {
        std::string csv = "id,name,email,phone,note,created_at\r\n"
                          "1,\"Doe, John\",john@example.com,+12025550101,\"note \"\"x\"\"\",2025-01-01T00:00:00Z\r\n";
        std::vector<std::vector<std::string>> out; std::string err;
        assert(csv_parse_rfc4180(csv, out, err));
        assert(out.size()==2);
        assert(out[1][1]=="Doe, John");
        assert(out[1][4]=="note \"x\"");
    }
