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

A *func-item* has some `signature` : *signature-item* and some `body` : *block-item*

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
`type` : *type-item*.

A *loop-item* has `descr` : `loop` (and of course some `body`, but no
`locals`) and may have a `while` : *expr* (whose computed type cannot
be `unit` or nil).

[GCCJIT]: http://gcc.gnu.org/onlinedocs/jit/

