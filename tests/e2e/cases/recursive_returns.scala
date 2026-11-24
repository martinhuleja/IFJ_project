import ifj26.*

def factorial(n: Int): Int =
  if n == 0 then
    return 1
  else
    val smaller = factorial(n - 1)
    return n * smaller

def fibonacci(n: Int): Int =
  if n < 2 then
    n
  else
    val first = fibonacci(n - 1)
    val second = fibonacci(n - 2)
    first + second

def sumTo(n: Int): Int =
  var total = 0
  var i = 0
  while i <= n do
    total = total + i
    i = i + 1
  total

def firstAbove(limit: Int): Int =
  var i = 0
  while i < 5 do
    if i > limit then
      return i
    else
      ()
    i = i + 1
  -1

@main def main(): Unit =
  val fact = factorial(5)
  val fib = fibonacci(6)
  val total = sumTo(5)
  val found = firstAbove(2)
  val missing = firstAbove(10)
  print(fact)
  print("\n")
  print(fib)
  print("\n")
  print(total)
  print("\n")
  print(found)
  print("\n")
  print(missing)
  print("\n")
