import ifj26.*

@main def main(): Unit =
  var x = 0
  val limit = 3

  while x + 1 < limit * 2 do
    print(x)
    print("\n")
    x = x + 1

  if (x - 1) * 2 == limit + 5 then
    print("rovnake\n")
  else
    print("chyba\n")

  if -(x + 1) <= -(limit * 2) then
    print("hranica\n")
  else
    print("chyba\n")

  while x * 2 >= limit + 3 do
    print(x)
    print("\n")
    x = x - 1

  if x + 1 != limit then
    print("chyba\n")
  else
    print("koniec\n")