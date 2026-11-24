import ifj26.*

def vypis(text: String, left: Int, right: Int, value: Double): Unit =
  val local = text + "!"
  print(local + "\n")
  print(left - right)
  print("\n")
  print(value + 0.5)
  print("\n")

def obal(): Unit =
  vypis("druhy", 20, 4, 3.5)

def odratavaj(n: Int): Unit =
  if n > 0 then
    print(n)
    print("\n")
    odratavaj(n - 1)
    print(n)
    print("\n")
  else
    ()

@main def main(): Unit =
  val text = "main"
  vypis("prvy", 8 + 2, 3, 2)
  obal()
  odratavaj(3)
  print(text + "\n")
