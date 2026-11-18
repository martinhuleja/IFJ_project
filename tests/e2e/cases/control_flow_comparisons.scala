import ifj26.*

@main def main(): Unit =
  var x = 2
  while x <= 4 do
    if x > 3 then
      print("GT ano\n")
    else
      print("GT nie\n")
    if x >= 3 then
      print("GE ano\n")
    else
      print("GE nie\n")
    if x <= 3 then
      print("LE ano\n")
    else
      print("LE nie\n")
    if x < 3 then
      print("LT ano\n")
    else
      print("LT nie\n")
    x = x + 1

  while x > 3 do
    print(x)
    print("\n")
    x = x - 1

  while x >= 2 do
    print(x)
    print("\n")
    x = x - 1

  print(x)
  print("\n")