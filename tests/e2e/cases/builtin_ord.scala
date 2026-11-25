import ifj26.*

def codeAt(text: String, index: Int): Int =
  ord(text, index)

def show(value: Int): Unit =
  print(value)
  print("\n")

@main def main(): Unit =
  val text = "ABC"
  val first = ord(text, 0)
  val last = ord(text, 2)
  val outside = ord(text, 3)
  val negativeIndex = -1
  val negative = ord(text, negativeIndex)
  val empty = ord("", 0)
  val zeroChar = chr(0)
  val boundaryChar = chr(127)
  val zeroCode = ord(zeroChar, 0)
  val boundaryCode = ord(boundaryChar, 0)
  val fromFunction = codeAt(text, 1)
  show(first)
  show(last)
  show(outside)
  show(negative)
  show(empty)
  show(zeroCode)
  show(boundaryCode)
  show(fromFunction)
  var i = 0
  while i < 4 do
    val current = ord(text, i)
    show(current)
    i = i + 1
  ord(text, 0)
  ord("", 0)
  print("koniec\n")
