import ifj26.*

@main def main(): Unit =
  val value = 3.5
  if value < 4 then
    print("LT ano\n")
  else
    print("chyba LT\n")
  if 3 <= value then
    print("LE ano\n")
  else
    print("chyba LE\n")
  if 4 > value then
    print("GT ano\n")
  else
    print("chyba GT\n")
  if value >= 4 then
    print("chyba GE\n")
  else
    print("GE nie\n")
  if value == 3 then
    print("chyba EQ\n")
  else
    print("EQ nie\n")
  if 3 != value then
    print("NE ano\n")
  else
    print("chyba NE\n")
  var counter: Double = 0
  while counter < 3 do
    print(counter)
    print("\n")
    counter = counter + 1
