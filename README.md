# MDR, a markdown runner

![Linux](https://github.com/fennecdjay/mdr/workflows/Linux/badge.svg)
![MacOs](https://github.com/fennecdjay/mdr/workflows/MacOs/badge.svg)
![Windows](https://github.com/fennecdjay/mdr/workflows/Windows/badge.svg)
[![Line Count](https://tokei.rs/b1/github/fennecdjay/mdr)](https://github.com/Gwion/mdr)  

mdr is a **small** *program* and *markup*
designed to facilitate documentation and testing.
It can both generate documentation pages and
generate and actualaly test small code examples
in your documentation. (This ensures any code
examples you present to your users actually work.)

![logo](assets/logoreadme.png "The Mdr logo! (WIP)")

I started this tool to help with [Gwion](https://github.com/fennecdjay/gwion)'s
development, but it is not tied in any way to this project.

Let's go over the basic functionality... :smile:


## How to write documentation pages with `mdr`

For how to do a basic documentation
page, [see this page's original source](
https://github.com/fennecdjay/mdr/blob/master/README.mdr).


## How to write code examples with `mdr`

Let's write our first code example created from our
documentation page, which also shows off templating and
how to have the code automatically compiled and tested.


### Define a program structure

  > hello_world.c
``` hello_world.c  
@[[ Includes ]]

int main(int argc, char** argv) {
  @[[ Print ]]
}
```  
We fill in the `Includes` template variable as follows:


### Filling in template variables

As we need the *puts* function, we need **stdio** headers.

  > Includes
``` Includes  
#include <stdio.h>
```  

We also fill in the print function we use above:

  > Print
``` Print  
puts("Hello, World!");
```  


### Compiling the example code

Now, let's compile *hello_world.c* to make sure
this test code owrks.

  > exec: cc hello_world.c -o hello_world  
```
```  

Yes, there should be no output, and that's good news
since that means no compilation errors.


### Including a code file or other output

Let's look at *hello_world.c*:

  > exec: cat hello_world.c  
```
#include <stdio.h>

int main(int argc, char** argv) {
  puts("Hello, World!");
}
```  

That's the content of the source file we generated (and compiled).
You can see how this can be used for arbitrary shell commands
to generate output.


### Testing example code

Then we run our test program:

  > exec: ./hello_world  
```
Hello, World!
```  

Do we read *Hello World!* ?
Assuming yes, let's continue.


### More test

  > exec: [ "$(./hello_world)" = "Hello, World!" ] && echo "OK" || echo "NOT_OK"  
```
OK
```  


## Building `mdr`

As a C program, it seemed natural to use [make](https://www.gnu.org/software/make)
as a build system.

``` sh
make
```

## Testing `mdr`
                    
Also using make:    
``` sh
make test                 
```                   

You can also try
``` sh
bash scripts/test.sh
```

## Installing `mdr`

As easy as before, just type.

``` sh
make install
```
or just copy `mdr` somewhere in your path.


## Using `mdr`

To generate this doc page itself, use this command
in the repository root:

``` sh
mdr README.mdr
```

-------

generated from [this file](https://github.com/fennecdjay/mdr/blob/master/README.mdr)

