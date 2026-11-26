import ifj26.*

def vypis(text: String, left: Int, right: Int, value: Double): Unit =
  val local = text + "!"
  print(local)
  print("\n")
  val difference = left - right
  print(difference)
  print("\n")
  val increased = value + 0.5
  print(increased)
  print("\n")

def obal(): Unit =
  vypis("druhy", 20, 4, 3.5)

def odratavaj(n: Int): Unit =
  if n > 0 then
    print(n)
    print("\n")
    val next = n - 1
    odratavaj(next)
    print(n)
    print("\n")
  else
    ()

@main def main(): Unit =
  val text = "main"
  val argument = 8 + 2
  vypis("prvy", argument, 3, 2)
  obal()
  odratavaj(3)
  print(text)
  print("\n")
