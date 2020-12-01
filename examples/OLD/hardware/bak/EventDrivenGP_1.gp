Fn-00000000:
  Nop
  Nop
  Nop
  SetMem(1, 16)
  While(1)
  Output(1)
  Call[01010101]
  Close

Fn-1:
  Nop
  Nop
  Nop

Fn-01010101:
  Input(1, 1)
  Mult(1, 1)
  Commit(1, 0)
