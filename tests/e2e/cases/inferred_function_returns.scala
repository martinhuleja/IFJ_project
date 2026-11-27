import ifj26.*

def bridge() = number()

def number() = 42

def decimal() = 2.5

def text() = "Ahoj"

def unused() = 123

def unit() = print("unit\n")

def echo(value: Int) = value

def choose(value: Int) =
  if value == 0 then
    print("L")
  else
    print("R")

@main def main() =
  val n = bridge()
  print(n)
  print("\n")
  val d = decimal()
  print(d)
  print("\n")
  val s = text()
  print(s)
  print("\n")
  val u = unit()
  print(u)
  print("\n")
  unit()
  val result = echo(7)
  print(result)
  print("\n")
  choose(0)
  print("\n")
  choose(1)
  print("\n")
  number()
  print("koniec\n")
