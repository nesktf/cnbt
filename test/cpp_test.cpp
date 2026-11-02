#include <cnbt/cnbt.hpp>

#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string_view>

static void test_primitives() {
  cnbt::Tag b(static_cast<int8_t>(42));
  assert(b.is_byte());
  assert(b.byte().value() == 42);
  assert(!b.string().has_value());

  cnbt::Tag s(static_cast<int16_t>(1024));
  assert(s.is_short());
  assert(s.int16().value() == 1024);
  assert(s.short_val().value() == 1024);

  cnbt::Tag i(static_cast<int32_t>(123456));
  assert(i.is_int());
  assert(i.integer().value() == 123456);
  assert(i.int32().value() == 123456);

  cnbt::Tag l(static_cast<int64_t>(9876543210LL));
  assert(l.is_long());
  assert(l.int64().value() == 9876543210LL);
  assert(l.long_val().value() == 9876543210LL);

  cnbt::Tag f(3.14f);
  assert(f.is_float());
  assert(f.float32().value() == 3.14f);

  cnbt::Tag d(2.718281828);
  assert(d.is_double());
  assert(d.float64().value() == 2.718281828);

  cnbt::Tag str("hello cnbt");
  assert(str.is_string());
  assert(str.string().value() == "hello cnbt");

  const int8_t barr_data[] = {1, 2, 3};
  cnbt::Tag barr(cnbt::ArrayView<const int8_t>(barr_data, 3));
  assert(barr.is_byte_array());
  assert(barr.byte_array().value().size() == 3);
  assert(barr.byte_array().value()[1] == 2);

  const int32_t iarr_data[] = {10, 20, 30};
  cnbt::Tag iarr(cnbt::ArrayView<const int32_t>(iarr_data, 3));
  assert(iarr.is_int_array());
  assert(iarr.int_array().value().size() == 3);

  const int64_t larr_data[] = {100LL, 200LL};
  cnbt::Tag larr(cnbt::ArrayView<const int64_t>(larr_data, 2));
  assert(larr.is_long_array());
  assert(larr.long_array().value().size() == 2);
}

static void test_compound() {
  cnbt::TagCompound comp;
  assert(comp.empty());

  assert(comp.set_byte("byte_key", 7));
  assert(comp.set_short("short_key", 77));
  assert(comp.set_int("int_key", 777));
  assert(comp.set_long("long_key", 7777LL));
  assert(comp.set_float("float_key", 1.5f));
  assert(comp.set_double("double_key", 2.5));
  assert(comp.set_boolean("bool_key", true));
  assert(comp.set_string("str_key", "value"));

  assert(comp.size() == 8);
  assert(!comp.empty());
  assert(comp.contains("str_key"));
  assert(comp.has_key("int_key"));
  assert(!comp.contains("missing_key"));

  assert(comp.byte("byte_key").value() == 7);
  assert(comp.int16("short_key").value() == 77);
  assert(comp.integer("int_key").value() == 777);
  assert(comp.int64("long_key").value() == 7777LL);
  assert(comp.float32("float_key").value() == 1.5f);
  assert(comp.float64("double_key").value() == 2.5);
  assert(comp.boolean("bool_key").value() == true);
  assert(comp.string("str_key").value() == "value");

  auto missing = comp.string("missing_key");
  assert(!missing.has_value());
  assert(missing.code() == CNBT_INVALID_DATA);

  auto type_err = comp.integer("str_key");
  assert(!type_err.has_value());
  assert(type_err.code() == CNBT_INVALID_DATA);

  size_t iterated = 0;
  for (auto [key, tag] : comp) {
    assert(!key.empty());
    assert(tag.type() != CNBT_TYPE_END);
    ++iterated;
  }
  assert(iterated == 8);

  cnbt::TagCompound child;
  assert(child.set_string("child_str", "nested"));
  assert(comp.set_compound("nested_comp", std::move(child)));

  auto nested = comp.compound("nested_comp");
  assert(nested.has_value());
  assert((*nested)->string("child_str").value() == "nested");
}

static void test_list() {
  cnbt::TagList list;
  assert(list.empty());

  assert(list.append_int(100));
  assert(list.append_int(200));
  assert(list.append_int(300));

  assert(list.size() == 3);
  assert(!list.empty());

  assert(list[0].integer().value() == 100);
  assert(list.integer(1).value() == 200);
  assert(list.at(2).has_value());
  assert(!list.at(3).has_value());

  int sum = 0;
  for (const auto& tag : list) {
    sum += tag.integer().value();
  }
  assert(sum == 600);
}

static void test_monadic() {
  cnbt::TagCompound comp;
  assert(comp.set_int("num", 10));

  assert(comp.integer("num").value_or(0) == 10);
  assert(comp.integer("absent").value_or(999) == 999);

  auto res = comp.integer("num")
               .transform([](int32_t val) { return val * 2; })
               .and_then([](int32_t val) -> cnbt::Expected<int32_t> { return val + 5; });
  assert(res.has_value());
  assert(*res == 25);

  auto fallback =
    comp.integer("absent").or_else([](cnbt_Status) -> cnbt::Expected<int32_t> { return 123; });
  assert(fallback.has_value());
  assert(*fallback == 123);
}

static void test_copy_move_clone() {
  cnbt::TagCompound orig;
  assert(orig.set_string("key", "initial"));

  cnbt::TagCompound copy = orig;
  assert(copy.string("key").value() == "initial");
  assert(copy.set_string("key", "modified"));
  assert(copy.string("key").value() == "modified");
  assert(orig.string("key").value() == "initial");

  cnbt::TagCompound moved = std::move(copy);
  assert(moved.string("key").value() == "modified");

  auto clone_res = orig.clone();
  assert(clone_res.has_value());
  assert(clone_res->string("key").value() == "initial");
}

static void test_io() {
  cnbt::TagCompound comp;
  assert(comp.set_string("author", "Antigravity"));
  assert(comp.set_int("version", 1));

  const char* tmp_file = "/tmp/cnbt_test_cpp.nbt";
  assert(comp.write_compressed_file(tmp_file));

  auto loaded = cnbt::TagCompound::read_compressed_file(tmp_file);
  assert(loaded.has_value());
  assert(loaded->string("author").value() == "Antigravity");
  assert(loaded->integer("version").value() == 1);
  std::remove(tmp_file);

  std::stringstream ss;
  assert(comp.write_compressed_stream(ss));

  auto loaded_stream = cnbt::TagCompound::read_compressed_stream(ss);
  assert(loaded_stream.has_value());
  assert(loaded_stream->string("author").value() == "Antigravity");

  auto bigtest = cnbt::TagCompound::read_compressed_file(TESTFILES "/bigtest.nbt");
  if (bigtest.has_value()) {
    assert(bigtest->size() == 14);
    assert(bigtest->byte("byteTest").value() == 127);
    assert(bigtest->string("stringTest").value() == "HELLO WORLD THIS IS A TEST STRING ÅÄÖ!");
  }
}

int main() {
  test_primitives();
  test_compound();
  test_list();
  test_monadic();
  test_copy_move_clone();
  test_io();

  std::cout << "All C++ tests passed!\n";
  return 0;
}
