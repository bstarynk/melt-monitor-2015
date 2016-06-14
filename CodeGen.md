<!-- -*- markdown -*- -->
# Code Generation

A *module-item* can be translated to C code or JavaScript code (and
perhaps in the future using [GCCJIT][]), using the `mom_emit_c_code`
and `mom_emit_javascript_code` functions. On successful translation,
several items may get updated with various meta data about the emitted
code. The translator is locking every item it is processing.

Most items processed by the code emitters have some `descr` attribute whose value is some descriptor item.

There is no alpha-conversion. Each item is (may become) bound to a single role. So
a variable cannot be redefined.

A *module-item* has `descr` : `module` and `module` :
*module-component-sequence*

A *module-component* is an item. It can be a *data-item* (of `descr` :
`data`), or a *func-item* (of `descr` : `func`), or a *routine-item*
(of `descr` : `routine`).

A *func-item* has some `signature` : *signature-item* and some `body` : *block-item*; the first formal is required to be `the_closure`.

A *signature-item* has `descr` : `signature` and some `formals` :
*formals-tuple* (with *formal-item*s inside), with an optional
`result` : *type-item* or *result-type-tuple* made of *type-item*s.

A *formal-item* has `descr` : `formal` and `type` : *type-item*. The *type-item* cannot be `unit`.

A *block-item* is either an *instructionseq-item* or a *loop-item*. Both have
a `body` : *instructions-tuple* which is a tuple made of
*instruction-item*s.

An *instructionseq-item* has `descr` : `sequence` (and of course some
`body` but no `while`) and may have a `locals` : *locals-tuple* made of
*variable-item*s (implicitly cleared). Each *variable-item* has a
`type` : *type-item* and `descr`: `variable`

A *loop-item* has `descr` : `loop` (and of course some `body`, but no
`locals`) and may have a `while` : *expr* (whose computed type cannot
be `unit` or nil).

An *instruction-item* is one of:

* an *assignment-item* with `descr`: `assign`, `to`: some
  *variable-item*, `from`: some *expression* of compatible type. A
  missing `from` is taken as a null or cleared value.

* a *break-item* with `descr`: `break` and `block`: some enclosing *block-item*

* a *continue-item* with  `descr`: `continue` and `loop`: some enclosing *loop-item*

* a nested fresh *block-item* 

* a *condition-item* with `descr` : `cond` and `cond`: a tuple of
non-nil *test-item*s ; each *test-item* has `descr`: `test`, `test`:
*expression*, `then`: *instruction-item*

a *call-item* with `descr` : `call`, `func` : an expression giving the
function to call, an optional `result` : a variable item or tuple of
variables; the arguments are the components of the instruction.

a *run-item* with `descr` : `run`, `run` : a node whose connective
is a primitive, and optional `result` as before.

a *switch-item* with `descr` : `switch`, `type` : `int` or `item` or
`string`, `switch` : an expression, and `case` : a tuple or set of
*case-item*s, optionally `otherwise` : a block or instruction


An *expression* is one of:

* a literal *int*, or *double*, or *string*

* a *variable-item*, if it has a `descr` : `variable`, `global`, `thread_local`, `formal`

* a *closed-item* if it has a `descr` : `closed` (all closed items are
  of type `value`).

* any other item is considered literally (as a constant)

* a *node*, see below.


A node expression is handled according to its connective. Most node
expressions are of fixed arity, with variadic expressions being the
exception. In particular:

* The `verbatim` connective requires one argument, handled as a "quoted" constant

* The `and` & `or` connectives are for *and-then* (like `&&` in C) or
  *or-else* (like `||` in C) variadic expressions

* The `sequence` connective requires at least one son. It is like `progn` in Lisp or comma operator in C, so is variadic

* The `tuple` and `set` connectives are variadic expressions to build
  tuples and sets from item arguments.

* If the connective is some *signature-item*, the expression is a closure application.

* If the connective is some *routine-item*, the expression is a routine call. Arguments should obey the type of its `signature`

* If the connective is a *primitive-item* of `descr` : `primitive` and
  `signature` : *signature-item*, the expression is a primitive
  invocation.  Arguments should obey the type of its *signature-item*

[GCCJIT]: http://gcc.gnu.org/onlinedocs/jit/

