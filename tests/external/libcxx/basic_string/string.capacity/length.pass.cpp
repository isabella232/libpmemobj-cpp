//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Copyright 2019-2020, Intel Corporation
//
// Modified to test pmem::obj containers
//

#include "unittest.hpp"

#include <libpmemobj++/container/string.hpp>

namespace nvobj = pmem::obj;

using S = pmem::obj::string;

struct root {
	nvobj::persistent_ptr<S> s_arr[4];
};

template <class S>
void
test(const S &s)
{
	UT_ASSERT(s.length() == s.size());
}

static void
test(int argc, char *argv[])
{
	if (argc < 2) {
		UT_FATAL("usage: %s file-name", argv[0]);
	}

	auto path = argv[1];
	auto pop = nvobj::pool<root>::create(
		path, "string_test", PMEMOBJ_MIN_POOL, S_IWUSR | S_IRUSR);

	auto r = pop.root();
	{
		auto &s_arr = r->s_arr;
		try {
			nvobj::transaction::run(pop, [&] {
				s_arr[0] = nvobj::make_persistent<S>();
				s_arr[1] = nvobj::make_persistent<S>("123");
				s_arr[2] = nvobj::make_persistent<S>(
					"12345678901234567890123456789012345678901234567890");
				s_arr[3] = nvobj::make_persistent<S>(
					"1234567890123456789012345678901234567890123456789012345678901234567890");
			});

			test(*s_arr[0]);
			test(*s_arr[1]);
			test(*s_arr[2]);
			test(*s_arr[3]);

			nvobj::transaction::run(pop, [&] {
				for (unsigned i = 0; i < 4; ++i) {
					nvobj::delete_persistent<S>(s_arr[i]);
				}
			});
		} catch (std::exception &e) {
			UT_FATALexc(e);
		}
	}

	pop.close();
}

int
main(int argc, char *argv[])
{
	return run_test([&] { test(argc, argv); });
}
