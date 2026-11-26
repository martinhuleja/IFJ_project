import ifj26.*

@main def main(): Unit =
  val positive = toInt(3.9)
  val negativeValue = -3.9
  val negative = toInt(negativeValue)
  val small = toInt(0.9)
  val whole = toInt(7)
  var changed = toInt(1.2)
  changed = toInt(8.9)
  print(positive)
  print("\n")
  print(negative)
  print("\n")
  print(small)
  print("\n")
  print(whole)
  print("\n")
  print(changed)
  print("\n")
  toInt(99.9)
  print("koniec\n")
