[**< Index**](README.md)

# Tables

In order to build meaningful SQL statements with sqlpp23, you need to represent
tables and their columns as structs that can be understood by the library.

The default way to do so is by using a code generator to translate DDL to C++. A
code generator covering a lot of use cases can be found
[here](https://github.com/rbock/sqlpp23/blob/main/scripts/ddl2cpp). There is
also a specific one for
[sqlite3](https://github.com/rbock/sqlpp23/blob/main/scripts/sqlite2cpp.py).

If you look at the output, you will see why a generator is helpful. Here is a
[sample](https://github.com/rbock/sqlpp23/blob/main/tests/core/usage/Sample.h).

### Tables

This is the most simple case.

```C++
select(all_of(foo)).from(foo).where(foo.id == 17);
```

### Aliased Tables

Table aliases are useful in self-joins.

```C++
// Outside of functions
SQLPP_ALIAS_PROVIDER(left);
SQLPP_ALIAS_PROVIDER(right);
[...]

// Inside a function
auto l = foo.as(left);
auto r = foo.as(right);
select(all_of(l)).from(l.join(r).on(l.x == r.y)).unconditionally();
```

Aliased tables might also be used to increase the readability of generated SQL
code, for instance if you have very long table names.

### Joins

You can join two tables like this:

```C++
foo.join(bar).on(foo.id == bar.foo);
```

If you want to join more tables, you can chain joins.

```C++
foo.join(bar).on(foo.id == bar.foo).left_outer_join(baz).on(bar.id == baz.ref);
```

The following join types are supported:

- `join`,
- `inner_join` (this is an alias of join),
- `left_outer_join`,
- `right_outer_join`,
- `full_outer_join`,
- `cross_join` (this is the only join that does not require/allow an `on`
  condition).

[**< Index**](README.md)
