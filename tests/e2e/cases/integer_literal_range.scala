import ifj26.*

def identity(value: Int): Int = value

def large() = 1234567890123

@main def main(): Unit =
  val positive = 2147483648
  val negative = -2147483648
  val big = 1234567890123
  val maximum = 9223372036854775807
  print(positive)
  print("\n")
  print(negative)
  print("\n")
  print(big)
  print("\n")
  print(maximum)
  print("\n")
  val passed = identity(1234567890123)
  print(passed)
  print("\n")
  val returned = large()
  print(returned)
  print("\n")
  print(1234567890123)
  print("\n")
  if big > 1000000000000 then
    print("large\n")
  else
    print("small\n")
