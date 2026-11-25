import ifj26.*

def cut(text: String, start: Int, end: Int): String =
  subStr(text, start, end)

def show(value: String): Unit =
  print("[")
  print(value)
  print("]\n")

@main def main(): Unit =
  val text = "Ahoj"
  val middle = subStr(text, 1, 3)
  val full = subStr(text, 0, 4)
  val empty = subStr(text, 2, 2)
  val last = subStr(text, 3, 4)
  val fromFunction = cut("ABCD", 1, 3)
  var changed = subStr(text, 0, 1)
  changed = subStr(text, 1, 4)
  show(middle)
  show(full)
  show(empty)
  show(last)
  show(fromFunction)
  show(changed)
  var i = 0
  while i < 4 do
    val next = i + 1
    val single = subStr(text, i, next)
    show(single)
    i = next
  subStr(text, 0, 4)
  print("koniec\n")
