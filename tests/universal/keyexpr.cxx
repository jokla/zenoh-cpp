//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//
#include "zenoh.hxx"
using namespace zenoh;

#undef NDEBUG
#include <assert.h>

void key_expr_view() {
    KeyExprView nul(nullptr);
    assert(!nul.check());
    KeyExprView nulstr((const char*)nullptr);
    assert(!nulstr.check());

    KeyExprView foo("FOO");
    assert(foo.check());
    assert(foo == "FOO");
    assert(foo.as_bytes() == "FOO");
    assert(foo.as_string_view() == "FOO");

    std::string sfoo("FOO");
    KeyExprView ksfoo(sfoo);
    assert(ksfoo.check());
    assert(ksfoo == "FOO");
    assert(ksfoo.as_bytes() == "FOO");
    assert(ksfoo.as_string_view() == "FOO");

#ifdef ZENOHCXX_ZENOHC
    std::string_view svfoo("FOOBAR", 3);
    KeyExprView ksvfoo(svfoo);
    assert(ksvfoo.check());
    assert(ksvfoo == "FOO");
    assert(ksvfoo.as_bytes() == "FOO");
    assert(ksvfoo.as_string_view() == "FOO");

    KeyExprView unchecked("a/*", KeyExprUnchecked());
    assert(unchecked.check());
    assert(unchecked.as_string_view() == "a/*");

    std::string_view sunchecked("a/*//*", 3);
    KeyExprView svunchecked(sunchecked, KeyExprUnchecked());
    assert(svunchecked.as_string_view() == "a/*");
#endif
}

void key_expr() {
    KeyExpr nul(nullptr);
    assert(!nul.check());
    KeyExpr nulstr((const char*)nullptr);
    assert(!nulstr.check());

    KeyExpr foo("FOO");
    assert(foo.check());
    assert(foo == "FOO");
    assert(foo.as_bytes() == "FOO");
    assert(foo.as_string_view() == "FOO");
    assert(foo.as_keyexpr_view() == "FOO");
}

void canonize() {
    ErrNo err;
    bool res;
    auto non_canon = "a/**/**/c";
    auto canon = "a/**/c";

    assert(keyexpr_is_canon(canon));
    assert(!keyexpr_is_canon(non_canon));
    assert(keyexpr_is_canon(canon, err));
    assert(err == 0);
    assert(!keyexpr_is_canon(non_canon, err));
    assert(err != 0);

    std::string foo(non_canon);
    res = keyexpr_canonize(foo, err);
    assert(foo == canon);
    assert(err == 0);
    assert(res);
}

void concat() {
#ifdef ZENOHCXX_ZENOHC
    assert(keyexpr_concat("FOO", "BAR") == "FOOBAR");
    assert(keyexpr_concat(KeyExpr("FOO"), "BAR") == "FOOBAR");
    assert(keyexpr_concat(KeyExprView("FOO"), "BAR") == "FOOBAR");
#endif
}

void join() {
#ifdef ZENOHCXX_ZENOHC
    assert(keyexpr_join("FOO", "BAR") == "FOO/BAR");
    assert(keyexpr_join(KeyExpr("FOO"), "BAR") == "FOO/BAR");
    assert(keyexpr_join(KeyExprView("FOO"), "BAR") == "FOO/BAR");
    assert(keyexpr_join("FOO", KeyExpr("BAR")) == "FOO/BAR");
    assert(keyexpr_join(KeyExpr("FOO"), KeyExpr("BAR")) == "FOO/BAR");
    assert(keyexpr_join(KeyExprView("FOO"), KeyExpr("BAR")) == "FOO/BAR");
    assert(keyexpr_join("FOO", KeyExprView("BAR")) == "FOO/BAR");
    assert(keyexpr_join(KeyExpr("FOO"), KeyExprView("BAR")) == "FOO/BAR");
    assert(keyexpr_join(KeyExprView("FOO"), KeyExprView("BAR")) == "FOO/BAR");
#endif
}

void equals() {
    KeyExprView nul(nullptr);
    ErrNo err;

    assert(keyexpr_equals("FOO", "FOO"));
    assert(keyexpr_equals("FOO", "FOO", err));
    assert(err == 0);

    assert(!keyexpr_equals("FOO", "BAR"));
    assert(!keyexpr_equals("FOO", "BAR", err));
    assert(err == 0);

    assert(!keyexpr_equals("FOO", nul));
    assert(!keyexpr_equals("FOO", nul, err));
    assert(err < 0);
}

void includes() {
    KeyExprView nul(nullptr);
    ErrNo err;

    assert(keyexpr_includes("FOO/*", "FOO/BAR"));
    assert(keyexpr_includes("FOO/*", "FOO/BAR", err));
    assert(err == 0);

    assert(!keyexpr_includes("FOO/BAR", "FOO/*"));
    assert(!keyexpr_includes("FOO/BAR", "FOO/*", err));
    assert(err == 0);

    assert(!keyexpr_includes("FOO/*", nul));
    assert(!keyexpr_includes("FOO/*", nul, err));
    assert(err < 0);

    KeyExpr foo("FOO");
    assert(keyexpr_includes(foo, "FOO/BAR"));
    assert(keyexpr_includes(foo, "FOO/BAR", err));
    assert(err == 0);
    assert(!keyexpr_includes("FOO/BAR", foo));
    assert(!keyexpr_includes("FOO/BAR", foo, err));
    assert(err == 0);
}

void intersects() {
    KeyExprView nul(nullptr);
    ErrNo err;

    assert(keyexpr_intersects("FOO/*", "FOO/BAR"));
    assert(keyexpr_intersects("FOO/*", "FOO/BAR", err));
    assert(err == 0);

    assert(!keyexpr_intersects("*/BUZ", "FOO/BAR"));
    assert(!keyexpr_intersects("*/BUZ", "FOO/BAR", err));
    assert(err == 0);

    assert(!keyexpr_intersects("FOO/*", nul));
    assert(!keyexpr_intersects("FOO/*", nul, err));
    assert(err < 0);

    KeyExpr foobar("FOO/BAR");
    assert(keyexpr_intersects("FOO/*", foobar));
    assert(keyexpr_intersects("FOO/*", foobar, err));
    assert(err == 0);
}

#include <variant>

void undeclare() {
    Config config;
    auto session = open(std::move(config));
    if (auto psession = std::get_if<Session>(&session)) {
        auto keyexpr = psession->declare_keyexpr("foo/bar");
        assert(keyexpr.check());
        ErrNo err;
        assert(psession->undeclare_keyexpr(std::move(keyexpr), err));
        assert(err == 0);
        assert(!keyexpr.check());
    } else {
        auto error = std::get<ErrorMessage>(session);
        std::cerr << "Error: " << error.as_string_view() << std::endl;
// zenohpico is unable to open session without zenoh router started
#ifdef ZENOHCXX_ZENOHC
        assert(false);
#endif
    }
}

int main(int argc, char** argv) {
    key_expr_view();
    key_expr();
    canonize();
    concat();
    equals();
    includes();
    intersects();
    undeclare();
};
