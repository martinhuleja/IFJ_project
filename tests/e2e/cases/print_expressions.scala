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
  print(2 + 3)
  print("\n")
  print("Ahoj " + "svet")
  print("\n")
  print(-(2.5 + 0.5))
  print("\n")
  print((x - 2) * (x + 1))
  print("\n")
  var i = 0
  while i < 3 do
    print(i + 10)
    print("\n")
    i = i + 1
