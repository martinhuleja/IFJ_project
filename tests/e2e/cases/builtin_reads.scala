import ifj26.*

def getNumber(): Int =
  readInt()

def showInt(value: Int): Unit =
  print(value)
  print("\n")

def showDouble(value: Double): Unit =
  print(value)
  print("\n")

@main def main(): Unit =
  val line = readLine()
  val integer = getNumber()
  val decimal = readDouble()
  val badInteger = readInt()
  val badDouble = readDouble()
  readInt()
  val afterDiscard = readInt()
  val eofLine = readLine()
  val eofInteger = readInt()
  val eofDouble = readDouble()
  val length = strLength(line)
  val eofLength = strLength(eofLine)
  print(line)
  showInt(length)
  showInt(integer)
  showDouble(decimal)
  showInt(badInteger)
  showDouble(badDouble)
  showInt(afterDiscard)
  showInt(eofLength)
  showInt(eofInteger)
  showDouble(eofDouble)
  print("koniec\n")
