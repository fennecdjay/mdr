# MDR, a markdown runner

![Linux](https://github.com/fennecdjay/mdr/workflows/Linux/badge.svg)
![MacOs](https://github.com/fennecdjay/mdr/workflows/MacOs/badge.svg)
![Windows](https://github.com/fennecdjay/mdr/workflows/Windows/badge.svg)

mdr is a **small** (less than **500 C SLOC** :champagne:) *program* and *markup*
designed to facilitate documentation and testing.  


![logo](assets/logoreadme.png "The Mdr logo! (WIP)")

I started it to ease [Gwion](https://github.com/fennecdjay/gwion)'s devellopment,
but it is not tied in any way to this project.  

Let' walktrough... :smile:

## Hello World
let's write our first litterate progam.

### Define program structure

``` hello_world.c
@[[ Includes ]]

int main(int argc, char** argv) {
  @[[ Print ]]
}
```


### Add Headers
As we need the *puts* function, we need **stdio** headers.

``` Includes
#include <stdio.h>
```


### Print function

``` Print
puts("Hello, World!");
```



### Compile
with this line
``` sh
@exec cc hello_world.c -o hello_world
```
we compile *hello_world.c*.

``` sh
```

Yes, there should be no output, and that good news.



### Check
Let's look at hello_world.c

``` sh
@exec cat hello_world.c
```

``` c
#include <stdio.h>

int main(int argc, char** argv) {
  puts("Hello, World!");
}
```

That's the content of the source file we generated (and compiled).



### Test

Then we run it
``` sh
@exec ./hello_world
```

``` sh
Hello, World!
```

Do we read *Hello World!* ?
Assuming yes, let's continue.

### More test
Let's try it
```
[ "$(./hello_world)" = "Hello, World!" ] && echo "OK" || echo "NOT_OK"
```

and the result is
``` sh
OK
```

## Building

As a C program, it seemed natural to use [make](https://www.gnu.org/software/make)
as a build system.

``` sh
make
```

## Testing

Also using make:
``` sh
make test
```

You can also try
``` sh
bash scripts/test.sh
```


## Installing

As easy as before, just type.

``` sh
make install
```
or just copy `mdr` somewhere in your path.


-------

generated from [this file](https://github.com/fennecdjay/mdr/blob/master/README.mdr)

<!-- cleaning -->
