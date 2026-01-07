# Seam Programming Language

Seam is a dynamically-typed, interpreted programming language designed to be simple yet powerful. It supports both functional and object-oriented programming paradigms, featuring first-class functions, closures, structs with inheritance, and an interactive REPL.

<div align="center">

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE-MIT)
[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE-APACHE)

</div>

## Features

- **Dynamic typing** with intuitive type coercion
- **First-class functions** and closures
- **Object-oriented programming** with structs, methods, and inheritance
- **Static methods** for utility functions on structs
- **Interactive REPL** with command history
- **Clean, expressive syntax** inspired by modern languages

## Quick Start

**Interactive REPL:**
```bash
./seam
```

**Execute a script:**
```bash
./seam run script.seam
```

**Show version:**
```bash
./seam version
```

## Language Guide

### Variables

Declare variables with `let`:

```seam
let name = "Seam";
let age = 1;
let pi = 3.14159;
let active = true;
let nothing = nil;

// Reassignment
age = 2;
```

### Data Types

| Type | Example |
|------|---------|
| Number | `42`, `3.14` |
| String | `"hello world"` |
| Boolean | `true`, `false` |
| Null | `nil` |

### Operators

```seam
// Arithmetic
let sum = 10 + 5;       // 15
let diff = 10 - 5;      // 5
let product = 10 * 5;   // 50
let quotient = 10 / 5;  // 2

// Comparison
10 == 10    // true
10 != 5     // true
10 > 5      // true
10 < 5      // false
10 >= 10    // true
10 <= 5     // false

// Logical
true and false  // false
true or false   // true
!true           // false

// Ternary
let result = age > 18 ? "adult" : "minor";
```

### Control Flow

**If/Else:**
```seam
if (score >= 90) {
    print "A";
} else if (score >= 80) {
    print "B";
} else {
    print "C";
}
```

**While Loop:**
```seam
let i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
```

**For Loop:**
```seam
for (let i = 0; i < 5; i = i + 1) {
    print i;
}
```

### Functions

**Named Functions:**
```seam
fn greet(name) {
    return "Hello, " + name + "!";
}

print greet("World");  // Hello, World!
```

**Multiple Parameters:**
```seam
fn add(a, b) {
    return a + b;
}

print add(2, 3);  // 5
```

**Lambda Expressions:**
```seam
let multiply = fn(a, b) {
    return a * b;
};

print multiply(4, 5);  // 20
```

**Closures:**
```seam
fn makeCounter() {
    let count = 0;
    return fn() {
        count = count + 1;
        return count;
    };
}

let counter = makeCounter();
print counter();  // 1
print counter();  // 2
print counter();  // 3
```

**Higher-Order Functions:**
```seam
fn applyTwice(f, x) {
    return f(f(x));
}

fn double(n) {
    return n * 2;
}

print applyTwice(double, 5);  // 20
```

### Structs

**Basic Struct:**
```seam
struct Point {
    init(x, y) {
        self.x = x;
        self.y = y;
    }

    display() {
        print "(" + self.x + ", " + self.y + ")";
    }
}

let p = Point(10, 20);
p.display();  // (10, 20)
```

**Methods:**
```seam
struct Rectangle {
    init(width, height) {
        self.width = width;
        self.height = height;
    }

    area() {
        return self.width * self.height;
    }

    perimeter() {
        return 2 * (self.width + self.height);
    }
}

let rect = Rectangle(5, 3);
print rect.area();       // 15
print rect.perimeter();  // 16
```

**Static Methods:**
```seam
struct Math {
    static square(n) {
        return n * n;
    }

    static max(a, b) {
        return a > b ? a : b;
    }
}

print Math.square(5);    // 25
print Math.max(10, 20);  // 20
```

### Inheritance

Use `+` to add a parent struct:

```seam
struct Animal {
    init(name) {
        self.name = name;
    }

    speak() {
        print self.name + " makes a sound";
    }
}

struct Dog + Animal {
    init(name, breed) {
        parent.init(name);
        self.breed = breed;
    }

    speak() {
        print self.name + " barks!";
    }

    fetch() {
        print self.name + " fetches the ball";
    }
}

let dog = Dog("Buddy", "Labrador");
dog.speak();  // Buddy barks!
dog.fetch();  // Buddy fetches the ball
```

**Calling Parent Methods:**
```seam
struct Cat + Animal {
    speak() {
        parent.speak();  // Call parent's speak
        print "Meow!";
    }
}

let cat = Cat("Whiskers");
cat.speak();
// Whiskers makes a sound
// Meow!
```

### Comments

```seam
// Single-line comment

/*
   Multi-line
   comment
*/

/* Nested /* comments */ are supported */
```

### Print Statement

```seam
print "Hello, World!";
print 42;
print true;
print 2 + 2;
```

## Example Programs

### Fibonacci Sequence

```seam
fn fib(n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

for (let i = 0; i < 10; i = i + 1) {
    print fib(i);
}
// Output: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34
```

### Factorial

```seam
fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

print factorial(5);  // 120
```

### Bank Account

```seam
struct BankAccount {
    init(owner, balance) {
        self.owner = owner;
        self.balance = balance;
    }

    deposit(amount) {
        self.balance = self.balance + amount;
        print "Deposited " + amount;
    }

    withdraw(amount) {
        if (amount > self.balance) {
            print "Insufficient funds";
            return false;
        }
        self.balance = self.balance - amount;
        print "Withdrew " + amount;
        return true;
    }

    getBalance() {
        return self.balance;
    }
}

let account = BankAccount("Alice", 1000);
account.deposit(500);
account.withdraw(200);
print account.getBalance();  // 1300
```

### Linked List

```seam
struct Node {
    init(value) {
        self.value = value;
        self.next = nil;
    }
}

struct LinkedList {
    init() {
        self.head = nil;
    }

    append(value) {
        let node = Node(value);
        if (self.head == nil) {
            self.head = node;
            return;
        }
        let current = self.head;
        while (current.next != nil) {
            current = current.next;
        }
        current.next = node;
    }

    printList() {
        let current = self.head;
        while (current != nil) {
            print current.value;
            current = current.next;
        }
    }
}

let list = LinkedList();
list.append(1);
list.append(2);
list.append(3);
list.printList();  // 1, 2, 3
```


## Project Structure

```
seam/
├── src/
│   ├── main.cxx          # Entry point
│   ├── seam.ixx          # REPL and script runner
│   ├── scanner.ixx       # Lexical analyzer
│   ├── parser.ixx        # Syntax parser
│   ├── ast.ixx           # AST definitions
│   ├── interpreter.ixx   # Execution engine
│   ├── resolver.ixx      # Semantic analysis
│   ├── runtime.ixx       # Runtime environment
│   └── error.ixx         # Error handling
├── tests/                # Test suite
├── CMakeLists.txt        # Build configuration
└── vcpkg.json            # Dependencies
```


## Building from Source

### Requirements

- vcpkg
- CMake
- Clang++
- Ninja

### Steps

1. Export `VCPKG_ROOT` to your vcpkg directory.
2. Configure the project using `cmake --preset release`.
3. Build the project using `cmake --build build/release`.
4. Run the REPL using `./build/release/seam`.


### Running Tests

Run the test suite using `./build/release/seam_tests`.

## License

Seam is dual-licensed under the [MIT License](LICENSE-MIT) and the [Apache License 2.0](LICENSE-APACHE). You may choose either license.
