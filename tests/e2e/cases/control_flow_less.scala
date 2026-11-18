import ifj26.*

@main def main(): Unit =
  var x = 2
  if x < 3 then
    print("mensie\n")
  else
    print("chyba\n")

  x = 3
  if x < 3 then
    print("chyba\n")
  else
    print("rovnake\n")

  x = 4
  if x < 3 then
    print("chyba\n")
  else
    print("vacsie\n")

  x = 3
  print("nula iteracii\n")
  while x < 3 do
    print("chyba\n")
    x = x + 1

  x = 2
  print("jedna iteracia\n")
  while x < 3 do
    print(x)
    print("\n")
    x = x + 1

  x = 0
  print("tri iteracie\n")
  while x < 3 do
    print(x)
    print("\n")
    x = x + 1