import ifj26.*

def show(n: Int): Unit =
  if n == 0 then
    return
  else
    ()
  print(n)
  print("\n")
  return n + 1
  print("NEVER function\n")

def noValue(): Unit =
  return ()

def ignoreExpression(n: Int): Unit = n + 1

def unitArgument(value: Unit): Unit =
  print(value)
  print("\n")

@main def main(): Unit =
  show(0)
  show(7)
  noValue()
  val returned: Unit = noValue()
  print(returned)
  print("\n")
  val literal: Unit = ()
  print(literal)
  print("\n")
  unitArgument(())
  ignoreExpression(3)
  42
  "ignored"
  print("koniec\n")
  return
  print("NEVER main\n")
