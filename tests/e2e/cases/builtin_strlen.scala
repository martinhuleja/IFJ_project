import ifj26.*

def lengthOf(text: String): Double =
  val length = strLength(text)
  length

@main def main(): Unit =
  val text = "Ahoj svet"
  val extended = text + "!"
  val empty = strLength("")
  val literal = strLength("Ahoj")
  var length = strLength(text)
  print(empty)
  print("\n")
  print(literal)
  print("\n")
  print(length)
  print("\n")
  length = strLength(extended)
  print(length)
  print("\n")
  val escaped = strLength("A B#\\\n")
  print(escaped)
  print("\n")
  val promoted: Double = strLength("abc")
  print(promoted)
  print("\n")
  val returned = lengthOf("abcde")
  print(returned)
  print("\n")
  strLength("ignorovany vysledok")
  print("koniec\n")
