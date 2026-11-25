import ifj26.*

@main def main(): Unit =
  val first = "Ahoj"
  val second = "svet"
  var result = first + " " + second + "!"
  print(result)
  print("\n")
  result = ("A" + "B") + ("C" + "D")
  print(result)
  print("\n")
  result = "" + result + "!"
  result = result + ""
  print(result)
  print("\n")
  val x = 7
  val sum = 2 + 3
  print(sum)
  print("\n")
  val greeting = "Ahoj " + "svet"
  print(greeting)
  print("\n")
  val negative = -(2.5 + 0.5)
  print(negative)
  print("\n")
  val product = (x - 2) * (x + 1)
  print(product)
  print("\n")
  var i = 0
  while i < 3 do
    val current = i + 10
    print(current)
    print("\n")
    i = i + 1
