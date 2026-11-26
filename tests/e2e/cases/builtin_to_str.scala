import ifj26.*

@main def main(): Unit =
  val negativeInt = -7
  val negativeDouble = -3.5
  val unit = ()
  val integerText = toStr(42)
  val negativeIntegerText = toStr(negativeInt)
  val doubleText = toStr(3.5)
  val negativeDoubleText = toStr(negativeDouble)
  val original = toStr("Ahoj")
  val unitText = toStr(unit)
  var changed = toStr(1)
  changed = toStr(2.5)
  val combined = integerText + "!"
  print(integerText)
  print("\n")
  print(negativeIntegerText)
  print("\n")
  print(doubleText)
  print("\n")
  print(negativeDoubleText)
  print("\n")
  print(original)
  print("\n")
  print(unitText)
  print("\n")
  print(changed)
  print("\n")
  print(combined)
  print("\n")
  toStr(99)
  print("koniec\n")
