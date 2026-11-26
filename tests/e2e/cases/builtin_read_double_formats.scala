import ifj26.*

@main def main(): Unit =
  readDouble()
  var value = 0.0
  var i = 0
  while i < 10 do
    value = readDouble()
    print(value)
    print("\n")
    i = i + 1
