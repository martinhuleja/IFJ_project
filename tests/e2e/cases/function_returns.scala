import ifj26.*

def relay(a: Int, b: Int): Int =
  return sum(a, b)

def sum(a: Int, b: Int): Int = a + b

def explicitSum(a: Int, b: Int): Int =
  return a + b

def promoted(a: Int, b: Int): Double = sum(a, b)

def greeting(name: String): String =
  val prefix = "Ahoj "
  prefix + name + "!"

def maximum(a: Int, b: Int): Int =
  if a > b then
    return a
  else
    return b

def nested(n: Int): Double =
  if n < 0 then
    if n < -2 then
      return 1
    else
      return 2.5
  else
    val base = 2
    return base * 2

@main def main(): Unit =
  val first = sum(2, 3)
  print(first)
  print("\n")
  var result = explicitSum(10, 4)
  print(result)
  print("\n")
  result = relay(1, 2)
  sum(99, 1)
  print(result)
  print("\n")
  val converted = promoted(2, 3)
  print(converted)
  print("\n")
  val stored: Double = sum(1, 2)
  print(stored)
  print("\n")
  val text = greeting("svet")
  print(text)
  print("\n")
  val left = maximum(9, 2)
  val right = maximum(3, 8)
  print(left)
  print("\n")
  print(right)
  print("\n")
  val negativeThree = -3
  val negativeOne = -1
  val below = nested(negativeThree)
  val middle = nested(negativeOne)
  val above = nested(0)
  print(below)
  print("\n")
  print(middle)
  print("\n")
  print(above)
  print("\n")
