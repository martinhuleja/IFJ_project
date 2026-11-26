import ifj26.*

@main def main(): Unit =
  val a = 10
  val b = 3
  val c = 4
  val d = 1
  var x = (a - b) * (c + d)
  print(x)
  print("\n")

  x = (x + 1) + (x - 1)
  print(x)
  print("\n")

  x = a - (b - c)
  print(x)
  print("\n")

  x = a / (b + 2)
  print(x)
  print("\n")

  x = -(a + b) * (c - d)
  print(x)
  print("\n")

  x = -17 / 3
  print(x)
  print("\n")

  x = 17 / -3
  print(x)
  print("\n")