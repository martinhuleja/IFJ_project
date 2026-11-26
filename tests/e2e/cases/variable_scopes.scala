import ifj26.*

@main def main(): Unit =
  val x = 100
  var i = 0
  while i < 3 do
    val x = i + 10
    print(x)
    print("\n")
    i = i + 1
  print(x)
  print("\n")
  if x == 100 then
    val x = x + 1
    print(x)
    print("\n")
  else
    ()
  print(x)
  print("\n")
